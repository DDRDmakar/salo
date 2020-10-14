/*
*
* SaloIntellect project
* Copyright (C) 2015-2020 Motylenok Mikhail, Makarevich Nikita
* 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <signal.h>

#include "headers/console.h"
#include "headers/misc.h"
//#include "conveer/headers/config.h" TODO
//#include "conveer/headers/teach.h"
#include "vk/headers/vkcom.h"
#include "web/headers/web.h"
#include "headers/translate.h"
#include "headers/statistics.h"
#include "headers/config.h" 
#include "headers/scrs.h"
#include "headers/cryptm.h"
#include "headers/external_api.h"

#include "headers/snapshots.h"
#include "headers/bots.h"

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>

int botui_enable = 0;


/*

Обрабатывает комманды snapshot

*/

std::string onSnapshot(const std::string& command)
{
	try
	{
		if(command == "snapshot help")
		{
			return miscGetFile("resources/snapshot_help.txt");
		}

		if(command.substr(0, 13) == "snapshot new ")
		{
			std::string arg = command.substr(13);
			if(arg == "") return std::string("No argument?"); 
			snapManager->createSnapshot(arg);
			return std::string("Snapshot \""+arg+"\" Created.");
		}

		if(command.substr(0, 17) == "snapshot bot add ")
		{
			std::string arg = command.substr(17);
			if(arg == "") return std::string("No argument?"); 
			snapManager->addBot(arg);
			return std::string("Done.");
		}

		if(command.substr(0, 19) == "snapshot bot create")
		{
			// format: snapshot bot create -name "botname" -pname "public bot name" --keywords "keyword1" "keyword2" ... "keywordN"
			
			std::string arg = command.length() <= 20 ? "" : command.substr(19); 
			snapManager->createBot(arg);
			return std::string("Done.");
		}

		if(command.substr(0, 23) == "snapshot interface add ")
		{
			std::string arg = command.substr(23);
			if(arg == "") return std::string("No argument?"); 

			std::cout << "\nSpecify interface type: \n1) vk.com\n2) Telegram\n3) Webiste and SaloAPI\n4) vk.com GROUP\nType number: ";
			char c;
			c = getchar();
			int TYPE;

			switch(c)
			{
				case '1': TYPE = INTERFACE_VKCOM; break;
				case '2': TYPE = INTERFACE_TELEGRAM; break;
				case '3': TYPE = INTERFACE_WEBFRONTEND; break;
				case '4': TYPE = INTERFACE_VKGROUP; break;
				default: return std::string ("Bad answer.");
			}

			std::string intfname;
			std::cout << "Specify interface simple name: ";
			std::cin >> intfname;

			snapManager->addInterface(arg, TYPE, intfname);
			return std::string("Done.");
		}

		if(command.substr(0, 20) == "snapshot bot delete ")
		{
			std::string arg = command.substr(20);
			if(arg == "") return std::string("No argument?"); 
			snapManager->deleteBot(arg);
			return std::string("Done.");
		}

		if(command.substr(0, 26) == "snapshot interface delete ")
		{
			std::string arg = command.substr(26);
			if(arg == "") return std::string("No argument?"); 
			
			std::string iname;
			std::cout << "Please, specify interface simple name (you can use \"snapshot show\" to learn it): ";
			std::cin >> iname;

			snapManager->deleteInterface(arg, iname);

			std::cout << "Changes will be fully applied afeter you have to RESTART SNAPSHOT (use \"snapshot start\").\n";
			return std::string("Done.");
		}

		if(command == "snapshot start")
		{
			snapManager->startBots();
			return std::string("Done.");
		}

		if(command == "snapshot stop")
		{
			snapManager->stopBots();
			return std::string("Done.");
		}

		if(command == "snapshot show")
		{
			return snapManager->listSnapshot();
		}

		if(command.substr(0, 14) == "snapshot load ")
		{
			std::string arg = command.substr(14);
			if(arg == "") return std::string("No argument?");
			snapManager->LoadSnapshot(arg); 
			return std::string("Done");
		}
	
		if(command.substr(0, 17) == "snapshot default ")
		{
			std::string arg = command.substr(17);
			snapManager->setDefault(arg);
			return std::string("Done.");
		}

		if(command.substr(0, 25) == "snapshot password delete ")
		{
			std::string arg = command.substr(25);
			if(arg == "") return std::string("No argument?");
			
			std::string iname;
			std::cout << "Please, specify interface simple name (you can use \"snapshot show\" to learn it): ";
			std::cin >> iname;

			snapManager->resetPassword(arg, iname); 
			return std::string("Done");
		}

		if(command == "snapshot convreloadcfg")
		{
			snapManager->reloadConveerConfigs();
			return std::string("Done.");
		}

		if(command.substr(0, 19) == "snapshot bot pause ")
		{
			std::string arg = command.substr(19);
			if(arg == "") return std::string("No argument?"); 
			snapManager->toggleProcessingPause(arg);
			return std::string("Bot \""+arg+"\" pause state changed.");
		}

		if(command == "snapshot exit")
		{
			snapManager->ExitSnapshot();
			return std::string("Done.");
		}

	}
	catch(...)
	{
		return std::string("Bad Command!");
	}

	return std::string("Unknown command!");
}


