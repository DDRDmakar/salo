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