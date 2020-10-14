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
#include "headers/telegram.h"
#include "../headers/message.h"
#include "../headers/misc.h"
#include "../headers/bots.h"
#include "../headers/json.h"


/*

Получает новое сообщение 

*/

bool networking_telegram::fetchMessage()
{
	try
	{
		std::string api = API_STRING + "getUpdates?limit=1&offset=" + std::to_string(offset);
		NetworkResult msg = ptNetwork->network_HTTPS_GET(api.c_str());
		
		if(!IsSuccessful(msg.DATA)) { throw std::string("*** ok marker not found *** " + msg.DATA); return false; }
		if(msg.DATA == std::string("{\"ok\":true,\"result\":[]}")) return false; //NO NEW MESSAGES
		// else std::cout << msg.DATA << std::endl;

		Document d;
		ParseResult ok = d.Parse(msg.DATA.c_str());
		if(!ok || !d.IsObject() || !d.HasMember("ok") || !d.HasMember("result")) 
		{ 
			throw std::string("*** Not a json object! *** " + msg.DATA); 
			return false;  
		}

		int uid = d["result"][0]["update_id"].GetInt();
		offset = uid + 1;
		
		if(!d["result"][0].HasMember("message"))
		{
			if(d["result"][0].HasMember("edited_message"))
			{
				return false;
			}
			else
			{
				throw std::string("*** No message or edited_message object! *** " + msg.DATA); 
				return false;
			}
		}

		const Value& message_items = d["result"][0]["message"];

		std::string type = std::string(message_items["chat"]["type"].GetString());
		if(type != "group" && type != "supergroup" && type != "private") 
			throw std::string("*** Unknown chat type! *** " + msg.DATA);

		if(!message_items.HasMember("from")) { throw std::string("*** 'from' is not an object! *** " + msg.DATA); }

		long long int userid = message_items["from"]["id"].GetInt64();
		long long int chatid = message_items["chat"]["id"].GetInt64();
		std::string firstname = std::string(message_items["from"]["first_name"].GetString());
		
		std::string lastname = std::string();
		if(message_items["from"].HasMember("last_name"))
		{
			lastname = std::string(message_items["from"]["last_name"].GetString());
		}

		std::string msgtxt = std::string();
		std::string attach = std::string("null");
		std::string event = std::string("null");
		bool hasGreeting = false;
		bool fromConf = false;

		if(type == "group") fromConf = true; 
		if(type == "supergroup") fromConf = true; 
		if(type == "private") fromConf = false; 
		

		if(message_items.HasMember("text"))
		{
			msgtxt = std::string(message_items["text"].GetString());

			if(msgtxt.find(" @SaloIntellectBot") != std::string::npos)
				{ msgtxt.erase(msgtxt.find(" @SaloIntellectBot"), 18);
				  hasGreeting = true; }

			if(msgtxt.find("@SaloIntellectBot") != std::string::npos)
				{ msgtxt.erase(msgtxt.find("@SaloIntellectBot"), 17);
				  hasGreeting = true; }
		
			if(message_items.HasMember("entities"))
			{
				if(message_items["entities"].IsArray())
				{
					for(SizeType i = 0; i < message_items["entities"].Size(); i++)
					{
						std::string entity = std::string(message_items["entities"][i]["type"].GetString());
						if(entity == "bot_command") { msgtxt = inlineToConveerCommands(msgtxt); }
						if(entity == "url") throw std::string("*** message declined due to url entity filter! ***");
						if(entity == "email") throw std::string("*** message declined due to email entity filter! ***"); 
						if(entity == "text_link") throw std::string("*** message declined due to text_link entity filter! ***");  
					}
				}
			}

			if(fromConf)
			{
				if(haveCorrectGreeting(msgtxt)) hasGreeting = true;
				if(msgtxt[0] != '/' && message_items.HasMember("reply_to_message")) hasGreeting = true;
			}

			if(fromConf && !hasGreeting)
			{ 
				return false;
			}
		}
		else
		{
			//В телеграме какой-то велосипедный механизм прикреплений
			//Нету поля attachment_type как в ВК, приходится искать члены вручную
			if(message_items.HasMember("audio")) { attach = "audio"; }
			if(message_items.HasMember("document")) { attach = "document"; }
			if(message_items.HasMember("photo")) { attach = "photo"; }
			if(message_items.HasMember("sticker")) { attach = "sticker"; }
			if(message_items.HasMember("video")) { attach = "video"; }
			if(message_items.HasMember("video_note")) { attach = "video"; }
			if(message_items.HasMember("voice")) { attach = "voice"; }
			if(message_items.HasMember("contact")) { attach = "contact"; }
			if(message_items.HasMember("location")) { attach = "location"; }
			if(message_items.HasMember("venue")) { attach = "venue"; }

			//Тоже самое для ивентов. Дуров в курсе, что есть языки помимо js и python?
			if(message_items.HasMember("new_chat_members")) { event = "telegram_new_chat_members"; }
			if(message_items.HasMember("left_chat_member")) { event = "telegram_left_chat_member"; }
			if(message_items.HasMember("group_chat_created")) { event = "telegram_group_chat_created"; }
			if(message_items.HasMember("new_chat_title")) { event = "telegram_new_chat_title"; }

			//заголовки аттачментов скидываются в текст сообщения
			if(message_items.HasMember("caption")) { msgtxt = message_items["caption"].GetString(); }
		}

		if(message_items.HasMember("forward_date")) attach = "fwd_messages";

		Person autor;
		autor.userID = userid;
		autor.IDENTIFICATOR = /*"telegram" +*/ std::to_string(userid);
		autor.Name = getUnicode(firstname);
		autor.Last = getUnicode(lastname);
			
		CONFIGURATION config;
		config.BOT = this->thisBot;
		config.INTERFACE = std::string("networking_telegram");
		config.INTERFACE_UNIQUE_IDENTIFICATOR = this->INTERFACE_IDENTIFICATOR;
		config.confId = chatid;
		config.attachment = attach;
		config.web_ip = "";
		config.isConf = fromConf; 

		bot* BOT = static_cast<bot*>(this->thisBot);
		if(event == "null")
			BOT->OnMessage(getUnicode(msgtxt), autor, config);
		else
			BOT->OnEvent(event, autor, config);
	}
	catch(std::string ex)
	{
		std::string errmsg = "Telegram Fetching Error! \n\nFetcher says: " + ex + "\n\nTriggered by: " + this->INTERFACE_SIMPLE_NAME;
		std::cout << errmsg << std::endl;
		AlertDevelopers(errmsg);
		this->interfaceAnswerTG.HaveMessage = false;
		return false;
	}
	catch(...)
	{
		std::string errmsg = "Telegram Fetching Error: Something wrong! Triggered by: " + this->INTERFACE_SIMPLE_NAME;
		std::cout << errmsg << std::endl;
		AlertDevelopers(errmsg);
		this->interfaceAnswerTG.HaveMessage = false;
		return false;
	}

	if(this->interfaceAnswerTG.HaveMessage)
	{
		pushMessage();
		return true;
	} 
	else
	{
		return false;
	}
}


