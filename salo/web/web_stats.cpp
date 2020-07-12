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
