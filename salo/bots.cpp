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

#include <chrono>
#include <functional>

#include "headers/bots.h"
#include "headers/misc.h"
#include "conveer/headers/exception.h"

//Классы интерфейсов (наследники interface)
#include "vk/headers/vkcom.h"
#include "telegram/headers/telegram.h"
#include "web/headers/web.h"



//Спавнит нового бота
bot::bot(const std::string& botname)
	: BOT_NAME(botname)
{
	std::cout << miscFormatted(miscFormatted("[SPAWN]", 2), 1) 
				<< " Spawned new bot \"" << BOT_NAME << "\"" << std::endl;

	botsCaptchaCounter = 0;
	interface_counter = 0;

	try
	{
		conveer = new Convspace::Conveer(BOT_NAME);
		conveerCreated = true;
	}
	catch(Convspace::Exception e)
	{
		std::cout << "COVEER EXCEPTION " << e.what() << std::endl;
		AlertDevelopers("CONVEER EXCEPTION in bot::bot!\n\nConveer says: " + e.what() + "\n\nTriggered by: " + BOT_NAME);
		conveerCreated = false;
	}

	if(conveer == NULL)
	{
		std::cout << "COVEER IS NULL" << std::endl;
		conveerCreated = false;
	}

	std::cout << miscFormatted(miscFormatted("[SPAWN]", 2), 1) 
			 	<< " Spawned new conveer for bot \"" << BOT_NAME << "\"" << std::endl;
}


//удаляет бота
bot::~bot()
{
	this->Stop();

	//Удаляет созданные интерфейсы
	for(int i = 0; i < interface_list.size(); i++)
	{
		std::string simple = interface_list[i]->INTERFACE_SIMPLE_NAME;
		std::string unique = interface_list[i]->INTERFACE_IDENTIFICATOR;
		
		delete interface_list[i];
		
		std::cout << miscFormatted(miscFormatted("[KILL]", 3), 1) 
			<< " Killed interface \"" << simple << "\" (unique: " << 
				unique << ") on bot \"" << BOT_NAME << "\"" << std::endl;
	}

	//Удаляет местный конвеер
	delete conveer;

	std::cout << miscFormatted(miscFormatted("[KILL]", 3), 1) 
			 	<< " Killed conveer on bot \"" << BOT_NAME << "\"" << std::endl;

	std::cout << miscFormatted(miscFormatted("[KILL]", 3), 1) 
			 	<< " Killed bot \"" << BOT_NAME << "\"" << std::endl;
}



/*

Инициализатор интерфейсов

*/

void globalThreadInit(interface* interf)
{
	interf->Tick();
}


/*

Метод создает нового бота, инциализирует его, и помещает в хранилище

*/

void bot::createInterface(int type, const std::string& SimpleName, const std::string& SnapshotName, InterfaceSettings set)
{
	if(!conveerCreated)
	{
		std::cout << miscFormatted(miscFormatted("[SPAWN]", 3), 1) << " Conveer error (null or exception), no interface will be created.\n"; 
		return;
	}


	for(int i = 0; i < interface_list.size(); i++)
	{
		if(interface_list[i]->INTERFACE_SIMPLE_NAME == SimpleName)
		{
			std::cout << miscFormatted(miscFormatted("[SPAWN]", 3), 1) << " Name already exists, aborting.\n"; 
			return;
		}

	}

	interface* tmp;

	switch(type)
	{
		case INTERFACE_VKCOM: tmp = new networking_vkcom(this, SimpleName, SnapshotName, false, set); break;
		case INTERFACE_TELEGRAM: tmp = new networking_telegram(this, SimpleName, SnapshotName); break;
		case INTERFACE_WEBFRONTEND: tmp = new webinterface(); break;
		case INTERFACE_VKGROUP: tmp = new networking_vkcom(this, SimpleName, SnapshotName, true, set); break;
		default: return; break;
	}

	tmp->interface_id = interface_counter;
	tmp->INTERFACE_SIMPLE_NAME = SimpleName;
	tmp->thisBot = this;
	tmp->state = &running_state;
	tmp->iStatistics = new interfaceStatistics();
	tmp->interface_type = type;
	tmp->SnapshotName = SnapshotName;
	tmp->interfaceSettings = set;

	interface_counter++;

	interface_list.push_back(tmp);
	
	std::cout << miscFormatted(miscFormatted("[SPAWN]", 2), 1) 
				<< " Spawned new interface \"" << SimpleName << "\" (unique: " << 
					tmp->INTERFACE_IDENTIFICATOR << ") on bot \"" << BOT_NAME << "\"\n";
}