/*

Отправляет новое сообщение

*/

void networking_telegram::pushMessage()
{
	if(this->interfaceAnswerTG.HaveMessage)
	{
		NetworkResult msg;

		if(this->interfaceAnswerTG.attachtype == 0)
		{
			//TODO: logs here
			std::string msg_txt = ptNetwork->urlencode(this->interfaceAnswerTG.messageText);
			std::string api = API_STRING + "sendMessage?chat_id=" + std::to_string(this->interfaceAnswerTG.chatID) 
					+ "&text=" + msg_txt;
			msg = ptNetwork->network_HTTPS_GET(api.c_str());
			this->interfaceAnswerTG.HaveMessage = false;
		}
		else
		{
			if(this->interfaceAnswerTG.attachtype == 1)
			{
				std::string api = API_STRING + "sendPhoto?chat_id=" + std::to_string(this->interfaceAnswerTG.chatID) 
						+ "&photo="+this->interfaceAnswerTG.messageAttachment;
				msg = ptNetwork->network_HTTPS_GET(api.c_str());
				this->interfaceAnswerTG.HaveMessage = false;
			}

			if(this->interfaceAnswerTG.attachtype == 2)
			{
				std::string api = API_STRING + "sendAudio?chat_id=" + std::to_string(this->interfaceAnswerTG.chatID) 
						+ "&audio="+this->interfaceAnswerTG.messageAttachment;
				msg = ptNetwork->network_HTTPS_GET(api.c_str());
				this->interfaceAnswerTG.HaveMessage = false;
			}
		}
	
		std::string message_id = miscFindAndCopyString(msg.DATA.c_str(), "\"message_id\":", ",");
		if(message_id == std::string())
		{
			std::string err = "Telegram Sending Error! \n\nTelegram api says: " + msg.DATA + "\n\nTriggered by: " + this->INTERFACE_SIMPLE_NAME;
			std::cout << err << std::endl;
			AlertDevelopers(err);
		}

		this->interfaceAnswerTG.HaveMessage = false;
	}
}