/*

Обрабатывает введеные в консоль команды

*/

std::string onCommand(const std::string& command, bool internal, Person p, CONFIGURATION h, void* externalBotV, const std::string& externalInterfaceName)
{
	logs->CommitGeneric(F, L, "onCommand called (" + command + ")");

	if(command == "exit") 
		{ 
			if(internal)
			{
				ServerAlive = 0; 
				logs->CommitGeneric(F, L, "Exit: SALO Server quit by console...");
				return std::string("Exit: SALO Server quit by console..."); 
			}
			else
			{
				return std::string("Exit: SALO Server cannot be killed by external console command.");
			}

		}

	if(command == "forcequit") 
		{ 
			if(internal)
			{
				ServerAlive = 0; 
				logs->CommitGeneric(F, L, "Exit: SALO Server UNSAFE FORCED quit by console...");
				std::cout << "Exit: SALO Server UNSAFE FORCED quit by console..." << std::endl;
				exit(-1);

				return std::string(); 
			}
			else
			{
				return std::string("Exit: SALO Server cannot be killed by external console command.");
			}

		}


	if(command == "help" || command == "помощь" || command == "инфо" || command == "info")
		{
			if(internal) return miscGetFile("resources/commands.txt");
			else return std::string("Service commands are not available from external console!");
		}

	if(command == "st")
		{
			networking_vkcom* vk;
			bot* externalBot;
			
			try
			{
				externalBot = static_cast<bot*>(externalBotV);
				if(externalBot == NULL) 
				{ 
					vk = NULL; 
				}
				else
				{
					try 
					{
						vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
					} 
					catch(std::bad_cast)
					{
						vk = NULL;
					}	
				}
			}
			catch(...)
			{
				vk = NULL;
				externalBotV = NULL;
			}

			std::string connstr, botstr;

			if(vk != NULL)
			{
				if(vk->getInterfaceConnector() != NULL)
				{
					connstr = "\nUsing Connector: " + vk->getInterfaceConnector()->connectorType + "\n";
				}
			}

			if(externalBot != NULL)
			{
				botstr = "Using Bot: "+externalBot->getName();
			}

			return (statistics->getStatusString() + connstr + botstr); 
		}

	if(command == "statistics")
		{
			return statistics->getStatisticsString(externalBotV); 
		}	

	if(command == "msgstats")
		{
			return statistics->getMessagesString(false, externalBotV);
		}

	if(command == "networkdbg")
		{ 
			if(!internal) { return std::string("Service commands are not available from external console!"); }

			bot* externalBot = static_cast<bot*>(externalBotV);
			if(externalBot == NULL) { return std::string("Bot is NULL for this command! Please contact developers!"); }
			networking_vkcom* vk;
			vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
			if(vk == NULL) { return std::string("Interface is NULL for this command! Please contact developers!"); }

			if(vk->pNetwork->DebugMode == 0) 
				{ 
					vk->pNetwork->DebugMode = 1; 
				} 
				else 
				{   
					vk->pNetwork->DebugMode = 0; 
				} 

			std::string res = "Network debugging mode = " + std::to_string(vk->pNetwork->DebugMode);
			return res; 
		}

	if(command == "vkrelogin")
		{ 
			if(!internal) { return std::string("Service commands are not available from external console!"); }
			
			bot* externalBot = static_cast<bot*>(externalBotV);
			if(externalBot == NULL) { return std::string("Bot is NULL for this command! Please contact developers!"); }
			networking_vkcom* vk;
			vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
			if(vk == NULL) { return std::string("Interface is NULL for this command! Please contact developers!"); }

			vk->force_relogin = true;

			return std::string("Done."); 
		}

	if(command == "timings")
		{ 
			bot* externalBot = static_cast<bot*>(externalBotV);
			if(externalBot == NULL) { return std::string("Bot is NULL for this command! Please contact developers!"); }
			networking_vkcom* vk;
			vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
			if(vk == NULL) { return std::string("Interface is NULL for this command! Please contact developers!"); }

			return vk->printTimings();  
		}
		
	if(command == "reloadcfg")
		{
			if(!internal) { return std::string("Service commands are not available from external console!"); }
			
			snapManager->reloadConveerConfigs();
			return std::string("Done.");
		}

	if(command == "logsecho")
		{
			if(!internal) { return std::string("Service commands are not available from external console!"); }

			logs->TriggerEcho();

			return std::string("Done.");
		}

	if(command == "loadstatus")
		{
			return statistics->getStressStatusString();
		}

	if(command == "idlebreaker")
		{
			if(!internal) { return std::string("Service commands are not available from external console!"); }
			
			bot* externalBot = static_cast<bot*>(externalBotV);
			if(externalBot == NULL) { return std::string("Bot is NULL for this command! Please contact developers!"); }
			networking_vkcom* vk;
			vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
			if(vk == NULL) { return std::string("Interface is NULL for this command! Please contact developers!"); }

			vk->enable_idlebreaker = !vk->enable_idlebreaker;

			return std::string("ENABLE IDLE BREAKER = " + std::to_string(vk->enable_idlebreaker));			
		}

	if(command == "cooldown")
		{
			if(!internal) { return std::string("Service commands are not available from external console!"); }

			bot* externalBot = static_cast<bot*>(externalBotV);
			if(externalBot == NULL) { return std::string("Bot is NULL for this command! Please contact developers!"); }
			networking_vkcom* vk;
			vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
			if(vk == NULL) { return std::string("Interface is NULL for this command! Please contact developers!"); }

			vk->floodprot->cooldown_enabled = !vk->floodprot->cooldown_enabled;

			return std::string("Cooldown enabled = " + std::to_string(vk->floodprot->cooldown_enabled));
		}

	if(command == "fetcherdbg")
		{
			if(!internal) { return std::string("Service commands are not available from external console!"); }

			bot* externalBot = static_cast<bot*>(externalBotV);
			if(externalBot == NULL) { return std::string("Bot is NULL for this command! Please contact developers!"); }
			networking_vkcom* vk;
			vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
			if(vk == NULL) { return std::string("Interface is NULL for this command! Please contact developers!"); }

			vk->debugFetcher = !vk->debugFetcher;

			return std::string("Fetcher debug mode = " + std::to_string(vk->debugFetcher));
		}

	if(command == "promotions")
		{
			std::string t = "Registered promotions list:\n\n";
			std::vector<std::string> promo = workdat->get_group("promotion_link"); 

			for(int i = 0; i < promo.size(); i++)
			{
				t += promo[i] + "\n";
			}
			
			bot* externalBot = static_cast<bot*>(externalBotV);
			if(externalBot == NULL) { return std::string("No bot specified!\n\n"+t); }
			networking_vkcom* vk;
			vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
			if(vk == NULL) { return std::string("No interface specified!\n\n"+t); }

			unsigned int k = vk->getPromotionSentCounter();

			std::string kk = std::to_string(k);

			return std::string("Promotions sent in this session from this interface: " + kk + "\n\n" + t);
		}

	if(command == "fetcherstatus")
		{
			return statistics->getFetcherStatusString();
		}
	
	if(command == "botstats")
	{
		return statistics->getBotStats(externalBotV); 
	}

	if(command == "botui")
		{
			botui_enable = 1;
			return std::string("Done.");
		}

	//snapshot commands
	if(command.length() > 8 && internal)
	{
		if(command.substr(0,8) == "snapshot")
		{
			return onSnapshot(command);
		}
	}

	//speechmal 
	if(command.substr(0, 10) == "speechmal ")
	{
		bot* externalBot = static_cast<bot*>(externalBotV);
		if(externalBot == NULL) { return std::string("Bot is NULL for this command! Please contact developers!"); }
		networking_vkcom* vk;
		vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
		if(vk == NULL) { return std::string("Interface is NULL for this command! Please contact developers!"); }

		int response_id = h.isConf ? h.confId : p.userID;
		
		ResponseData dat;
		dat.targetInterface = vk;
		dat.targetPeer = response_id;

		std::string speechText = command;
		speechText.erase(0, 10);
		speechText = "a" + speechText;

		scrs("synth", speechText, dat);
	}

	//speechfem 
	if(command.substr(0, 10) == "speechfem ")
	{
		bot* externalBot = static_cast<bot*>(externalBotV);
		if(externalBot == NULL) { return std::string("Bot is NULL for this command! Please contact developers!"); }
		networking_vkcom* vk;
		vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
		if(vk == NULL) { return std::string("Interface is NULL for this command! Please contact developers!"); }

		int response_id = h.isConf ? h.confId : p.userID;
		
		ResponseData dat;
		dat.targetInterface = vk;
		dat.targetPeer = response_id;

		std::string speechText = command;
		speechText.erase(0, 10);
		speechText = "b" + speechText;

		scrs("synth", speechText, dat);
	}

	//Обработка изображений
	if(command.substr(0,8) == "krapiva ")
	{
		bot* externalBot = static_cast<bot*>(externalBotV);
		if(externalBot == NULL) { return std::string("Bot is NULL for this command! Please contact developers!"); }
		networking_vkcom* vk;
		vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
		if(vk == NULL) { return std::string("Interface is NULL for this command! Please contact developers!"); }

		int response_id = h.isConf ? h.confId : p.userID;
		
		ResponseData dat;
		dat.targetInterface = vk;
		dat.targetPeer = response_id;
		dat.krapivaInputPhotoId = h.attachment_jpeg;
		//if(h.attachment_token != std::string()) dat.krapivaInputPhotoId += "_" + h.attachment_token;
		dat.krapivaFingerprint = vk->getSCRSfingerprint();

		if(dat.krapivaInputPhotoId == std::string())
			dat.krapivaInputPhotoId = "null";

		//std::cout << dat.krapivaInputPhotoId << std::endl;

		if(dat.krapivaFingerprint == std::string())
		{
			AlertDevelopers("SCRS Error: Error generating Salo's fingerprint!");
			return std::string("Error generating fingerprint"); 
		}

		std::string krapivaQuery = command;
		krapivaQuery.erase(0, 8);

		scrs("krapiva", krapivaQuery, dat);
	}	

	//weather
	if(command.substr(0, 8) == "weather " && command.length() > 8)
	{
		std::string qr = command;
		qr = qr.erase(0, 8);

		return weather(qr);
	}

	//weather
	if(command == "weather")
	{
		return("Укажите, пожалуйста, город (желательно полное название города, без ошибок и сокращений)");
	}

	if(command == "oldsyncbans")
	{
		bot* externalBot = static_cast<bot*>(externalBotV);
		if(externalBot == NULL) { return std::string("Bot is NULL for this command! Please contact developers!"); }
		networking_vkcom* vk;
		vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
		if(vk == NULL) { return std::string("Interface is NULL for this command! Please contact developers!"); }

		return vk->forceBanlistRefresh(externalBot->getName(), externalBot->getConveerPtr()->get_database_name());
	}

	if(command == "cryptm")
	{
		if(!internal) { return std::string("Service commands are not available from external console!"); }
		return ("Using libcryptm: " + std::string(version()));
	}

	if(command.substr(0, 7) == "repost " && command.length() > 7)
	{
		std::string qr = command;
		qr = qr.erase(0, 7);

		bot* externalBot = static_cast<bot*>(externalBotV);
		if(externalBot == NULL) { return std::string("Bot is NULL for this command! Please contact developers!"); }
		networking_vkcom* vk;
		vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
		if(vk == NULL) { return std::string("Interface is NULL for this command! Please contact developers!"); }

		return vk->makeRepost(qr, false);
	}

	if(command.substr(0, 10) == "pinrepost " && command.length() > 10)
	{
		std::string qr = command;
		qr = qr.erase(0, 10);

		bot* externalBot = static_cast<bot*>(externalBotV);
		if(externalBot == NULL) { return std::string("Bot is NULL for this command! Please contact developers!"); }
		networking_vkcom* vk;
		vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
		if(vk == NULL) { return std::string("Interface is NULL for this command! Please contact developers!"); }

		return vk->makeRepost(qr, true);
	}

	//Test function, to simulate extremal situtaions.
	if(command == "segfault")
	{
		if(!internal)
		{
			return std::string("SALO Server cannot be segfaulted by external console command!");
		}

		std::cout << "error exit mannually..." << std::endl;
		fprintf(stdout, "\n\nChecking stdout dumping...\n\n");
		fprintf(stderr, "\n\nChecking stderr dumping...\n\n");

		//*(int*)0 = 0;

		std::cout << "Enter test crash type: \n1) exit(-1); \n2) segfault \n3) invalid instruction \n4) abort();\n> ";
		int i = 0;
		std::cin >> i;

		switch(i)
		{
			case 1: exit(-1); break;
			case 2: raise(SIGSEGV); break;
			case 3: raise(SIGILL);  break;
			case 4: abort(); break;
			default: return std::string("Not today."); break;
		}		
	}

	//speech generator
	if(command.substr(0, 10) == "speechgen ")
	{
		std::string qr = command;
		qr = qr.erase(0, 10);

		int arg = 0;

		try 
		{
			arg = std::stoi(qr);
		}
		catch (...)
		{
			return std::string("Укажите (числом) количество предложений в речи после команды!");	
		}

		return generateSpeech(arg);
	}

	//speech generator
	if(command == "speechgen")
	{
		return std::string("Укажите количество предложений в речи после команды!");
	}

	//line5
	if(command.substr(0, 6) == "line5 ")
	{
		std::string qr = command;
		qr = qr.erase(0, 6);	

		miscSetFile("resources/line5.txt", qr);

		return std::string("line5 updated.");
	}

	//syncbans2
	if(command == "syncbans")
	{
		bot* externalBot = static_cast<bot*>(externalBotV);
		if(externalBot == NULL) { return std::string("Bot is NULL for this command! Please contact developers!"); }
		networking_vkcom* vk;
		vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(externalInterfaceName));
		if(vk == NULL) { return std::string("Interface is NULL for this command! Please contact developers!"); }

		return vk->forceBanlistRefreshPy(externalBot->getName(), externalBot->getConveerPtr()->get_database_name());
	}


	//TODO: add more commands here

	if(command != "")
	{
		logs->CommitGeneric(F, L, "unknown command (" + command + ")");
		return "SALO Server doesn't know \"" + command + "\". Try \"help\" to learn more!";
	}

	return std::string();
}


