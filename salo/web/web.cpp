/*
*
*  Copyright ©, 2015-2020. All Rights Reserved.
*
*    Autors:
*    Motylenok Mikhail
*    Makarevich Nikita
*
*    This code is privately owned and is a commercial secret. We do not provide
*    code to anyone without the written agreement. Copying, publication, use
*    for commercial or non-commercial purposes without the consent
*    of the authors is a violation of applicable law.
*
*/

#include <iostream>
#include <dirent.h>
#include <fstream>
#include <thread>
#include <chrono>
#include "headers/web.h"
#include "../headers/console.h"
#include "../headers/misc.h"
#include "../headers/log.h"
#include "../headers/statistics.h"
#include "../headers/bots.h"


webinterface::webinterface()
{
	SERVICE_MUTEX.lock();
	std::cout << "Web frontend listening started..." << std::endl;
	k = 10000;
	enabled = true;
	INTERFACE_IDENTIFICATOR = "web_and_saloapi";
	std::cout << "Unique identifier = " << INTERFACE_IDENTIFICATOR << std::endl;
	SERVICE_MUTEX.unlock();
}

webinterface::~webinterface()
{
	SERVICE_MUTEX.lock();
	
	serviceThread.join();
	enabled = false;

	std::cout << "Web frontend listening stopped..." << std::endl;
	
	SERVICE_MUTEX.unlock();
}


/*

Цикл обновления потока веб-фронтенда

*/

void webinterface::Tick()
{
	std::cout << miscFormatted(miscFormatted("[SPAWN]", 2), 1) 
				<< " WEB: Thread started. \n";

	while(ServerAlive && this->enabled && *state)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(120)); //120 ms sleep

		this->iStatistics->iTiming.StartCounting();
		
			this->OnUpdate();
		
		this->iStatistics->iTiming.EndCounting();
		
		if(this->iStatistics->iTiming.getPeriod() > 0.001) //слишком маленькие величины усредняют счетчик к нулю.
		{
			this->iStatistics->iStorage.TX_TIMINGS += this->iStatistics->iTiming.getPeriod();
			this->iStatistics->iStorage.TX_TIMINGS /= 2; 
			this->iStatistics->iStorage.RX_TIMINGS = this->iStatistics->iStorage.TX_TIMINGS;
			//statistics->IncrementInRAM(GENERAL_RX_TIMING, INTERFACE_WEBFRONTEND, statistics->saloapiGeneralTimings->getPeriod()); 
		}

		//std::cout << statistics->saloapiGeneralTimings->getPeriod() << std::endl;
	}

	std::cout << miscFormatted(miscFormatted("[KILL]", 3), 1) 
				<< " WEB: Update function reached end: Thread terminated. \n";

	thisState = false;
}


/*

Получаем сообщения с веб-интерфейса

*/

