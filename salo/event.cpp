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
#include "headers/bots.h"
#include "vk/headers/vkcom.h"
#include "telegram/headers/telegram.h"

/*

Первое ли это сообщение?

*/

bool user_is_new(const std::string &userID, const std::string& botname)
{
	std::string dbname = "SALODATABASE";
	//if (BOT_NAME == "salo" || BOT_NAME == "shava" || BOT_NAME == "kim" || BOT_NAME == "doshik" || BOT_NAME == "dedmoroz")
	//	dbname = "SALODATABASE";
	if (botname == "maryana") dbname = "MARYANA";
	if (botname == "grib") dbname = "GRIBDATABASE";
	if (!botname.empty() && botname.substr(0, 10) == "commercial") dbname = "COMMERCIALBOTS";
	std::vector<std::string> res = database->database_returnQuery("SELECT COUNT(*) FROM `"+dbname+"`.`user` WHERE `id` = '"+userID+"';").RowStrings;
	if (!res.empty() && res[0] != "0" && res[0] != "") return false;
	res = database->database_returnQuery("SELECT COUNT(*) FROM `"+dbname+"`.`web_user` WHERE `id` = '"+userID+"';").RowStrings;
	if (!res.empty() && res[0] != "0" && res[0] != "") return false;
	res = database->database_returnQuery("SELECT COUNT(*) FROM `"+dbname+"`.`user_reserve` WHERE `id` = '"+userID+"';").RowStrings;
	if (!res.empty() && res[0] != "0" && res[0] != "") return false;
	return true;
}


/*

Метод вызывающийся при наступлении определенных событий в группах ВКонтакте

*/

