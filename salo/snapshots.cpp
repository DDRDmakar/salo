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


#include "headers/config.h"
#include "headers/snapshots.h"
#include "headers/misc.h"

#include "vk/headers/vkcom.h"

#include "conveer/headers/arg_parser.h"
#include "conveer/headers/file.h"
#include "conveer/headers/exception.h"
#include "headers/salostring.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <chrono>


/*

Деструктор

*/

snapshotManager::~snapshotManager()
{
	std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Shutdown... " << std::endl;

	//TODO: ask for save unsaved changes

	//разрушает ботов
	for(int i = 0; i < bots.size(); i++)
	{
		delete bots[i];
	}
}


/*

Создает новый снапшот

*/

void snapshotManager::createSnapshot(const std::string& Name)
{
	if(active_snapshot != std::string())
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Exit active snapshot before creating a new one." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	struct stat st = {0};

	std::string basepath = "bots/";

	if (stat(std::string(basepath + Name).c_str(), &st) == -1) 
	{
	    mkdir(std::string(basepath + Name).c_str(), 0766);
	    std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) 
	    			<< " Switching to a new empty snapshot \"" << Name << "\" (" << std::string(basepath + Name) << ")" << 
	    			std::endl << "Now, you may whant add some bots to it? Use \"snapshot bot add [botname]\" command." << std::endl;
	    active_snapshot = Name;
	}
	else
	{
		std::cout << "Already exists, aborting." << std::endl;
	}
}


/*

Возвращает бота по имени

*/

bot* snapshotManager::getBotByName(const std::string& name)
{
	for(int i =0; i < bots.size(); i++)
	{
		if(bots[i]->getName() == name)
			return bots[i];
	}

	return NULL;
}


/*

Добавляет ботов

*/

void snapshotManager::addBot(const std::string& botname, bool reuse)
{
	if(active_snapshot == std::string())
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No active snapshots, load it from disk or create a new one." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	bot* newbee = new bot(botname);

	std::string basepath = "bots/" + active_snapshot + "/";

	if(!reuse)
	{
		struct stat st = {0};

		if (stat(std::string(basepath + newbee->getName()).c_str(), &st) == -1) 
		{
		    mkdir(std::string(basepath + newbee->getName()).c_str(), 0766);
		}
		else
		{
			std::cout << "Already exists, aborting." << std::endl;
			return;
		}
	}

	bots.push_back(newbee);
	std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Bot added. " 
			<< std::endl << "Now, you may whant add some interfaces? Use \"snapshot interface add [botname]\" command." << std::endl; 
}


/*

Создаёт конфигурацию для нового бота и запускает его

*/

