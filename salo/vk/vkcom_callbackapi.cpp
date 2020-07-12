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

#include "headers/vkcom.h"
#include "headers/vkcom_callbackapi.h"
#include "../headers/json.h"
#include "../headers/message.h"
#include "../headers/database.h"
#include "../headers/bots.h"
#include "../headers/salostring.h"


/*

Конструктор

*/

CallbackApi::CallbackApi(void* callbackedBot, std::string& vkSelfPageId)
	:	selfId(vkSelfPageId)
{
	bot* Bot = static_cast<bot*>(callbackedBot);
	botname = Bot->getName();
	callbackConveer = Bot->getConveerPtr();

	initial_tick = true;

	connectorType = "group-callbackapi";
	
	std::cout << "Created " << connectorType << " connector for bot \"" << botname << "\"" << std::endl;  
}


/*

Деструктор

*/

CallbackApi::~CallbackApi()
{
	std::cout << "Stopped " << connectorType << " connector for bot \"" << botname << "\"" << std::endl; 
}


/*

Парсит жсон одного сообщения

*/

vkcomBasicMessage CallbackApi::processMessageObject(const std::string& jsonMessageObj)
{
	//parsing JSON
	rapidjson::Document d;
	d.Parse(jsonMessageObj.c_str());

	if(!d.IsObject())
	{
		throw 0;
	}

	const rapidjson::Value& items = d;


	vkcomBasicMessage tmp;
	tmp.have_message = true;
	
	if(items["id"].IsInt() && items["peer_id"].IsInt() && items["text"].IsString())
	{
		tmp.id = items["id"].GetInt();
		tmp.user_id = std::to_string(items["peer_id"].GetInt());
		tmp.text = std::string(items["text"].GetString());
		tmp.haveEvent = false;
	}
	else
	{
		throw 5;
	}

	if(atoll(tmp.user_id.c_str()) > 2000000000)
	{
		tmp.is_conf = true;
		tmp.conf_id = tmp.user_id;
		tmp.user_id = std::to_string(items["from_id"].GetInt());
	}
	else
	{
		tmp.is_conf = false;
	}

	if(items.HasMember("attachments") && items["attachments"].Size() > 0)
	{
		tmp.have_attachment = true;
		tmp.attachment_type = std::string(items["attachments"][0]["type"].GetString());
		
		if(tmp.attachment_type == "photo" || tmp.attachment_type == "video" 
		|| tmp.attachment_type == "audio" || tmp.attachment_type == "doc")
		{
			if(items["attachments"][0].HasMember(tmp.attachment_type.c_str()))
			{
				//обычное сохранение 
				tmp.attachment_link = tmp.attachment_type 
						+ std::to_string(items["attachments"][0][tmp.attachment_type.c_str()]["owner_id"].GetInt())+"_"
						+ std::to_string(items["attachments"][0][tmp.attachment_type.c_str()]["id"].GetInt());
				if(items["attachments"][0][tmp.attachment_type.c_str()].HasMember("access_key"))
					if(items["attachments"][0][tmp.attachment_type.c_str()]["access_key"].IsString())
						tmp.attachment_token = std::string(items["attachments"][0][tmp.attachment_type.c_str()]["access_key"].GetString());

				//сохранение по ссылке
				if(tmp.attachment_type == "photo")
					{
						if(items["attachments"][0][tmp.attachment_type.c_str()]["sizes"].IsArray())	
						if(items["attachments"][0][tmp.attachment_type.c_str()]["sizes"].Size() > 0)
						{
							unsigned int j = items["attachments"][0][tmp.attachment_type.c_str()]["sizes"].Size() - 1;
							if(items["attachments"][0][tmp.attachment_type.c_str()]["sizes"][j]["url"].IsString())
								tmp.attachment_jpeg = items["attachments"][0][tmp.attachment_type.c_str()]["sizes"][j]["url"].GetString();
						}
						
					}
			}
		}
	}
	else
	{
		tmp.have_attachment = false;
		if(items.HasMember("fwd_messages"))
			{
				if(items["fwd_messages"].Size() > 0)
				{
					tmp.have_attachment = true;
					tmp.attachment_type = "fwd_messages";
				}
			}
	}

	return tmp;
}


/*

Расшифровывает одно сообщение и складывает в структуру

*/

