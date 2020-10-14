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


#include <mutex>
#include <iostream>
#include "headers/misc.h"
#include "headers/statistics.h"
#include "conveer/headers/information.h"
#include "headers/bots.h"
#include "vk/headers/vkcom.h"

/*

TODO: доделать

*/

Statistics::Statistics()
{
	START = miscUnixTimeNow();
	vkcom_storage.messages_count = 0;
	vkcom_storage.msg_avg_time = 0;
	vkcom_storage.RX_TIMINGS = 0;
	vkcom_storage.TX_TIMINGS = 0;
	web_storage.messages_count = 0;
	web_storage.msg_avg_time = 0;
	web_storage.RX_TIMINGS = 0;
	web_storage.TX_TIMINGS = 0;
	telegram_storage.messages_count = 0;
	telegram_storage.msg_avg_time = 0;
	telegram_storage.RX_TIMINGS = 0;
	telegram_storage.TX_TIMINGS = 0;
	conveerTimings = new Timing();
	databaseTimings = new Timing();
	
	db_query_total = 0;
	db_query_per_msg = 0;
	db_avg_time = 0;

	cache_usage = 0;
	messages_per_fetch = 0;
	fetcher_time = 0;

	all_messages = 0;

	std::cout << std::endl << "SALO Statistics enabled..." << std::endl;
}

Statistics::~Statistics()
{
	delete conveerTimings;
	delete databaseTimings;

	std::cout <<  "SALO Statistics stopping..." << std::endl;
}


/*

Возвращает аптайм

*/

long int Statistics::getUptime()
{
	long NOW = miscUnixTimeNow();

	return NOW - START; 
}


/*

Возвращает строку со статусом

*/

std::string Statistics::getStatusString()
{
	std::string t = "SALO INTELLECT PLATFORM.\n"; 
	t += "\nVersion: " + miscGetStringFromFile("resources/version.txt") + " (build: " + std::string(buildid) + ")\n"; 
	t += "\nUptime: " + miscSecondsToDHMS(getUptime()) + "\n";

	//t += "\nDebug Mode = " + std::to_string(vk->pNetwork->DebugMode) + "\n";
	//t += "Offline Mode = " + std::to_string(vk->pNetwork->OfflineMode) + "\n";

	//t += "\n\nStatistics on VK: \n";
	t += "Messages (session): " + std::to_string(all_messages) + "\n";
	//t += "Messages (total): " + std::to_string(messages_amount()) + "\n";
	t += "Answer generation: " + std::to_string(vkcom_storage.msg_avg_time) + " sec\n";
	
	//t += "\nDatabase statistics:\nDatabase queries in this session: " + std::to_string(db_query_total) + "\n"; 
	//t += "Database queries per message: " + std::to_string(db_query_per_msg) + "\n"; 	
	t += "Database query time: " + std::to_string(db_avg_time) + " sec.\n";

	return t; 
}


/*

Возвращает строку со статистикой 

*/

std::string Statistics::getStatisticsString(void* Bot)
{
	//TODO: делить на сумму всех интерфейсов
	if(vkcom_storage.messages_count != 0)
	{
		db_query_per_msg = db_query_total / all_messages; 
	}
	else
	{
		db_query_per_msg = 0;
	}

	if(Bot == NULL)
	{
		return std::string("Unknown bot");
	}

	bot* thisBot = static_cast<bot*>(Bot);

	std::string t = "\n(bot: "+thisBot->getName()+") Statistics: \n\n";

	t += "Database queries (session): " + std::to_string(db_query_total) + "\n"; 
	t += "Database queries per message: " + std::to_string(db_query_per_msg) + "\n"; 	
	t += "Database query time: " + std::to_string(db_avg_time) + " sec.\n";
	t += "Main database size: " + std::to_string(thisBot->getConveerPtr()->information.typical_question_count()) + " phrases\n";
	t += "All Unique users (all bots/DBs): " + std::to_string(thisBot->getConveerPtr()->information.global_user_count()) + "\n";
	t += "Unsolved messages: " + std::to_string(thisBot->getConveerPtr()->information.unsolved_count()) + "\n";
	t += "Teachmode: " + std::to_string(thisBot->getConveerPtr()->information.unsolved_done_count()) + " messages to be approved by admins\n";
	t += "History size: " + std::to_string(thisBot->getConveerPtr()->information.history_size()) + "\n";
	t += "All media attachments: " + std::to_string(thisBot->getConveerPtr()->information.attachment_count()) + "\n";
	//t += "Media - photo: " + std::to_string(attachments_amount("photo")) + "\n";
	//t += "Media - video: " + std::to_string(attachments_amount("video")) + "\n";
	//t += "Media - audio: " + std::to_string(attachments_amount("audio")) + "\n";
	t += "Captcha Solved: " + std::to_string(thisBot->botsCaptchaCounter) + " times\n";
	t += "Messages (session): " + std::to_string(all_messages) + "\n";


 	return t;
}

/*

Возвращает строку со статистикой по сообщениям

*/