void snapshotManager::createBot(const std::string &argline)
{
	try
	{
		if (argline.empty())
		{
			std::cout << "Argument line is empty! Use arguments as in example:\n";
			std::cout << "snapshot bot create -template \"template bot name\" -name \"botname\" -pname \"public bot name\" --keywords \"keyword1\" \"keyword2\" ... \"keywordN\"\n";
			std::cout << "-template - bot, on which this bot would be based. (optional) DEFAULT: commercial_template\n";
			std::cout << "-name - bot name in database. Should be unique.\n";
			std::cout << "-pname - bot name for answers (how bot calls itself). (optional)\n";
			std::cout << "--keywords - keywords to call bot in group conversation. (keywords from template will be imported too) (optional) DEFAULT: empty array\n";
			return;
		}
		
		const Convspace::Arg_parser args(argline);
		
		if (
			!args.arg_named_exists("name") ||
			args.get_arg_named("name").empty() ||
			(args.arg_named_exists("pname") && args.get_arg_named("pname").empty())
		)
		{
			std::cout << "Bot creation error - incorrect arguments\n";
			return;
		}
		
		const std::string bot_directory   = "resources/config/bot_config/" + args.get_arg_named("name") + "/";
		const std::string templ_directory = "resources/config/bot_config/" + (args.arg_named_exists("template") ? args.get_arg_named("template") : "commercial_template") + "/";
		
		if (Convspace::dir_exists(bot_directory))  { std::cout << "Bot creation error - bot already exists\n"; return; }
		if (!Convspace::dir_exists(templ_directory)) { std::cout << "Bot creation error - template not found\n"; return; }
		
		// Create bot directory and put config files into it
		Convspace::make_dir(bot_directory);
		if (Convspace::file_exists(templ_directory + "main.yaml")) Convspace::copy_file(templ_directory + "main.yaml", bot_directory + "main.yaml");
		else { std::cout << "Bot creation error - main.yaml not found\n"; return; }
		if (Convspace::file_exists(templ_directory + "addition.yaml")) Convspace::copy_file(templ_directory + "addition.yaml", bot_directory + "addition.yaml");
		else Convspace::write_file(bot_directory + "addition.yaml", L"additions: []");
		
		Convspace::Config_yaml co(WideToMultibyteString(Convspace::read_file(bot_directory + "addition.yaml")));
		
		YAML::Node addition = co.editable_subtree();
		
		int additions_list_size = addition["additions"] ? addition["additions"].size() : 0;
		
		if (!addition["additions"]) { std::cout << "Bot creation error - additions list does not exist in template\n"; return; };
		addition["additions"][additions_list_size]["mode"] = "set";
		addition["additions"][additions_list_size]["path"] = "bot_name";
		addition["additions"][additions_list_size]["value"] = args.get_arg_named("name");
		++additions_list_size;
		if (args.arg_named_exists("pname"))
		{
			addition["additions"][additions_list_size]["mode"] = "set";
			addition["additions"][additions_list_size]["path"] = "bot_name_public";
			addition["additions"][additions_list_size]["value"] = args.get_arg_named("pname");
			++additions_list_size;
		}
		
		const std::vector<std::string> keywords = args.get_array_named("keywords");
		for (const auto &e : keywords)
		{
			addition["additions"][additions_list_size]["mode"] = "insert";
			addition["additions"][additions_list_size]["path"] = "fetcher_keyword";
			addition["additions"][additions_list_size]["value"] = e;
			++additions_list_size;
			addition["additions"][additions_list_size]["mode"] = "insert";
			addition["additions"][additions_list_size]["path"] = "primary_keyword";
			addition["additions"][additions_list_size]["value"] = e;
			++additions_list_size;
		}
		
		Convspace::write_file(bot_directory + "addition.yaml", MultibyteToWideString(co.show()));
		
		std::cout << "New bot configuration created successfully\n";
	}
	catch (Convspace::Exception &e) { std::cout << "Configuration creating failed: " + e.what() + "\n"; }
	catch (YAML::Exception &e) { std::cout << "Configuration creating failed: " + std::string(e.what()) + "\n"; }
}

/*

Добавляет интерфейс к боту

*/

void snapshotManager::addInterface(const std::string& botname, int TYPE, const std::string& name, bool reuse)
{
	if(active_snapshot == std::string())
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No active snapshots, load it from disk or create a new one." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	bot* currBot = getBotByName(botname);

	if(currBot == NULL)
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Bot not found. Please check its name or id using \"snapshot show\" command" 
				<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
		return;
	}

	std::string path = "bots/" + active_snapshot + "/" + botname + "/i" + std::to_string(TYPE) + "_" + name + ".bots";
	InterfaceSettings set;

	if(!reuse)
	{
		std::string defaultContent = "Salo 3 bot interface file\nConfiguration strings here:\n'official'='0';\n'anticaptcha_enabled'='0';\n'anticaptcha_key'='';\n\
'alertdevelopers_here'='0';\n'vkgroup_numeric_id'='';\n'vkgroup_members_delay'='0';\n'vk_ignore_friends'='0';\n'vk_ignore_bdater'='0';\n\
'vk_confchats_only'='0';\n'custom_grouplink'='';\n'vkgroup_use_callbackapi'='1';\n'personal_promotion'='';";		
		miscSetFile(path.c_str(), defaultContent); 
	}
	else
	{
		std::string params = miscGetFile(path.c_str());

		std::string offic = miscFindAndCopyString(params.c_str(), "'official'='", "';");
		if(offic == std::string()) offic = "0";
		set.official = std::stoi(offic);

		std::string caEnable = miscFindAndCopyString(params.c_str(), "'anticaptcha_enabled'='", "';");
		if(caEnable == std::string()) caEnable = "0";
		set.anticaptchaEnabled = std::stoi(caEnable);

		std::string caKey = miscFindAndCopyString(params.c_str(), "'anticaptcha_key'='", "';");
		if(caKey == std::string()) caKey = "";
		set.anticaptchaApiKey = caKey;

		std::string alertDev = miscFindAndCopyString(params.c_str(), "'alertdevelopers_here'='", "';");
		if(alertDev == std::string()) alertDev = "0";
		set.alertdevelopersEnabled = std::stoi(alertDev);

		std::string vkgroupid = miscFindAndCopyString(params.c_str(), "'vkgroup_numeric_id'='", "';");
		if(vkgroupid == std::string()) vkgroupid = "";
		set.vkgroupId = vkgroupid;

		std::string memdelay = miscFindAndCopyString(params.c_str(), "'vkgroup_members_delay'='", "';");
		if(memdelay == std::string()) memdelay = "0";
		set.vkgroupMemberscheckDelay = std::stoi(memdelay);

		std::string ignorefr = miscFindAndCopyString(params.c_str(), "'vk_ignore_friends'='", "';");
		if(ignorefr == std::string()) ignorefr = "0";
		set.ignoreFriendsReq = std::stoi(ignorefr);

		std::string ignorebdates = miscFindAndCopyString(params.c_str(), "'vk_ignore_bdater'='", "';");
		if(ignorebdates == std::string()) ignorebdates = "0";
		set.ignoreBirthdays = std::stoi(ignorebdates);

		std::string confonly = miscFindAndCopyString(params.c_str(), "'vk_confchats_only'='", "';");
		if(confonly == std::string()) confonly = "0";
		set.confchatsOnly = std::stoi(confonly);

		std::string customgroup = miscFindAndCopyString(params.c_str(), "'custom_grouplink'='", "';");
		if(customgroup == std::string()) customgroup = "";
		set.customGroupLink = customgroup;

		std::string usecallback = miscFindAndCopyString(params.c_str(), "'vkgroup_use_callbackapi'='", "';");
		if(usecallback == std::string()) usecallback = "0";
		set.useCallbackApiConnector = std::stoi(usecallback);

		std::string perspromo = miscFindAndCopyString(params.c_str(), "'personal_promotion'='", "';");
		if(perspromo == std::string()) perspromo = "";
		set.personalPromotion = perspromo;


		//'vkgroup_numeric_id'='';
		//'vkgroup_members_delay'='0';
	}

	currBot->createInterface(TYPE, name, active_snapshot, set);

	std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Interface added" << std::endl;
}