bool webinterface::ProcessMessage(const std::string& fname)
{
	if(fname == "." || fname == ".." || fname == "" || fname == "outcoming" || fname == "stats" || fname == "resources")
	{
		return false;
	}

	std::string fullfname = std::string("/var/salowebcache/") + fname;

	//std::cout << fullfname << std::endl;
	std::string content = miscGetFile(fullfname.c_str());
	//std::cout << content << std::endl;
	std::remove(fullfname.c_str());


	std::string rHash = std::string(fname);

	char* ip = miscFindAndCopy(content.c_str(), "ip=\"", "\"");
	char* agent = miscFindAndCopy(content.c_str(), "agent=\"", "\"");
	char* cid = miscFindAndCopy(content.c_str(), "cid=\"", "\"");
	char* txt = miscFindAndCopy(content.c_str(), "text=\"", "\"");
	char* api = miscFindAndCopy(content.c_str(), "API@@_", "__@@");

	bool isApi = false;

	if(api == NULL)
	{
		isApi = false;
	}
	else
	{
		isApi = true;
	}

	std::string rIp;
	std::string rAgent;
	std::string rCid;
	std::string rText;
	std::string rApi;

	if(ip != NULL) { rIp = std::string(ip); delete[] ip; } // else { error }
	if(agent != NULL) { rAgent = std::string(agent); delete[] agent; }
	if(cid != NULL) { rCid = std::string(cid); delete[] cid; }
	if(txt != NULL) { rText = std::string(txt); delete[] txt; } 
	if(api != NULL) { rApi = std::string(api); delete[] api; }

	if(ip == NULL || agent == NULL || txt == NULL || cid == NULL)
	{
		return false;
	}
	
	Person p;

	if(!isApi) //если не Api значит веб-версия
	{
		p.userID = 0;
		p.Name = "WebUser";
		p.Last = "WebUser";
		p.IDENTIFICATOR = rHash; 
	}
	else
	{
		p.userID = 0;
		p.Name = "ApiUser";
		p.Last = "ApiUser";
		p.IDENTIFICATOR = rApi; 
	}

	CONFIGURATION c;

	if(!isApi)
	{
		c.BOT = this->thisBot;
		c.INTERFACE = "webfrontend";
		c.INTERFACE_UNIQUE_IDENTIFICATOR = this->INTERFACE_IDENTIFICATOR;
		c.isConf = false;
		c.confId = 0;
		c.attachment = "null";
		c.web_userhash = rHash;
		c.web_ip = rIp;
		c.web_useragent = rAgent;
	}
	else
	{
		c.BOT = this->thisBot;
		c.INTERFACE = "SaloAPI";
		c.INTERFACE_UNIQUE_IDENTIFICATOR = this->INTERFACE_IDENTIFICATOR;
		c.isConf = false;
		c.confId = 0;
		c.attachment = "null";
		c.web_userhash = rHash;
		c.web_ip = rIp;
		c.web_useragent = rAgent;
	}

	bot* BOT = static_cast<bot*>(this->thisBot);
	BOT->OnMessage(rText, p, c);

	if(this->interfaceAnswerWeb.HaveMessage)
	{
		this->interfaceAnswerWeb.cid = rCid;
	}

	PushMessage();

	return true;
}


/*

Отправляем их назад

*/

void webinterface::PushMessage()
{
	
	//database->database_simpleQuery("INSERT INTO `SALODATABASE`.`web_cache` (`hash`, `message_text`, `cid`) VALUES ('" 
	//	+ currentAnswerWeb->userHash + "', '" + currentAnswerWeb->messageText + "', '" + currentAnswerWeb->cid + "')");
	
	std::string InputText = std::string(this->interfaceAnswerWeb.messageText);
	std::string FinalText;

	for(int i = 0; i < InputText.length(); i++)
	{
		if(InputText[i] != '\n')
		{
			FinalText.push_back(InputText[i]);
		}
		else
		{
			FinalText += "<br>";
		}
	}

	std::string outfname = std::string("/var/salowebcache/outcoming/__") + this->interfaceAnswerWeb.userHash + "_" + this->interfaceAnswerWeb.cid; 

	std::ofstream out(outfname);
	out << FinalText;
	out.flush();
	out.close();

	this->interfaceAnswerWeb.HaveMessage = false;
}


/*

Функция обновления

*/

void webinterface::OnUpdate()
{

	if(k >= 10000)
	{
		if(!serviceThread.joinable())
		{
			serviceThread = this->spawn_updater();
		}
		else
		{
			serviceThread.join();
			serviceThread = this->spawn_updater();
		}

		//updateStats();	
		k = 0;
	}
	else
	{
		k++;
	}


	if(this->interfaceAnswerWeb.HaveMessage)
	{
		PushMessage();
	}
	else
	{
		struct dirent** namelist;
		
	    int n = scandir("/var/salowebcache", &namelist, 0, alphasort);
	    if (n < 0)
	    {
	    	logs->CommitGeneric(F, L, "Error during directory lookup");
	        //perror("scandir");
	        //ERROR ERROR ERROR
	    }
	    else 
	    {
	    bool res = false;

		    for (int i = 0; i < n; i++) 
	        	{
		           	//printf("%s\n", namelist[i]->d_name);
		        	if(!res)
		        	{ 
		        		res = ProcessMessage(namelist[i]->d_name);
		        	}

		           	free(namelist[i]);
	            }
	    }
	    free(namelist);
	}
}
