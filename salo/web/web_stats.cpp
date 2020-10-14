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
#include <fstream>


#include "headers/web.h"

#include "../headers/statistics.h"
#include "../headers/bots.h"
#include "../headers/snapshots.h"
#include "../headers/scrs.h"
#include "../headers/misc.h"


/*

Обновление статистики

*/

void webinterface::updateStats()
{
	SERVICE_MUTEX.lock();

	std::string stats = "<?php ";

	stats += "$stats_varsstring = \"";
	stats += statistics->getMessagesString(true, thisBot);
	stats += "\";";
	stats += "\n\n";

	if(snapManager != NULL)
	{
		stats += "$ofbots = [" + snapManager->getBotsInfo(true) + "];\n";
		stats += "\n\n";
		stats += "$usbots = [" + snapManager->getBotsInfo(false) + "];\n";
	}

	stats += "\n";

	bot* saloBot = static_cast<bot*>(thisBot);

	unsigned int allusers = saloBot->getConveerPtr()->information.global_user_count();

	stats += "$platformusers = " + std::to_string(allusers) + "; \n";
	stats += "$platformdbsize = " + std::to_string(saloBot->getConveerPtr()->information.typical_question_count()) + "; \n";

	stats += "\n";

	stats += "?>";

	std::ofstream out("/var/salowebcache/stats");
	out << stats;
	out.flush();
	out.close();

	///------------------------ обновляется шапка

	std::string line1 = "Сообщений за день: " + std::to_string(saloBot->getConveerPtr()->information.global_last_time_messages("1 DAY"));
	std::string line2 = "Сообщений за час: " + std::to_string(saloBot->getConveerPtr()->information.global_last_time_messages("1 HOUR"));
	std::string line3 = "Уникальных пользователей: " + std::to_string(allusers);
	std::string line4 = "Аптайм: " + miscSecondsToRusDHM(statistics->getUptime());
	std::string line5 = miscGetFile("resources/line5.txt", 1);
	std::string strings = "{ \"line1\":\""+line1+"\", \"line2\":\""+line2+"\",\"line3\":\""+line3+"\",\"line4\":\""+line4+"\",\"line5\":\""+line5+"\" }";
	ResponseData empty;
	scrs("infoheader", strings, empty);

	std::cout << "Service thread done statistics refresh...\n";

	SERVICE_MUTEX.unlock();	
}