/*

Удаляет бота из снэпшота

*/

void snapshotManager::deleteBot(const std::string& botname)
{
	if(active_snapshot == std::string())
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No active snapshots, load it from disk or create a new one." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	bot* currBot = getBotByName(botname);

	if(currBot == NULL)
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Bot not found. Please check its name or id using \"snapshot show\" command" 
				<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
		return;
	}

	std::string basepath = "bots/" + active_snapshot + "/";

	struct stat st = {0};

	if (stat(std::string(basepath + currBot->getName()).c_str(), &st) == -1) 
	{
	    std::cout << "Doesen't exists, aborting." << std::endl;
		return;
	}
	else
	{
		//currBot->Stop();

		for(int i = 0; i < bots.size(); i++)
		{
			if(bots[i]->getName() == botname)
			{
				bots.erase(bots.begin() + i);
				break;
			}
		}

		delete currBot;

		system(std::string("rm -rf " + basepath + botname).c_str());
	}

	
	std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Bot deleted. \n"; 
}


/*

Удаляет интерфейс из бота в снэпшоте

*/

void snapshotManager::deleteInterface(const std::string& botname, const std::string& interfacename)
{
	if(active_snapshot == std::string())
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No active snapshots, load it from disk or create a new one." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	bot* currBot = getBotByName(botname);

	if(currBot == NULL)
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Bot not found. Please check its name or id using \"snapshot show\" command" 
				<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
		return;
	}

	std::string answ;
	std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Are you shure? [y/n]: ";
	std::cin >> answ;

	if(answ != "y" && answ != "Y")
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Answer is \"NO\" -> Aborting. ";
		return;
	}

	int TYPE = currBot->getInterfaceTypeBySimpleName(interfacename);

	if(TYPE == -1)
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " interface not found -> Aborting. ";
		return;
	}

	std::string path = "bots/" + active_snapshot + "/" + botname + "/i" + std::to_string(TYPE) + "_" + interfacename + ".bots";
	system(std::string("rm -f " + path).c_str());

	currBot->deleteInterface(interfacename);

	std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Interface deleted. It is strongly recommended to restart the bots unsing \"snapshot start\"\n";
}


/*

Удаляет логин и пароль от бота

*/

void snapshotManager::resetPassword(const std::string& botname, const std::string& interfacename)
{
	if(active_snapshot == std::string())
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No active snapshots, load it from disk or create a new one." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	bot* currBot = getBotByName(botname);

	if(currBot == NULL)
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Bot not found. Please check its name or id using \"snapshot show\" command" 
				<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
		return;
	}
	
	int TYPE = currBot->getInterfaceTypeBySimpleName(interfacename);

	if(TYPE == -1)
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " interface not found -> Aborting. ";
		return;
	}

	std::string path = "bots/" + active_snapshot + "/" + botname + "/i" + std::to_string(TYPE) + "_" + interfacename + ".account";
	std::cout << "Account file: " << path << std::endl;
	system(std::string("rm -f " + path).c_str());

	std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Login data deleted. You can enter it again on next bot start.\n";
}


