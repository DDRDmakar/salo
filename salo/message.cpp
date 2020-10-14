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

#include "headers/message.h"
#include "headers/database.h"
#include "headers/salostring.h"
#include "headers/conveer.h"
#include "conveer/headers/exception.h"
#include "headers/statistics.h"
#include "headers/log.h"
#include "headers/external_api.h"

#include "vk/headers/vkcom.h"
#include "telegram/headers/telegram.h"
#include "web/headers/web.h"

#include "headers/bots.h"

#include <mutex>
//std::mutex CONVEER_MUTEX;
std::mutex ALERT_DEVEL;


/*

Функция обрабатывает прибывшее сообщение!
Вызывается при получении сообщения.
Первый аргумент содержит текст полученного сообщения
второй аргумент содержит структуру с данными об отправителе (описана в headers/message.h) 

С версии Сало 2.0 функция является бот-специфичной. 

*/

void bot::OnMessage(const std::string& messageText, Person sender, CONFIGURATION config)
{
	//Conveer uses shared objects, so it isn't thread-safe.
	//Locking conveer, so two or more threads cant access it in a single time   
	//CONVEER_MUTEX.lock();
	
	// Проверка забаненных пользователей
	/*  TODO
	source_info->add_basic_data(sender, config);
	security->remove_old();
	if(security->check())
	{
		CONVEER_MUTEX.unlock();
		return;
	}
	*/

	Convspace::Answerbox answer;
	BIGINT response_id;
	int attach_type;
	
	logs->CommitGeneric(F, L, "OnMessage called");

	interfaceStatistics* iStat = this->getInterfaceByUniqueName(config.INTERFACE_UNIQUE_IDENTIFICATOR)->iStatistics;
	
	response_id = config.isConf ? config.confId : sender.userID;

	
	if(config.INTERFACE == "networking_vkcom")
	{
		networking_vkcom* thisVK = dynamic_cast<networking_vkcom*>(getInterfaceByUniqueName(config.INTERFACE_UNIQUE_IDENTIFICATOR));
				
		if(config.isConf)
		{
			//std::cout << "conf name = " << config.confTitle << std::endl;
			//std::cout << "conf users count = " << config.confUsersCount << std::endl;

			if(config.confUsersCount >= 280)
			{
				AlertDevelopers("Warning! Bot used in VK conf. chat \"" + config.confTitle + "\" witch have " + std::to_string(config.confUsersCount)
					+ " members. (Kick after 300). \n\n Triggered by:" + this->getName());
			}

			if(config.confUsersCount >= 300)
			{
				//Отправка сообщения с группой в вк
				thisVK->interfaceAnswer.messageText = "Использовать бота можно только в диалоге, в котором меньше 300 участников. ";
				thisVK->interfaceAnswer.messageAttachment = "NONE";
				thisVK->interfaceAnswer.userID = response_id;
				thisVK->interfaceAnswer.HaveMessage = true;
				thisVK->interfaceAnswer.selfKick = true;

				return;
			}
		}
	}

	
	iStat->tConveerTimings.StartCounting(); //Начало замера времени 

	logs->CommitGeneric(F, L, "Starting main conveer function");

	//
	// КОНВЕЕР
	//

	
	if(this->conveer == NULL) { std::cout << "Conveer is NULL!!!" << std::endl; /* CONVEER_MUTEX.unlock(); */ return; }

	try
	{
		this->BOT_INTERNAL_LOCKER.lock();
		answer = this->conveer->generate_answer(sender, config, messageText);
		this->BOT_INTERNAL_LOCKER.unlock();
	}
	catch (Convspace::Exception e) 
	{ 
		answer = Convspace::Answerbox("Conveer exception! " + e.what()); 
		AlertDevelopers("CONVEER EXCEPTION in this->conveer->generate_answer!\n\nConveer says: " + e.what() + "\n\nTriggered by: " + this->getName());
	}
	
	//
	// КОНВЕЕР
	//

#ifdef _KOSTILI
	if (answer.name == "groupFirstName") answer.name = sender.Name;
#endif
	
	// Добавление обращения к сообщению
	if (config.isConf && !answer.name.empty()) answer.text.insert( 0, answer.name + ", " );
	
	
	iStat->tConveerTimings.EndCounting(); //Конец замера времени 
	statistics->IncrementInRAM(MESSAGES_TIME_AVG, INTERFACE_VKCOM, iStat->tConveerTimings.getPeriod()); //Запись времени
	iStat->iStorage.msg_avg_time += iStat->tConveerTimings.getPeriod(); iStat->iStorage.msg_avg_time /= 2;


	//Отправка ответа назад к пользователю
	//std::cout << "act = " << answer.is_active << "\n";
	
	if( answer.is_active )
	{
		//Вебфронтенд
		if(config.INTERFACE == "webfrontend") 
		{ 
			iStat->iStorage.messages_count++;

			//SendMessageWeb(answer.text, config.web_userhash); 
			statistics->IncrementInRAM(MESSAGES_COUNT, INTERFACE_WEBFRONTEND); 
			
			webinterface* thisWEB = dynamic_cast<webinterface*>(getInterfaceByUniqueName(config.INTERFACE_UNIQUE_IDENTIFICATOR));

			thisWEB->interfaceAnswerWeb.messageText = answer.text;
			thisWEB->interfaceAnswerWeb.userHash = config.web_userhash;
			thisWEB->interfaceAnswerWeb.HaveMessage = true;
		}

		//SaloAPI
		if(config.INTERFACE == "SaloAPI") 
		{ 
			iStat->iStorage.messages_count++;
			//SendMessageWeb(answer.text, config.web_userhash); 
			/*statistics->IncrementInRAM(MESSAGES_COUNT, INTERFACE_WEBFRONTEND);*/ 

			webinterface* thisAPI = dynamic_cast<webinterface*>(getInterfaceByUniqueName(config.INTERFACE_UNIQUE_IDENTIFICATOR));

			thisAPI->interfaceAnswerWeb.messageText = answer.text;
			thisAPI->interfaceAnswerWeb.userHash = config.web_userhash;
			thisAPI->interfaceAnswerWeb.HaveMessage = true;
		}
		
		//Вконтакте
		if(config.INTERFACE == "networking_vkcom") 
		{
			iStat->iStorage.messages_count++;

			//SendMessageVK(answer.text, reponse_id, answer.attachment); 
			statistics->IncrementInRAM(MESSAGES_COUNT, INTERFACE_VKCOM);
			logs->CommitGeneric(F, L, "Sending to VK");

			networking_vkcom* thisVK = dynamic_cast<networking_vkcom*>(getInterfaceByUniqueName(config.INTERFACE_UNIQUE_IDENTIFICATOR));

			thisVK->interfaceAnswer.messageText = answer.text;
			thisVK->interfaceAnswer.messageAttachment = answer.attachment;
			thisVK->interfaceAnswer.userID = response_id;
			thisVK->interfaceAnswer.HaveMessage = true;
			thisVK->interfaceAnswer.selfKick = false;
		}

		//Телеграм
		if(config.INTERFACE == "networking_telegram")
		{
			iStat->iStorage.messages_count++;

			statistics->IncrementInRAM(MESSAGES_COUNT, INTERFACE_TELEGRAM);

			attach_type = 0;
			if( answer.type == "photo" ) attach_type = 1;
			if( answer.type == "audio" ) attach_type = 2;
			
			//SendMessageTelegram(answer.text, config.confId, answer.attachment, attach_type);
			
			networking_telegram* thisTG = dynamic_cast<networking_telegram*>(getInterfaceByUniqueName(config.INTERFACE_UNIQUE_IDENTIFICATOR));

			thisTG->interfaceAnswerTG.messageText = answer.text;
			thisTG->interfaceAnswerTG.messageAttachment = answer.attachment;
			thisTG->interfaceAnswerTG.attachtype = attach_type;
			thisTG->interfaceAnswerTG.chatID = config.confId;
			thisTG->interfaceAnswerTG.HaveMessage = true;
		}
	
		//Группы Вконтакте
		if(config.INTERFACE == "networking_vkgroup") 
		{
			iStat->iStorage.messages_count++;

			//SendMessageVK(answer.text, reponse_id, answer.attachment); 
			statistics->IncrementInRAM(MESSAGES_COUNT, INTERFACE_VKCOM);
			logs->CommitGeneric(F, L, "Sending to VK Group");

			networking_vkcom* thisVK = dynamic_cast<networking_vkcom*>(getInterfaceByUniqueName(config.INTERFACE_UNIQUE_IDENTIFICATOR));

			thisVK->interfaceAnswer.messageText = answer.text;
			thisVK->interfaceAnswer.messageAttachment = answer.attachment;
			thisVK->interfaceAnswer.userID = response_id;
			thisVK->interfaceAnswer.HaveMessage = true;
			thisVK->interfaceAnswer.selfKick = false;
		}
	}

	//Unlocking conveer. 
	//CONVEER_MUTEX.unlock();

	return;
}



/*

Функция оповещает разработчиков, через удобную для разработчиков соцсеть

*/

void AlertDevelopers(std::string messageText)
{
	ALERT_DEVEL.lock();
	logs->CommitGeneric(F, L, "AlertDevelopers called");

	int num = atoi(miscGetStringFromFile("resources/vk_devepoersid.conf").c_str());

	//std::cout << messageText << std::endl;

	AnswerError tmp;
	tmp.errorText = std::string(messageText);
	tmp.errorID = num;
	tmp.HaveError = true;
	ErrorStack.push(tmp);

	ALERT_DEVEL.unlock();
}