void bot::OnEvent(const std::string& eventType, Person sender, CONFIGURATION config)
{
	//std::cout << sender.Name << " " << sender.Last << " " << eventType << std::endl;

	/*
	
	Значения eventType:
		
		Вконтакте:

		"join" - юзер вступил в группу
		"leave" - юзер покинул группу 

		Телеграм:

		"telegram_new_chat_members" - добавлен новый участник чата
		"telegram_left_chat_member" - участник чата вышел
		"telegram_group_chat_created" - создана конфа с ботом
		"telegram_new_chat_title" - поменяли название конфы 

	*/

	if(eventType == "join")
	{
		std::string answ = "(bot) Спасибо за подписку на группу, "+sender.Name+" "+sender.Last+"!\n\n🚀\
Чтобы начать работу с ботом, просто напишите ему что-нибудь. Бот поддерживает команды - их список вы можете увидеть, отправив боту слово \"помощь\"\
\n\nЭтот бот поддерживается проектом \"Платформа Сало Интеллект\" (https://vk.com/saloint_club).✔\
Там же вы найдете других замечательных ботов (наших и не только), сможете задать вопросы или высказать мнение 😉";

		if(config.INTERFACE == "networking_vkgroup") 
		{
			//SendMessageVK(answer.text, reponse_id, answer.attachment); 
			networking_vkcom* thisVK = dynamic_cast<networking_vkcom*>(getInterfaceByUniqueName(config.INTERFACE_UNIQUE_IDENTIFICATOR));
			
			if (user_is_new(BOT_NAME + "_networking_vkgroup_" + sender.IDENTIFICATOR, BOT_NAME))
			{
				thisVK->interfaceAnswer.HaveMessage = false;
				return;
			}
			
			thisVK->interfaceAnswer.messageText = answ;
			thisVK->interfaceAnswer.messageAttachment = "null";
			thisVK->interfaceAnswer.userID = sender.userID;
			thisVK->interfaceAnswer.HaveMessage = true;
			thisVK->interfaceAnswer.selfKick = false;
		}
	}

	if(eventType == "leave")
	{
		std::string answ = "(bot) Пока, "+sender.Name+" "+sender.Last+"! Был рад знакомству! 💌";

		if(config.INTERFACE == "networking_vkgroup") 
		{
			//SendMessageVK(answer.text, reponse_id, answer.attachment); 
			networking_vkcom* thisVK = dynamic_cast<networking_vkcom*>(getInterfaceByUniqueName(config.INTERFACE_UNIQUE_IDENTIFICATOR));
			
			if (user_is_new(BOT_NAME + "_networking_vkgroup_" + sender.IDENTIFICATOR, BOT_NAME))
			{
				thisVK->interfaceAnswer.HaveMessage = false;
				return;
			}
			
			thisVK->interfaceAnswer.messageText = answ;
			thisVK->interfaceAnswer.messageAttachment = "null";
			thisVK->interfaceAnswer.userID = sender.userID;
			thisVK->interfaceAnswer.HaveMessage = true;
			thisVK->interfaceAnswer.selfKick = false;
		}
	}

	if(eventType == "telegram_new_chat_members")
	{
		srand(time(NULL));
		int val = rand() % 4;
		std::string answ;

		switch(val)
		{
			case 0: answ = "Всем привет! 💌"; break;
			case 1: answ = "Какие люди!"; break;
			case 2: answ = "Ну вот мы и встретились..."; break;
			case 3: answ = "Добро пожаловать! 🖐🤘🙏"; break;
			default: answ = "Всем привет! 💌"; break;
		}

		if(config.INTERFACE == "networking_telegram")
		{
			int attach_type = 0;
		
			networking_telegram* thisTG = dynamic_cast<networking_telegram*>(getInterfaceByUniqueName(config.INTERFACE_UNIQUE_IDENTIFICATOR));

			thisTG->interfaceAnswerTG.messageText = answ;
			thisTG->interfaceAnswerTG.messageAttachment = "null";
			thisTG->interfaceAnswerTG.attachtype = attach_type;
			thisTG->interfaceAnswerTG.chatID = config.confId;
			thisTG->interfaceAnswerTG.HaveMessage = true;
		}
	}

	if(eventType == "telegram_left_chat_member")
	{
		srand(time(NULL));
		int val = rand() % 4;
		std::string answ;

		switch(val)
		{
			case 0: answ = "Пффф, мне он никогда не нравился! 🙃"; break;
			case 1: answ = "Скучать не буду."; break;
			case 2: answ = "Теперь можно хоть поговорить нормально."; break;
			case 3: answ = "Ахаха 🙏"; break;
			default: answ = "Ну как же так"; break;
		}

		if(config.INTERFACE == "networking_telegram")
		{
			int attach_type = 0;
		
			networking_telegram* thisTG = dynamic_cast<networking_telegram*>(getInterfaceByUniqueName(config.INTERFACE_UNIQUE_IDENTIFICATOR));

			thisTG->interfaceAnswerTG.messageText = answ;
			thisTG->interfaceAnswerTG.messageAttachment = "null";
			thisTG->interfaceAnswerTG.attachtype = attach_type;
			thisTG->interfaceAnswerTG.chatID = config.confId;
			thisTG->interfaceAnswerTG.HaveMessage = true;
		}
	}

	if(eventType == "telegram_group_chat_created")
	{
		srand(time(NULL));
		int val = rand() % 4;
		std::string answ;

		switch(val)
		{
			case 0: answ = "Ну, с богом!"; break;
			case 1: answ = "Начинается..."; break;
			case 2: answ = "Кек, че за базар?"; break;
			case 3: answ = "Ахаха, бота звали? 🤠"; break;
			default: answ = "Привед медвед"; break;
		}

		if(config.INTERFACE == "networking_telegram")
		{
			int attach_type = 0;
		
			networking_telegram* thisTG = dynamic_cast<networking_telegram*>(getInterfaceByUniqueName(config.INTERFACE_UNIQUE_IDENTIFICATOR));

			thisTG->interfaceAnswerTG.messageText = answ;
			thisTG->interfaceAnswerTG.messageAttachment = "null";
			thisTG->interfaceAnswerTG.attachtype = attach_type;
			thisTG->interfaceAnswerTG.chatID = config.confId;
			thisTG->interfaceAnswerTG.HaveMessage = true;
		}
	}

	if(eventType == "telegram_new_chat_title")
	{
		srand(time(NULL));
		int val = rand() % 4;
		std::string answ;

		switch(val)
		{
			case 0: answ = "Мне это название больше нравится."; break;
			case 1: answ = "Начинается..."; break;
			case 2: answ = "Задолбали менять название"; break;
			case 3: answ = "Кек 🙈"; break;
			default: answ = "Привед медвед"; break;
		}

		if(config.INTERFACE == "networking_telegram")
		{
			int attach_type = 0;
		
			networking_telegram* thisTG = dynamic_cast<networking_telegram*>(getInterfaceByUniqueName(config.INTERFACE_UNIQUE_IDENTIFICATOR));

			thisTG->interfaceAnswerTG.messageText = answ;
			thisTG->interfaceAnswerTG.messageAttachment = "null";
			thisTG->interfaceAnswerTG.attachtype = attach_type;
			thisTG->interfaceAnswerTG.chatID = config.confId;
			thisTG->interfaceAnswerTG.HaveMessage = true;
		}
	}
}