/*

Старт всех ботов

*/

void snapshotManager::startBots()
{
	if(active_snapshot == std::string())
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No active snapshots, load it from disk or create a new one." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	if(bots.size() == 0)
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No bots on snapshot. There is nothing to start." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Upstarting inactive bots and restarting active bots.\n"; 

	for(int i = 0; i < bots.size(); i++)
	{
		//restart if already running
		if(bots[i]->isRunning())
		{
			bots[i]->Stop();
		}

		bots[i]->Upstart();
	}
}


/*

Стоп всех ботов

*/

void snapshotManager::stopBots()
{

	if(active_snapshot == std::string())
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No active snapshots, load it from disk or create a new one." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	if(bots.size() == 0)
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No bots on snapshot. There is nothing to stop." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Stopping all bots.\n"; 

	for(int i = 0; i < bots.size(); i++)
	{
		bots[i]->Stop();
	}
}


/*

Список ботов на снэпшоте

*/

std::string snapshotManager::listSnapshot()
{
	if(active_snapshot == std::string())
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No active snapshots, load it from disk or create a new one." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return std::string();
	}

	if(bots.size() == 0)
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No bots on snapshot. There is nothing to show." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return std::string();
	}

	std::string t;

	t += miscFormatted(miscFormatted("SNAPSHOT: ", 5), 1) + " Name: \"" + active_snapshot + "\"; Bots: " 
			+ std::to_string(bots.size()) + "; Unsaved changes: ";

	if(unsaved_changes)
		t += miscFormatted("Yes", 3);
	else
		t += miscFormatted("No", 2);

	t += "\n";

	for(int i = 0; i < bots.size(); i++)
	{
		if(i == bots.size()-1)
		{
			t += " └─ ";
			t += bots[i]->getInfo(false);
			t += "\n";
		}
		else
		{
			t += " ├─ ";
			t += bots[i]->getInfo(true);
			t += " |\n";
		}
	}

	return t;
}


/*

Обновление конфига у каждого конвеера ботов

*/

void snapshotManager::reloadConveerConfigs()
{
	if(active_snapshot == std::string())
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No active snapshots, load it from disk or create a new one." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	if(bots.size() == 0)
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No bots on snapshot. There is nothing to refresh." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Reloading conveer config files on bots.\n"; 

	for(int i = 0; i < bots.size(); i++)
	{
		std::cout << "Refreshing conveer config on \"" << bots[i]->getName() << "\"...  ";
		bots[i]->lockBotsProcessing();
		bots[i]->getConveerPtr()->refresh_config();
		bots[i]->unlockBotsProcessing();
		std::cout << "Done." << std::endl;
	}

	std::cout << "Refreshing generic config on \"GENERIC\"...  ";
	workdat->refresh();
	std::cout << "Done." << std::endl;

}


/*

Загрузка снэпшота из файла

*/

void snapshotManager::LoadSnapshot(const std::string& snap)
{
	if(active_snapshot != std::string())
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Exit active snapshot before creating a new one." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	bool fail = false;
	active_snapshot = snap;

	struct stat st = {0};

	std::string basepath = "bots/";

	if (stat(std::string(basepath + snap).c_str(), &st) == -1) 
	{
	    std::cout << "File doesen't exists or inaccessible (stat failed), aborting." << std::endl;
	    fail = true;
	    return;
	}
	else
	{
		struct dirent** namelist;
		
	    int n = scandir(std::string(basepath + snap).c_str(), &namelist, 0, alphasort);
	    
	    if (n < 0)
	    {
	    		std::cout << "ERROR - FILE NOT FOUND OR INACCESSIBLE!" << std::endl;
	    		fail = true;
	    }
	    else 
	    {
		    for (int i = 0; i < n; i++) 
	        	{
		           	//printf("%s\n", namelist[i]->d_name);
		        	std::string bts = std::string(namelist[i]->d_name);
		        	free(namelist[i]);

		        	if(bts == "." || bts == "..")
		        	{
		        		continue;
		        	}

		        	std::cout << "Found bot: " << bts << std::endl;
		        	addBot(bts, true);

		        	struct dirent** namelist2;
		
					    int p = scandir(std::string(basepath + snap + "/" + bts).c_str(), &namelist2, 0, alphasort);
					    
					    if (p < 0)
					    {
					    		std::cout << "ERROR - FILE NOT FOUND OR INACCESSIBLE!" << std::endl;
					    		fail = true;
					    }
					    else 
					    {
						    for (int j = 0; j < p; j++) 
					        	{
						           	//printf("%s\n", namelist[i]->d_name);
						        	std::string itf = std::string(namelist2[j]->d_name);
						        	free(namelist2[j]);

						        	char* cacc = miscFindAndCopy(itf.c_str(), "_", ".account");
						        	char* cdat = miscFindAndCopy(itf.c_str(), "_", ".datetime");

						        	if(itf == "." || itf == "..")
						        	{
						        		continue;
						        	}

						        	if(cacc != NULL) { delete[] cacc; continue; }
						        	if(cdat != NULL) { delete[] cdat; continue; }

						        	std::cout << "Found interface: " << itf << std::endl;

						        	char* ctyp = miscFindAndCopy(itf.c_str(), "i", "_");
						        	char* cnam = miscFindAndCopy(itf.c_str(), "_", ".bots");

						        	int type = atoi(ctyp);
						        	std::string name = std::string(cnam);

						        	delete[] ctyp;
						        	delete[] cnam;

						        	addInterface(bts, type, name, true);
						        }
					    }
					    
					    free(namelist2);
		        }
	    }
	    
	    free(namelist);


	    if(!fail)
	    {
	    	startBots();
	    }

	}

}