vkCallbackObject CallbackApi::decryptMessage(const std::string& message)
{
	vkCallbackObject res;

	try
	{
		std::size_t pos = message.find("/");
		std::string jsonobj = message.substr(pos+1);
		std::string strid = message.substr(0, pos);

		//jsonobj = miscExecSystem(("netcryptm --decrypt "+jsonobj).c_str());

		for(int i = 0; i < jsonobj.length(); i++)
		{
			if(jsonobj[i] == '~') jsonobj[i] = '\"';
			if(jsonobj[i] == '%') jsonobj[i] = '\'';
		}

		jsonobj = getUnicodeFromCodes(jsonobj);

		//std::cout << strid << std::endl << std::endl;
		//std::cout << jsonobj << std::endl;
		//std::cout << "------------" << std::endl;

		res.saloId = std::stoi(strid);
		res.jsonData = jsonobj;	

		res.isEmpty = false;
	}
	catch(...)
	{
		AlertDevelopers("Error in connector "+connectorType+"!\n\n Error while parsing callbackapi message! "+
			"Triggered by: "+botname);
		res.isEmpty = true;
	}

	return res;
}


/*

Возвращает текущее событие

*/

vkcomBasicEvent CallbackApi::getEvent()
{
	try
	{
		vkcomBasicEvent evnt;
		evnt.isEmpty = true;

		//std::cout << "\n\n.." << std::endl;

		DatabaseResult events = database->database_returnQuery("SELECT `event_salo_id`,`user_vk_uid`,`event` FROM `GENERICDATABASE`.`callbackapi_events` WHERE `bot_vk_uid` = \""+selfId+"\" LIMIT 1");
		
		if(events.RowStrings.size() != 0)
		{
			//have 1 event
			//std::cout << events.RowStrings[0] << std::endl;
			std::size_t pos = events.RowStrings[0].find("/");

			std::string eventId = events.RowStrings[0].substr(0, pos);
			std::string userId = events.RowStrings[0].substr(pos+1);
			pos = userId.find("/");
			std::string eventType = userId.substr(pos+1);
			userId = userId.substr(0, pos);

			evnt.userId = userId;
			evnt.eventType = eventType;
			evnt.isEmpty = false;

			std::string sql = "DELETE FROM `GENERICDATABASE`.`callbackapi_events` WHERE `event_salo_id` = " + eventId;
			database->database_simpleQuery(sql);
			
			//std::cout << eventId << " -- " << userId << " -- " << eventType << std::endl;

			return evnt;
		}
		else
		{
			//no events
			evnt.isEmpty = true;
			return evnt;
		}

		return evnt;
	}
	catch(...)
	{
		//callbackapi err
		AlertDevelopers(connectorType + " connector throwed events error!\nTriggered by: \"" + botname + "\".");
		vkcomBasicEvent evnt;
		evnt.isEmpty = true;
		return evnt;
	}
}


/*

Метод возвращает текущее сообщение

*/

