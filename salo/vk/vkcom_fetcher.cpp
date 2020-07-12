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

#include <string>
#include "headers/vkcom.h"
#include "../headers/json.h"
#include "../headers/message.h"
#include "../headers/misc.h"
#include "../headers/bots.h"


//constr
Fetcher::Fetcher(void* fetcherBot, network* vkNetwork, std::string& fetcherToken, bool isGroupBot)
	:	token(fetcherToken)
{
	bot* Bot = static_cast<bot*>(fetcherBot);
	fetcherConveer = Bot->getConveerPtr();
	botname = Bot->getName();

	initial_tick = true;
	processed_messages = 0;
	accoutErrorsCount = 0;

	//token = fetcherToken;
	fetcherNetwork = vkNetwork;

	if(!isGroupBot)
	{
		connectorType = "profile-fetcher-legacy";
	}
	else
	{
		connectorType = "group-fetcher-legacy";
	}

	std::cout << "Created " << connectorType << " connector for bot \"" << botname << "\"" << std::endl;  
}


//destr
Fetcher::~Fetcher()
{
	CURRENT_MESSAGES.clear();
	CACHE_ID.clear();
	std::cout << "Stopped " << connectorType << " connector for bot \"" << botname << "\"" << std::endl; 
}


/*

Функция для работы с ошибками

*/

std::string GetErrorVK(const std::string& unexpected)
{
	char* Error = miscFindAndCopy(unexpected.c_str(), "\"error_msg\":\"", "\"");

	if(Error == NULL)
	{
		return unexpected;
	}
	else
	{
		std::string errret = std::string(Error);
		delete[] Error;
		return errret;
	}
}

/*

Функция скачивает сообщения со вконтакте и располагает их в кэше Сала

*/

void Fetcher::vkDownload(const std::string& data)
{
	//std::cout << "Downloading..." << std::endl;

	//parsing JSON
	rapidjson::Document d;
	d.Parse(data.c_str());

	if(!d.IsObject() || !d.HasMember("response") || !d["response"].HasMember("items"))
	{
		throw 0;
	}

	const rapidjson::Value& items = d["response"]["items"];

	//parsing ALL MESSAGES to salo own format. 
	CURRENT_MESSAGES.clear();
	processed_messages = 0;

	for(SizeType ii = 0; ii < items.Size(); ii++)
	{
		vkcomBasicMessage tmp;
		tmp.have_message = true;
		tmp.id = items[ii]["id"].GetInt();
		tmp.user_id = std::to_string(items[ii]["user_id"].GetInt());
		tmp.text = std::string(items[ii]["body"].GetString());
		tmp.haveEvent = false;

		if(items[ii].HasMember("chat_id"))
		{
			tmp.is_conf = true;
			tmp.conf_id = std::to_string(items[ii]["chat_id"].GetInt());
		}
		else
		{
			tmp.is_conf = false;
		}

		if(items[ii].HasMember("attachments"))
		{
			tmp.have_attachment = true;
			tmp.attachment_type = std::string(items[ii]["attachments"][0]["type"].GetString());
			
			if(tmp.attachment_type == "photo" || tmp.attachment_type == "video" 
			|| tmp.attachment_type == "audio" || tmp.attachment_type == "doc")
			{
				if(items[ii]["attachments"][0].HasMember(tmp.attachment_type.c_str()))
				{
					//обычное сохранение 
					tmp.attachment_link = tmp.attachment_type 
							+ std::to_string(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["owner_id"].GetInt())+"_"
							+ std::to_string(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["id"].GetInt());
					if(items[ii]["attachments"][0][tmp.attachment_type.c_str()].HasMember("access_key"))
						if(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["access_key"].IsString())
							tmp.attachment_token = std::string(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["access_key"].GetString());

					//сохранение по ссылке
					if(tmp.attachment_type == "photo")
					{
						//75px
						if(items[ii]["attachments"][0][tmp.attachment_type.c_str()].HasMember("photo_75"))
							if(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["photo_75"].IsString())
								tmp.attachment_jpeg = std::string(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["photo_75"].GetString());
						//130px
						if(items[ii]["attachments"][0][tmp.attachment_type.c_str()].HasMember("photo_130"))
							if(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["photo_130"].IsString())
								tmp.attachment_jpeg = std::string(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["photo_130"].GetString());
						//604px
						if(items[ii]["attachments"][0][tmp.attachment_type.c_str()].HasMember("photo_604"))
							if(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["photo_604"].IsString())
								tmp.attachment_jpeg = std::string(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["photo_604"].GetString());
						//807px
						if(items[ii]["attachments"][0][tmp.attachment_type.c_str()].HasMember("photo_807"))
							if(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["photo_807"].IsString())
								tmp.attachment_jpeg = std::string(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["photo_807"].GetString());
						//1280px
						if(items[ii]["attachments"][0][tmp.attachment_type.c_str()].HasMember("photo_1280"))
							if(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["photo_1280"].IsString())
								tmp.attachment_jpeg = std::string(items[ii]["attachments"][0][tmp.attachment_type.c_str()]["photo_1280"].GetString());
					}
				}
			}
		}
		else
		{
			tmp.have_attachment = false;
			if(items[ii].HasMember("fwd_messages"))
				{
					tmp.have_attachment = true;
					tmp.attachment_type = "fwd_messages";
				}
		}

		//std::cout << "id = " << tmp.id << "\nuid = " << tmp.user_id << "\ntext = " << tmp.text << "\nis_conf = " << tmp.is_conf << "\nconf_id= " << tmp.conf_id 
		//	<< "\nhave_attch = " << tmp.have_attachment << "\nattch_type = " << tmp.attachment_type <<"\n--------------------------------\n";

	CURRENT_MESSAGES.push_back(tmp);
	}
}