/*

Поиск интерфейса по уникальному имени

*/

interface* bot::getInterfaceByUniqueName(const std::string& Name)
{
	for(int i = 0; i < interface_list.size(); i++)
	{
		if(interface_list[i]->INTERFACE_IDENTIFICATOR == Name)
		{
			return interface_list[i];
		}
	}

	return NULL;
}


/*

Поиск интерфейса по простому имени

*/

interface* bot::getInterfaceBySimpleName(const std::string& Name)
{
	for(int i = 0; i < interface_list.size(); i++)
	{
		if(interface_list[i]->INTERFACE_SIMPLE_NAME == Name)
		{
			return interface_list[i];
		}
	}

	return NULL;
}


/*

Возвращает тип интерфейса по заданному имени

*/

int bot::getInterfaceTypeByUniqueName(const std::string& Name)
{
	interface* t = getInterfaceByUniqueName(Name);
	if(t == NULL) return -1;
	return (t->interface_type);
}


/*

Возвращает тип интерфейса по заданному имени

*/

int bot::getInterfaceTypeBySimpleName(const std::string& Name)
{
	interface* t = getInterfaceBySimpleName(Name);
	if(t == NULL) return -1;
	return (t->interface_type);
}


/*

Удаление интерфейса

*/

void bot::deleteInterface(const std::string& SimpleName)
{
	interface* tbd = getInterfaceBySimpleName(SimpleName);
	if(tbd == NULL) { std::cout << "Not found.\n"; return; }

	tbd->thisState = false;
	tbd->state = &tbd->thisState;
	tbd->toBeDeleted = true;
}


/*

Старт бота после инициализации

*/

void bot::Upstart()
{
	if(!conveerCreated)
	{
		std::cout << miscFormatted(miscFormatted("[SPAWN]", 3), 1) << " Conveer error (null or exception), bot will not start.\n"; 
		running_state = false;
		return;
	}

	std::cout << miscFormatted(miscFormatted("\n[UPSTART]", 2), 1) 
				<< " Activated bot \"" << BOT_NAME << "\"\n";

	running_state = true;

	for(int i = 0; i < interface_list.size(); i++)
	{
		std::thread* tmp_trd;
		//std::function<void(interface*)> thread_func = [](interface* lambdtmp){ lambdtmp->Tick(); };
		tmp_trd = new std::thread(std::bind(globalThreadInit, interface_list[i]));
		interface_list[i]->thisState = true;
		interface_threads.push_back(tmp_trd);
		std::this_thread::sleep_for(std::chrono::milliseconds(120));
	}

	
}


/*

Возвращает true если бот был инциализирован и запущен

*/

bool bot::isRunning()
{
	return running_state;
}


/*

Остановка Бота

*/

void bot::Stop()
{
	std::cout << miscFormatted(miscFormatted("[STOPPING]", 3), 1) 
			 	<< " Stopping bot \"" << BOT_NAME << "\"" << std::endl;

	if(isProcessingLocked()) unlockBotsProcessing();
	running_state = false;

	for(int i =0; i < interface_threads.size(); i++)
	{
		if(interface_threads[i]->joinable())
		interface_threads[i]->join();
		delete interface_threads[i];
		interface_threads.erase(interface_threads.begin() + i);
	}

	for(int i = 0; i < interface_list.size(); i++)
	{
		interface_list[i]->thisState = false;

		if(interface_list[i]->toBeDeleted)
		{
			delete interface_list[i];
			interface_list.erase(interface_list.begin() + i);
		}
	}

}


/*

Возвращает имя бота

*/

