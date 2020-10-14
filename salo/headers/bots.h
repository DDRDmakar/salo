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


#ifndef BOTS
#define BOTS


#define INTERFACE_VKCOM 0
#define INTERFACE_WEBFRONTEND 1
#define INTERFACE_TELEGRAM 2
#define INTERFACE_VKGROUP 3

#include "message.h"
#include "conveer.h"
#include "interface.h"

#include <vector>
#include <thread>
#include <mutex>


class bot
{
private:
	std::string BOT_NAME;
	
	std::vector<interface*> interface_list;
	std::vector<std::thread*> interface_threads;
		
	bool running_state = false;
	bool officalBot = true; //TODO
	bool conveerCreated = false;

	unsigned int interface_counter;

	Convspace::Conveer* conveer;
	std::mutex BOT_INTERNAL_LOCKER;
	bool manualLock = false;
	
public:
	//whoami?
	std::string getName();
	Convspace::Conveer* getConveerPtr();

	//методы для работы с состояниями бота
	void Upstart(); //метод, приводящий к запуску бота
	void Stop(); //метод приводящий к остановке бота
	bool isRunning(); //работает?

	std::string getInfo(bool ln);

	//методы работы с интерфейсами
	interface* getInterfaceByUniqueName(const std::string& Name); 
	interface* getInterfaceBySimpleName(const std::string& Name); 
	int getInterfaceTypeByUniqueName(const std::string& Name);
	int getInterfaceTypeBySimpleName(const std::string& Name);
	std::vector<interface*> getAllInterfaces();
	void createInterface(int type, const std::string& SimpleName, const std::string& SnapshotName, InterfaceSettings set);
	void deleteInterface(const std::string& SimpleName);

	//Метод обработки сообщений ботом
	void OnMessage(const std::string& messageText, Person sender, CONFIGURATION config);

	//Метод обработки событий ботом
	void OnEvent(const std::string& eventType, Person sender, CONFIGURATION config);

	//блокировка конвеера бота
	void lockBotsProcessing();
	void unlockBotsProcessing();
	bool isProcessingLocked();

	//Финансовый показатель
	unsigned long long botsCaptchaCounter;

	bot(const std::string& botname);
	~bot();
};

#endif