vkcomBasicMessage CallbackApi::go()
{
	vkcomBasicMessage res;
	res.have_message = false;
	res.haveEvent = false;

	//dbg
	cach_lookups = 0;
	mess_lookups = 0;

	if(initial_tick)
	{
		std::cout << (connectorType + " connector initialized for bot \"" + botname + "\".") << std::endl;
		AlertDevelopers(connectorType + " connector initialized for bot \"" + botname + "\".");
		initial_tick = false;

		//В начальном тике сразу же дропаем кэш сообщений для данного бота
		AlertDevelopers("Messages cache clean and reinit for bot \"" + botname + "\" (uid = "+selfId+").");
		std::string dropsql = "DELETE FROM `GENERICDATABASE`.`callbackapi_messages` WHERE `bot_vk_uid` = \""+selfId+"\"";
		database->database_simpleQuery(dropsql);
	}
	else
	{
		try
		{
			DatabaseResult callbacks = database->database_returnQuery("SELECT `message_salo_id`,`message_obj` FROM `GENERICDATABASE`.`callbackapi_messages` WHERE `bot_vk_uid` = \""+selfId+"\" LIMIT 256");
			
			for(int i = 0; i < callbacks.RowStrings.size(); i++)
			{
				mess_lookups++;
				bool jsonerr = false;

				//std::cout << callbacks.RowStrings[i] << std::endl << "----" << std::endl;
				vkCallbackObject tmpRaw = decryptMessage(callbacks.RowStrings[i]);
				vkcomBasicMessage tmpVK;

				try
				{
					tmpVK = processMessageObject(tmpRaw.jsonData);
				}
				catch(int ex)
				{
					AlertDevelopers(connectorType + " connector's json solver throwed exception "+std::to_string(ex)
						+"!\n\nVK Says: "+tmpRaw.jsonData+"\n\nTriggered by: \"" + botname + "\".");
					jsonerr = true;
				}
								
				toBeDeleted.push_back(tmpRaw.saloId);

				//если в парсинге json была ошибка, то возвращаемся
				if(jsonerr)
				{
					break;
				}

				//отбрасываем сообщения от отрицательных id
				if(atoll(tmpVK.user_id.c_str()) < 0)
					{  break; 	}

				//std::cout << tmpVK.user_id << " " << tmpVK.text << std::endl;

				//если сообщение непригодно, просто добавить его id в вектор на удаление
				//если пригодно, то добавляем его в результат, и тоже добавляем в вектор на удаление
				if(tmpVK.is_conf)
				{
					if(haveCorrectGreeting(tmpVK.text))
					{
						if(tmpVK.text.find("[club"+selfId+"|") != std::string::npos)
						{
							unsigned int pos1 = tmpVK.text.find("[club"+selfId+"|");
							unsigned int pos2 = tmpVK.text.find("]", pos1);
							int length = pos2 - pos1;
							if(length > 0)
							{
								tmpVK.text.erase(pos1, length+1);
							}					
						}
					
						res = tmpVK;
						break;
					}
				}
				else
				{
					res = tmpVK;
					break;
				}

			}
		
		}
		catch(...)
		{
			//callbackapi err
			AlertDevelopers(connectorType + " connector throwed database error!\n\nTriggered by: \"" + botname + "\".");
		}
	}

	
	std::string deleteList = std::string();

	for(int i = 0; i < toBeDeleted.size(); i++)
	{
		deleteList += "`message_salo_id` = "+std::to_string(toBeDeleted[i]);
	
		if(/*i != 0 && */ i != (toBeDeleted.size()-1))
		{
			deleteList += " OR ";
		}
	}
	
	toBeDeleted.clear();

	if(deleteList != std::string())
	{
		std::string sql = "DELETE FROM `GENERICDATABASE`.`callbackapi_messages` WHERE " + deleteList;
		database->database_simpleQuery(sql);
	}

	//если ответ пустой, поискать ивенты
	if(res.have_message == false)
	{
		vkcomBasicEvent eventEntity = getEvent();
	
		if(!eventEntity.isEmpty)
		{
			res.event.eventType = eventEntity.eventType;
			res.event.userId = eventEntity.userId;
			res.event.isEmpty = false;
			res.haveEvent = true;
		}
	}
	else
	{
		//std::cout << "New message!" << std::endl;
		//std::cout << std::to_string(res.have_attachment) << std::endl;
		//std::cout << res.attachment_jpeg << std::endl;
		//std::cout << res.attachment_type << std::endl;
		//std::cout << res.attachment_link << std::endl;
	}
	
	this->cacheCheckTimer++;
	if(this->cacheCheckTimer == 64)
	{
		this->cacheCheckTimer = 0;

		std::string countsql = "SELECT COUNT(*) FROM `GENERICDATABASE`.`callbackapi_messages` WHERE 1";
		DatabaseResult rowcount = database->database_returnQuery(countsql);

		if (rowcount.RowStrings.size() != 1)
		{
			std::cout << "Failed to verify message cache! Check database!\n\nTriggered by: \"" + botname + "\"." << std::endl;
			AlertDevelopers("Failed to verify message cache! Check database!\n\nTriggered by: \"" + botname + "\".");
		}
		else
		{
			int rownum = atoll(rowcount.RowStrings[0].c_str());
			
			if (rownum > 5000)
			{
				std::cout << "Error! callbackApi cache verify error, cache overflow (" << rownum << " rows!)" << std::endl;
				AlertDevelopers("Dropping callbackapi cache due to overflow!!! Check server/database if this error repeats frequently!\n\nTriggered by: \"" + botname + "\".");

				std::string dropsql = "DELETE FROM `GENERICDATABASE`.`callbackapi_messages` WHERE 1";
				database->database_simpleQuery(dropsql);
			}
		}
	}

	return res;
}