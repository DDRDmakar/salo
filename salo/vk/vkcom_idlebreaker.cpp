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

#include "headers/vkcom.h"
#include "../headers/database.h"
#include "../headers/message.h"
#include "../headers/log.h"
#include "../headers/misc.h"
#include "../headers/bots.h"
//#include "../headers/conveer_information.h"
#include <iostream>
#include <fstream>
#include <chrono>

/*

Код, вызывающийся при 5000 пустых тиков

*/

void networking_vkcom::idlebreaker()
{
	std::cout << "IDLE BREAKER sending messages!" << std::endl;
	logs->CommitGeneric(F, L, "idlebreaker called");

	srand(time(NULL));

	DatabaseResult users = database->database_returnQuery("SELECT `id` FROM `user` WHERE `messages_sent` > 120");

	int index = rand() % users.RowStrings.size();  
	std::string struserid = users.RowStrings[index];
	BIGINT userid = atoll(struserid.c_str() + 17);


	DatabaseResult messages = database->database_returnQuery("SELECT `initial_phrase` FROM `initialphrases_idle`");
	std::string msgtxt = messages.RowStrings[rand() % messages.RowStrings.size()];

	if(enable_idlebreaker && /*last_time_messages(1, "DAY") TODO */ 1000 < 300)
	{
		//SendMessageVK(msgtxt, userid);
		AlertDevelopers(
				std::string("IDLE BREAKER\n\nSalo detected idle, and successfully send idle-breaking message\n\n") +
				std::string("Debugging info about idle-breaking message: \n") +
				std::string("Send it to: id") + std::to_string(userid) + std::string("\n") +
				std::string("Message text: ") + msgtxt + std::string("\n\n") + 
				std::string("You can use console command to disable it.")
			);
	}
	else
	{
		AlertDevelopers(
				std::string("IDLE BREAKER\n\nSalo detected idle, and formed idle-breaking message, but IDLE BREAKER disabled or more than 300 messages send past day. \n\n") + 
				std::string("Debugging info about idle-breaking message: \n") +
				std::string("Send it to: id") + std::to_string(userid) + std::string("\n") +
				std::string("Message text: ") + msgtxt + std::string("\n\n") + 
				std::string("You can use console command to re-enable it.")
			);
	}

	idle_timer = 0;
}


/*

Код вызывающийся каждый тик, для рассылки поздравлений с днем рождения

*/