/*

Основная функция консоли сервера
Запускается в отдельном потоке

*/

void server_console()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(1337));

	std::cout <<  std::endl << "Salo console: " << std::endl << "Type \"help\" for list of availible commands." << std::endl;
	std::cout << "Type \"botui\" to switch from Salo Console to BotUI mode" << std::endl;
	int cmdc = 0;

		while(ServerAlive)
		{
			if(botui_enable == 0)
			{
				std::cout << "#> ";
				std::string cmd; 
				std::getline(std::cin, cmd);
				std::string command_result = onCommand(cmd, true, Person(), CONFIGURATION());
				std::cout << command_result << std::endl;
				cmdc++;
			}
			else
			{
				botui();
			}
		}
}



/*

Небольшой интерфейс, чтобы показывать список ботов и быстрые действия с ними

*/

void botui()
{
	std::cout << "\033[2J\033[1;1H";
	std::string upper = " Press Enter to get back to the text console ";
	std::string line;

	struct winsize ws;
    ioctl(0, TIOCGWINSZ, &ws);
    line += "+";
    for(int i = 0; i < ws.ws_col-2; i++) line += "-";
    line += "+";

	std::cout << line;
	
	while(upper.length() < ws.ws_col)
	{
		if(upper.length() % 2 == 0)
			upper = upper + " ";
		else
			upper = " " + upper;
	}
	
	std::cout << miscFormatted(miscFormatted(upper, 6), 7) << std::endl << std::endl;
	std::cout << miscFormatted("It tekes some resources to update this screen, so please switch back to text console before leaving terminal session!\n\n", 3);
	std::cout << statistics->getStatusString() << std::endl << std::endl;
	std::cout << line;
	std::cout << snapManager->listSnapshot();
	std::cout << line << std::endl << std::endl; 

	fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(STDIN_FILENO, &readSet);
    struct timeval tv = {0, 900};  // 0 sec, 700 msec
    if (select(STDIN_FILENO+1, &readSet, NULL, NULL, &tv) < 0) perror("select");

    char C = (FD_ISSET(STDIN_FILENO, &readSet)) ? (C = getchar()) : 0;

	if(C != 0)
	{
		botui_enable = 0;
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));	
}