/*

Функция поиска в кэше сообщений

*/

bool Fetcher::isInCache(const unsigned int& id)
{

for(int i = CACHE_ID.size()-1; i >= 0; i--)
	{
		cach_lookups++;
		if(id == CACHE_ID[i])
		{
			return true;
		}
	}

return false;
}


/*

Функция добавления в кэш

*/

void Fetcher::addToCache(const unsigned int& id)
{
	if(CACHE_ID.size() < 60)
	{
		CACHE_ID.push_back(id);
	}
	else
	{
		CACHE_ID.erase(CACHE_ID.begin());
		CACHE_ID.push_back(id);
	}
}


/*

Основная функция 

*/

vkcomBasicMessage Fetcher::go()
{
	std::string apiurlf = "https://api.vk.com/method/messages.get?v=5.60&out=0&count=30&access_token=" + token;
	NetworkResult ftc = fetcherNetwork->network_HTTPS_GET(apiurlf.c_str());

	std::string data = ftc.DATA;

	vkcomBasicMessage res;
	res.have_message = false;
	res.haveEvent = false;

	//dbg
	cach_lookups = 0;
	mess_lookups = 0;

	//Загрузка
	bool fail = false;

	try
	{
		vkDownload(data);
	}
	catch(...)
	{
		std::string errwhat = GetErrorVK(data);

		//интерналы вк нас не сильно интересуют.
		if(errwhat.substr(0, 15) != "Internal server")
		{
			AlertDevelopers(connectorType + " connector failed: Downloader failed.\n\n"+errwhat+ "\nTriggered by: " + botname);
		}

		//Отключать аккаунты, которые спамят подобными ошибками

		if(errwhat.substr(0, 25) == "User authorization failed")
		{
			accoutErrorsCount++;
		}
		
		fail = true;
	}

	if(accoutErrorsCount >= 100)
	{
		AlertDevelopers("Critical error by " + botname +"'s " + connectorType + " connector: too big login error streak. Logging out...");
		res.loginError = true;
		fail = true;
	}
	else
	{
		res.loginError = false;
	}

	if(fail)
	{
		return res;
	}
	

	//начальный тик, нужный для построения кэша
	if(initial_tick)
	{
		for(int i = 0; i < CURRENT_MESSAGES.size(); i++)
		{
			addToCache(CURRENT_MESSAGES[i].id);
		}

		std::string alerter = connectorType + " connector initialized: " + std::to_string(CURRENT_MESSAGES.size()) 
										+ " messages downloaded form vk. Triggered by: " + botname;

		std::cout << alerter << std::endl;
		AlertDevelopers(alerter);

		initial_tick = false;
		return res;
	}

	
	//шерстим весь список на предмет наличия сообщений
	for(int i = 0; i < CURRENT_MESSAGES.size(); i++)
	{
		processed_messages++;
		mess_lookups++;

		if(!isInCache(CURRENT_MESSAGES[i].id))
		{
			addToCache(CURRENT_MESSAGES[i].id);

			if(!CURRENT_MESSAGES[i].is_conf)
			{
				return CURRENT_MESSAGES[i]; //сообщение из лс, пойдет
			}
			else
			{
				if(haveCorrectGreeting(CURRENT_MESSAGES[i].text))
				{
					return CURRENT_MESSAGES[i]; //сообщение из диалога с корректным приветсвием
				}
			}
		}
	}

	//printf("value = %d\n", d["response"]["items"][0]["id"].GetInt());


	//std::cout << data << std::endl;


	return res;
}