std::string Statistics::getMessagesString(bool web, void* Bot)
{
	std::string ending;
	
	if(web)
	{
		ending = "<br>";
	}
	else
	{
		ending = "";
	}

	if(Bot == NULL)
	{
		return std::string("Unknown bot");
	}

	bot* thisBot = static_cast<bot*>(Bot);

	std::string t = "Messages Statistics:\n\n" + ending + ending;

	t += "VK messages (session): " + std::to_string(vkcom_storage.messages_count) + "\n" + ending;
	//t += "VK messages (all time): " + std::to_string(stable_messages_amount("networking_vkcom")) + "\n" + ending;
	t += "Telegram messages (session): " + std::to_string(telegram_storage.messages_count) + "\n" + ending;
	//t += "Telegram messages (all time): " + std::to_string(stable_messages_amount("networking_telegram")) + "\n" + ending;
	t += "Social Networks messages (session): " + std::to_string(vkcom_storage.messages_count + telegram_storage.messages_count) + "\n" + ending;
	//t += "Social Networks messages (all time): " + std::to_string(stable_messages_amount()) + "\n" + ending;
	t += "Website messages (session): " + std::to_string(web_storage.messages_count) + "\n" + ending;
	//t += "Website messages (all time): " + std::to_string(unstable_messages_amount()) + "\n" + ending;
 	t += "All Messages (session): " + std::to_string(all_messages) + "\n" + ending;
	t += "All Messages (all time): " + std::to_string(thisBot->getConveerPtr()->information.global_message_count()) + "\n" + ending;
	t += "All Messages (5 min): " + std::to_string(thisBot->getConveerPtr()->information.global_last_time_messages("5 MINUTE")) + "\n" + ending;
	t += "All Messages (1 hr): " + std::to_string(thisBot->getConveerPtr()->information.global_last_time_messages("1 HOUR")) + "\n" + ending;
	t += "All Messages (1 day): " + std::to_string(thisBot->getConveerPtr()->information.global_last_time_messages("1 DAY")) + "\n" + ending;

 	return t;
}


/*

Возвращает строку с статистикой конкретного бота

*/

std::string Statistics::getBotStats(void* Bot)
{
	if(Bot == NULL)
	{
		return std::string("Unknown bot");
	}

	bot* thisBot = static_cast<bot*>(Bot);

	std::string t = "Statistics for bot \"" + thisBot->getName() + "\"\n\n";

	t += "All Messages (session): " + std::to_string(thisBot->getConveerPtr()->information.get_total()) + "\n";
	t += "All Messages (all time): " + std::to_string(thisBot->getConveerPtr()->information.message_count()) + "\n";
	t += "All Messages (5 min): " + std::to_string(thisBot->getConveerPtr()->information.last_time_messages("5 MINUTE")) + "\n";
	t += "All Messages (1 hr): " + std::to_string(thisBot->getConveerPtr()->information.last_time_messages("1 HOUR")) + "\n";
	t += "All Messages (1 day): " + std::to_string(thisBot->getConveerPtr()->information.last_time_messages("1 DAY")) + " ("+
	std::to_string(thisBot->getConveerPtr()->information.last_time_commands("1 DAY")) + " cmd's)\n";

	return t;
}


/*

Возвращает строку с информацией о текущей нагрузке на Сало

*/

std::string Statistics::getStressStatusString()
{
	std::string t = "SALO STRESS STATUS\n";
	t += "Data for " + miscGetCurrentDateTime() + "\n";

	t += "Flood Protection is not present in Salo 2... Maybe it will come back later...";
	//t += vk->floodprot->load_status();

	return t;
}


/*

Возвращает строку с информацией о текущей нагрузке на фетчер

*/

std::string Statistics::getFetcherStatusString()
{
	std::string t = "FETCHER STATUS:\n\n";
	t += "Data for " + miscGetCurrentDateTime() + "\n\n";

	t += "Cache usage: " + std::to_string(cache_usage) + " per tick\n";
	t += "Messages parsed: " + std::to_string(messages_per_fetch) + " per tick\n";
	t += "Fetcher time: " + std::to_string(fetcher_time) + " sec.\n";

	return t;
}


std::mutex INCREMENT_MUTEX;


/*

Инкрементит значения, не хранящиеся на постянной основе

*/

void Statistics::IncrementInRAM(int parameter, int interface, double value)
{
	INCREMENT_MUTEX.lock();

	Storage* stor;

	switch(interface)
	{
		case INTERFACE_VKCOM: stor = &vkcom_storage; break;
		case INTERFACE_WEBFRONTEND: stor = &web_storage; break;
		case INTERFACE_TELEGRAM: stor = &telegram_storage; break;
	}

	switch(parameter)
	{
		case MESSAGES_COUNT: stor->messages_count++; all_messages++; break;
		case MESSAGES_TIME_AVG: stor->msg_avg_time += value; stor->msg_avg_time = stor->msg_avg_time / 2; break;
		case QUERYES_TOTAL: db_query_total++;  break; 
		case QUERY_TIME_AVG: db_avg_time += value; db_avg_time = db_avg_time / 2; break;
		case GENERAL_RX_TIMING: stor->RX_TIMINGS += value; stor->RX_TIMINGS = stor->RX_TIMINGS / 2; break;
		case GENERAL_TX_TIMING: stor->TX_TIMINGS += value; stor->TX_TIMINGS = stor->TX_TIMINGS / 2; break;
		case FETCHER_CACHE_USAGE: cache_usage += value; cache_usage = cache_usage / 2; break;
		case FETCHER_MESSAGES_LOAD: messages_per_fetch += value; messages_per_fetch = messages_per_fetch / 2; break;
		case FETCHER_TIME: fetcher_time += value; fetcher_time = fetcher_time / 2000; break; 
	}

	INCREMENT_MUTEX.unlock();
}


/*

Возвращает среднее время конвеера

*/

float Statistics::getConveerAvgTime()
{
	return (vkcom_storage.msg_avg_time);	
}


/*

Начинает замер интервала времени

*/

void Timing::StartCounting()
{
	start = std::chrono::system_clock::now();
}


/*

Заканчивает замер интервала времени

*/

void Timing::EndCounting()
{
	end = std::chrono::system_clock::now();
}


/*

Возвращает затраченное время в секундах

*/

double Timing::getPeriod()
{
	 double elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

	 double res = elapsed_ms / 1000;

	 return res;
}