void networking_vkcom::bdater()
{
	time_t mytime;
	mytime = time(NULL);
	tm* mtm = localtime(&mytime);
	
	std::string today = std::to_string(mtm->tm_mday);
	std::string tomonth = std::to_string(mtm->tm_mon+1);

	bot* Bot = static_cast<bot*>(this->thisBot);
	std::string path = "bots/"+this->SnapshotName+"/"+Bot->getName()+"/i0_"+this->INTERFACE_SIMPLE_NAME+".datetime";

	if((!this->LoggedIn) || (this->selfPageId == ""))
	{
		//bot is not logged in, go away!
		return;
	}

	std::string todayfile = miscGetStringFromFile(path.c_str());

	if(todayfile != today)
	{
		if(this->interfaceSettings.ignoreBirthdays == 1)
		{
			//Наступил новый день
			logs->CommitGeneric(F, L, "bdater detected new day, but bdater is supressed");

			//Наступил новый день
			logs->CommitGeneric(F, L, "bdater detected new day");
			std::cout << "bdater detected new day: " << today << "." << tomonth << std::endl << std::endl;

			std::ofstream out(path);
			out << today;
			out.flush();
			out.close();

			//Taкже в 12 ночи разбаниваем пользователей
			std::string rr = this->forceBanlistRefreshPy(Bot->getName(), Bot->getConveerPtr()->get_database_name());
			AlertDevelopers("Salo 2 everyday unbanner (bdater is supressed!!!): \n\n" + rr + "\n\n Triggered by: " + INTERFACE_SIMPLE_NAME + " (" + path + ")");
			return;
		}


		//Наступил новый день
		logs->CommitGeneric(F, L, "bdater detected new day");
		std::cout << "bdater detected new day: " << today << "." << tomonth << std::endl << std::endl;

		std::ofstream out(path);
		out << today;
		out.flush();
		out.close();

		std::string congrat = "Привет друг!\n\nПоздравляю тебя с Днем Рождения! Желаю тебе хорошего настроения и хорошего здоровья! Сегодня твой день!\n\nИскренне ваши - боты проекта Платформа Сало Интеллект!\n";
		congrat = pNetwork->urlencode(congrat);

		std::string apiurl = "https://api.vk.com/method/friends.get?fields=bdate&v=5.53&access_token=" + VKONTAKTE_ACCESS_TOKEN;
		NetworkResult res = pNetwork->network_HTTPS_GET(apiurl.c_str());

		std::vector<std::string> ids = findBirthdateUsers(res.DATA, today, tomonth);
		std::string ids_list;

		int cel = 0;

		//Рассылка оповещений
		for(int i = 0; i < ids.size(); i++)
		{
				std::string api = "https://api.vk.com/method/messages.send?peer_id=" + ids[i]
								+ "&message=" + congrat + pNetwork->urlencode(std::to_string(i)) + "&v=5.53&access_token="+VKONTAKTE_ACCESS_TOKEN;
				NetworkResult bdres = pNetwork->network_HTTPS_GET(api.c_str());
				std::string errortext = GetError(res.DATA);

				if(errortext == "Captcha needed")
				{
					if(this->interfaceSettings.anticaptchaEnabled)
					{
						bool cres = captcha(res.DATA);

						if(cres)
						{
							std::string apiurl = "https://api.vk.com/method/messages.send";
							std::string postkey = "peer_id="+ ids[i] + "&message=" + congrat + pNetwork->urlencode(std::to_string(i)) 
								+ "&captcha_sid="+ captcha_sid +"&captcha_key="+ captcha_answer + "&v=5.53&access_token="+VKONTAKTE_ACCESS_TOKEN;

							pNetwork->network_HTTPS_POST(apiurl.c_str(), postkey.c_str(), "(none)", 0);
						}
						else
						{
							//logs->CommitGeneric(F, L, "Captcha failed!!!\n\nError Text = " + res.DATA);
							AlertDevelopers("Captcha in bdater, but anti-captcha failed.");
						}
					}
					else
					{
						AlertDevelopers("Captcha in bdater, but anti-captcha is disabled by interface atrributes.");
					}
				}
				else
				{
					AlertDevelopers("BDATER!\n\nError: " + errortext + "\n\n");
				}


				std::this_thread::sleep_for(std::chrono::milliseconds(768)); //1000 ms sleep

				ids_list += ("id" + ids[i] + "\n");
				cel++; 
		}
		

		std::cout << "\nSalo 2 Birthdate Congratulation service detected new day.\n\nBirthdate messages sent: " 
				+ std::to_string(cel) << "\n\n" << ids_list << std::endl;
		AlertDevelopers("Salo 2 Birthdate Congratulation service detected new day.\n\nBirthdate messages sent: " 
			+ std::to_string(cel) + "\n\nUsers:\n" + ids_list + "\n\nBdater Triggered by: " + INTERFACE_SIMPLE_NAME + " (" + path + ")");


		//Taкже в 12 ночи разбаниваем пользователей
		std::string rr = this->forceBanlistRefreshPy(Bot->getName(), Bot->getConveerPtr()->get_database_name());
		AlertDevelopers("Salo 2 everyday unbanner: \n\n" + rr + "\n\n Triggered by: " + INTERFACE_SIMPLE_NAME + " (" + path + ")");
		return;
	}
	else
	{
		//Новый день еще не наступил
		//std::cout << "nope\n";
		return;
	}

	return;
}