/*

Переключение статуса паузы у бота

*/

void snapshotManager::toggleProcessingPause(const std::string& botname)
{
	if(active_snapshot == std::string())
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " No active snapshots, load it from disk or create a new one." 
			<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
			return;
	}

	bot* currBot = getBotByName(botname);

	if(currBot == NULL)
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Bot not found. Please check its name or id using \"snapshot show\" command" 
				<< std::endl << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Aborting." << std::endl;
		return;
	}

	if(currBot->isProcessingLocked())
	{
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Bot processing resumed." << std::endl;
		currBot->unlockBotsProcessing();
	}
	else
	{
		currBot->lockBotsProcessing();
		std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Bot processing paused. You can enter this command again to unpause bot!" << std::endl; 
	}

	return;
}




/*

Установка дефолтного снапшота

*/

void snapshotManager::setDefault(const std::string& snap)
{
	std::cout << miscFormatted(miscFormatted("[SNAPSHOT]", 5), 1) << " Note: you can disable auto-load by passing empty or \"none\" argument\n";

	if(snap == "none" || snap == "")
	{
		system("rm -f resources/snapdefault.account");
	}
	else
	{
		miscSetFile("resources/snapdefault.account", snap);
	}
}


/*

Выход из всех снапшотов

*/

void snapshotManager::ExitSnapshot()
{
	//разрушает ботов
	for(int i = 0; i < bots.size(); i++)
	{
		delete bots[i];
		std::this_thread::sleep_for(std::chrono::milliseconds(120));
	}

	bots.clear();
	active_snapshot = "";
}


/*

Информация о ботах вк и вкгрупп для сайта

*/

std::string snapshotManager::getBotsInfo(bool official)
{
	std::string result;

	bool f = false;

	//перебираем по ботам
	for(int i = 0; i < bots.size(); i++)
	{
		std::vector<interface*> interf = bots[i]->getAllInterfaces();
		
		for(int j = 0; j < interf.size(); j++)
		{
			if(interf[j]->interface_type == 0 || interf[j]->interface_type == 3)
			{
				networking_vkcom* currentVK = dynamic_cast<networking_vkcom*>(interf[j]);

				if(official != currentVK->interfaceSettings.official) continue;
				if(!currentVK->LoggedIn) continue; //пропускаем незалогиненых
				if(currentVK->selfPageId == "") continue; //пропускаем невалидных

				if(f)
				{
					result += ",\n";
				}

				f = true;
				
				std::string groupFactor = "";
				if(interf[j]->interface_type == 3) groupFactor = "-";

				result += groupFactor+currentVK->selfPageId + " => [ \"official\" => " + std::to_string(currentVK->interfaceSettings.official) 
				+ ",\n \"type\" => " + std::to_string(interf[j]->interface_type) + ",\n \"botname\" => \"" + bots[i]->getName() + std::string("\",\n");

				result += " \"allmsg\" => " + std::to_string(bots[i]->getConveerPtr()->information.message_count()) +
				",\n \"msg1hr\" => " + std::to_string(bots[i]->getConveerPtr()->information.last_time_messages("1 HOUR")) + "]"; 
			}
		}
	}

	return result;
}
