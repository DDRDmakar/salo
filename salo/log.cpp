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
#include <cstdlib>
#include <fstream>
#include <mutex>

#include "headers/log.h"
#include "headers/misc.h"
#include "headers/console.h"

std::mutex LOG_MUTEX;
bool logsEnabled = true;

#define LOG_SIZE 256000


/*

Конструктор, создает и инициализирует логи

*/

Log::Log()
{
	if(miscGetStringFromFile("resources/logs_enabled.conf") == "1")
	{
		logsEnabled = true;
	}
	else
	{
		std::cout << "logs disabled by logs_enabled.conf" << std::endl;
		logsEnabled = false;
	}

	if(!logsEnabled) return;

	LOG_MUTEX.lock();

	LOGDATE = miscGetCurrentDateTime();
	LOGFOLDER = "logs/" + LOGDATE;

	const int dir_err = system(std::string(std::string("mkdir -p \"") + LOGFOLDER + "\"").c_str());
	if (dir_err == -1)
		{	
    		std::cout << "Failed to begin log! Check disk and permissions!" << std::endl;
    		ServerAlive = 0;
		}


	LOG_GENERIC = LOGFOLDER + "/generic.log";
	LOG_CONVEER = LOGFOLDER + "/conveer.log";
	LOG_ERROR = LOGFOLDER + "/error.log";

	try
	{
		std::ofstream generic(LOG_GENERIC);
		if(!generic) { throw 0; }
		generic_buffer = "		GENERIC LOG\n	Log started at " + LOGDATE;
		generic_buffer += " \n \n * * * \n \n";
		generic << generic_buffer;
		generic.flush();
		generic.close();

		std::ofstream conveer(LOG_CONVEER);
		if(!conveer) { throw 1; }
		conveer_buffer = "		CONVEER LOG\n	Log started at " + LOGDATE;
		conveer_buffer += " \n \n * * * \n \n";
		conveer << conveer_buffer;
		conveer.flush();
		conveer.close();
		
		/*
		std::ofstream ecmain(LOG_ECMAIN);
		if(!ecmain) { throw 2; }
		ecmain << "No transmissions yet!";
		ecmain.flush();
		ecmain.close();

		std::ofstream ecmaout(LOG_ECMAOUT);
		if(!ecmaout) { throw 3; }
		ecmaout << "No transmissions yet!";
		ecmaout.flush();
		ecmaout.close();
		*/
	}
	catch(int ex)
	{
		std::cout << "Failed to initialize log! Check disk and permissions!" << std::endl;
		ServerAlive = 0;
	}

	ECHO = false;
	ECMA_ECHO = false;


	std::cout << std::endl << "Logging started..." << std::endl;
	std::cout << "Log started: " <<  LOGDATE << std::endl;

	LOG_MUTEX.unlock();
}


/*

Закрывает логи 

*/

Log::~Log()
{
	if(!logsEnabled) return;

	CommitGeneric(F, L, "Log file end.");
	CommitConveer(F, L, "Log file end.");
	LOG_MUTEX.lock();
	std::cout << "Log ended: " << miscGetCurrentDateTime() << std::endl;
	LOG_MUTEX.unlock();
}


/*

Добавляет запись в общий лог

*/

void Log::CommitGeneric(const char* file, const int line, const std::string& message, const std::string& unique)
{
	if(!logsEnabled) return;

	LOG_MUTEX.lock();
	
	std::string identifier = std::string();

	if(unique != "generic")
	{
		identifier = "[unique = " + unique + "] ";
	}

	generic_buffer += "$ [" + miscGetCurrentDateTime() + "] [" + std::string(file) + "@" + std::to_string(line) + "] " + identifier + message + "\n";

	unsigned int buflen = generic_buffer.length();

	if(buflen > LOG_SIZE) 
	{
		generic_buffer = generic_buffer.substr(buflen - LOG_SIZE);
	}

	if(ECHO)
	{
		std::cout << message << std::endl;
	}

	std::ofstream generic(LOG_GENERIC, std::ios_base::out | std::ios_base::trunc);
	generic << generic_buffer;
	generic.flush();
	generic.close();

	LOG_MUTEX.unlock();
}



/*

Добавляет запись в лог конвеера

*/

void Log::CommitConveer(const char* file, const int line, const std::string& message, const std::string& unique)
{
	if(!logsEnabled) return;

	LOG_MUTEX.lock();

	std::string identifier = std::string();

	if(unique != "generic")
	{
		identifier = "[unique = " + unique + "] ";
	}

	conveer_buffer += "$ [" + miscGetCurrentDateTime() + "] [" + std::string(file) + "@" + std::to_string(line) + "] " + identifier + message + "\n";

	unsigned int buflen = conveer_buffer.length();

	if(buflen > LOG_SIZE) 
	{
		conveer_buffer = conveer_buffer.substr(buflen - LOG_SIZE);
	}

	if(ECHO)
	{
		std::cout << message << std::endl;
	}

	std::ofstream conveer(LOG_CONVEER, std::ios_base::out | std::ios_base::trunc);
	conveer << conveer_buffer;
	conveer.flush();
	conveer.close();

	LOG_MUTEX.unlock();
}


/*

Обновляет записи взаимодействия с vk



void Log::Update(int out, const std::string& message)
{
	if(!out)
	{
		std::ofstream ecmain(LOG_ECMAIN);
		ecmain << message;
		ecmain.flush();
		ecmain.close();

		if(ECMA_ECHO) 
		{
			std::cout << message << std::endl;
		}
	}
	else
	{
		std::ofstream ecmaout(LOG_ECMAOUT);
		ecmaout << message;
		ecmaout.flush();
		ecmaout.close();

		if(ECMA_ECHO)
		{
			std::cout << message << std::endl;
		}
	}
}
*/


/*

Переключает режим вывода в консоль для Generic и Conveer логов

*/

void Log::TriggerEcho()
{
	if(!logsEnabled) return;
	
	ECHO = !ECHO;
	std::cout << "Log echoeing mode = " << ECHO << std::endl;
}


/*

Переключает режим вывода в консоль для логов взаимодействия с vk



void Log::TriggerEcmaEcho()
{
	ECMA_ECHO = !ECMA_ECHO;
	std::cout << "ECMA echoeing mode = " << ECMA_ECHO << std::endl;
}

*/