std::string bot::getName()
{
	return BOT_NAME;
}


/*

Возвращает указатель на конвеер бота

*/

Convspace::Conveer* bot::getConveerPtr()
{
	return conveer;
}


/*

Функция возвращает строку состояния интерфейса

*/

std::string interface::getInfo()
{
	std::string t;
	std::string attr;

	if(interfaceSettings.official)                   attr += "O"; else attr += "-";
	if(interfaceSettings.anticaptchaEnabled)         attr += "A"; else attr += "-";
	if(interfaceSettings.anticaptchaApiKey.length()) attr += "K"; else attr += "-";
	if(interfaceSettings.alertdevelopersEnabled)     attr += "D"; else attr += "-";
	if(interfaceSettings.vkgroupId.length())         attr += "G"; else attr += "-";
	if(interfaceSettings.vkgroupMemberscheckDelay)   attr += "M"; else attr += "-";
	if(interfaceSettings.ignoreFriendsReq)           attr += "F"; else attr += "-";
	if(interfaceSettings.ignoreBirthdays)            attr += "B"; else attr += "-";
	if(interfaceSettings.confchatsOnly)              attr += "I"; else attr += "-";
	if(interfaceSettings.customGroupLink.length())   attr += "L"; else attr += "-";
	if(interfaceSettings.useCallbackApiConnector)    attr += "C"; else attr += "-";
	if(interfaceSettings.personalPromotion.length()) attr += "P"; else attr += "-";
	
	t += std::to_string(interface_id) + "\t";
	if(thisState) t += miscFormatted("start/running\t", 2);
	else t += miscFormatted("stopped/killed\t", 3);
	t += miscStringDouble(iStatistics->iStorage.RX_TIMINGS, 3)+"s\t";
	t += miscStringDouble(iStatistics->iStorage.TX_TIMINGS, 3)+"s\t";
	t += miscStringDouble(iStatistics->iStorage.msg_avg_time, 3)+"s\t";
	t += miscUpscale(std::to_string(iStatistics->iStorage.messages_count), 10)+"\t";
	t += attr + "\t";
	t += miscUpscale(INTERFACE_IDENTIFICATOR, 22)+"\t";
	t += INTERFACE_SIMPLE_NAME+"\t";
	return t; 
};


/*

Функция возвращает таблицу состояния бота

*/

std::string bot::getInfo(bool ln)
{
	std::string t;
	std::string l;

	if(ln) l = " |"; else l = "  ";
	
	t += miscFormatted(miscFormatted(">>> BOT: ", 4), 1) + "Name: \"" + BOT_NAME + "\"; Status: ";
	if(running_state) t += miscFormatted("RUNNING", 2); else t += miscFormatted("STOPPED", 3);
	if(manualLock) t += miscFormatted(" (PROCESSING PAUSED!)", 3);
	t += "; Interfaces: " + std::to_string(interface_list.size());
	t += "; Threads: " + std::to_string(interface_threads.size()) + "; \n";
	t += l+"      |  id\tThread Status\tRX\tTX\tConveer\tMessages Sent\tAttribute\tUnique Identificator\tSimple Name\n";

	for(int i = 0; i < interface_list.size(); i++)
	{
		if(i == interface_list.size()-1)
			t += l+"      └─  ";
		else
			t += l+"      ├─  ";
		t += interface_list[i]->getInfo();
		t += "\n";
	}

	return t;
}


/*

Функция блокирует конвеер 

*/

 void bot::lockBotsProcessing()
 {
 	BOT_INTERNAL_LOCKER.lock();
 	manualLock = true;
 	return;
 }


/*

Функция разблокирует конвеер 

*/

 void bot::unlockBotsProcessing()
 {
 	BOT_INTERNAL_LOCKER.unlock();
 	manualLock = false;
 	return;
 }


/*

Возвращает True если конвеер заблокирован

*/

bool bot::isProcessingLocked()
{
	return manualLock;
}


/*

Возвращает вектор из всех интерфейсов

*/

std::vector<interface*> bot::getAllInterfaces()
{
	return interface_list;
}
