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
#include <fstream>
#include <thread>
#include <chrono>
#include <signal.h>

#include "headers/console.h"
#include "headers/misc.h"
#include "headers/message.h"
#include "headers/security.h"
#include "headers/database.h"
#include "headers/statistics.h"
#include "headers/log.h"
#include "headers/translate.h"
#include "conveer/headers/conveer.h"
#include "headers/snapshots.h"
#include "headers/scrs.h"
#include "headers/config.h"

#include "vk/headers/vkcom.h"
#include "telegram/headers/telegram.h"
#include "web/headers/web.h"

#include "headers/bots.h"

#define noop

int ServerAlive; //extern int - определен в headers/console.h
dbController* database;
Statistics* statistics;
Log* logs;
Translate* translate;
Convspace::Config *workdat;
snapshotManager* snapManager;
SecurityGeneric* sec;
std::stack<AnswerError> ErrorStack;
std::string PathToBinary;


/*

Обработчик сигнала прерывания (Signal Interrupt)

*/

static void onTerminationSignal(int sig)
{
	std::cout << "Using SIGINT is unsafe, use console command \"exit\" instead"
			<< std::endl << "If salo process is unresposible, use unix kill\n"; 
}


/*

Обработчик сигнала истечения времени выполнения (Signal Alarm)

*/

static void onAlarmSignal(int sig)
{
	//No Operation
	//Dummy handler
}


/*

Функция main 

*/

int main(int argc, char** argv)
{
	ServerAlive = 1;
	setlocale(LC_ALL, "ru_RU.UTF-8");
	
	if(argc == 2)
	{
		if (strcmp(argv[1], "-unsafe") == 0) 
		{
			std::cout << miscFormatted(miscFormatted("[DEBUG]", 3), 1) << " Skipping process name check to allow using of GDB and mtrace" << std::endl;
		}
		else
		{
			miscCheckExecName(argv[0]);
		}
	}
	else
	{
		miscCheckExecName(argv[0]);
	}
	
	std::string greeting = "\nSALO INTELLECT PLATFORM SERVER v" + miscGetStringFromFile("resources/version.txt");
	greeting += " (build " + std::string(buildid) + ")\n\n";
	greeting = miscFormatted(miscFormatted(greeting, 2), 1);
	std::cout << greeting;
	std::cout << "Compilation timestamp: " << __DATE__ << " " << __TIME__ << std::endl;


	//watchdog support
	bool havePid = false;
	int pid = 0;
	if(argc > 1) 
	{
		if(strcmp(argv[1], "-pid") == 0)
		{
			havePid = true;
			pid = atoi(argv[2]);
			std::cout << "Registered as watchdog process ---> pid = " << pid << std::endl;
		}
	}
	//watchdog support

	logs = new Log();
	SecurityGeneric* genericSecProv = new SecurityGeneric("resources/generic.account");
	sec = genericSecProv;
	statistics = new Statistics();
	database = new dbController(genericSecProv);
	translate = new Translate();
	workdat = new Convspace::Config( "GENERIC" );
	
	PathToBinary = std::string(argv[0]).substr(0, std::string(argv[0]).find_last_of("/") + 1);

	logs->CommitGeneric(F, L, "Starting loops...");

	signal(SIGINT, onTerminationSignal);
	signal(SIGALRM, onAlarmSignal);

	//SALO 2 behaviors
	
	scrs_connect();

	snapManager = new snapshotManager();
	
	if(miscFileExists("resources/snapdefault.account"))	
	{
		std::string snap = miscGetStringFromFile("resources/snapdefault.account");
		snapManager->LoadSnapshot(snap);
	}

	//SALO 2 behaviors

	std::thread console_thread(server_console); //Консоль сервера работает в отдельном потоке
	console_thread.join(); //Ожидать завершения потока консоли
	

	delete snapManager;
	delete database;
	delete statistics;
	delete genericSecProv;
	delete workdat;
	delete logs;

	if(havePid)
	{
		std::ofstream watchdogpid("resources/watchdog.pid");
		watchdogpid << std::to_string(pid);
		watchdogpid.flush();
		watchdogpid.close();
	}
	
	std::cout << "SALO Server shutting down now!" << std::endl;
	return 0;
}
