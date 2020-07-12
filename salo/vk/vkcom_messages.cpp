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
#include "../headers/misc.h"
#include "../headers/statistics.h"
#include "../headers/bots.h"

//dbg
#include <ctime> 
#include <chrono>

/*

Функция для работы с ошибками

*/

std::string GetError(const std::string& unexpected)
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

Пытается забрать непрочитанные сообщения Вконтакте
true - если вышло, false - если нет

*/

bool networking_vkcom::fetchNewMessages()
{
	//Dumping here
	//logs->Update(ECMA_IN, ftc.DATA.c_str());

	unsigned int start_time = clock();
		vkcomBasicMessage msgf = connector->go();
	unsigned int end_time = clock(); 
	
	if(debugFetcher)
	{
		std::cout << "Cache requests: " << connector->cach_lookups << "\nMessage lookups: " 
				<< connector->mess_lookups << "\nFetcher time: " << end_time - start_time << "ms.\n\n";
	}

	if(msgf.loginError)
	{
		LoggedIn = false;
		VKONTAKTE_ACCESS_TOKEN = "";
		TOKEN_LIFETIME = 0;
		this->selfPageId = "";
		return false;
	}

	if(msgf.have_message)
	{
	//std::cout << "id = " << msgf.id << "\nuid = " << msgf.user_id << "\ntext = " << msgf.text << "\nis_conf = " << msgf.is_conf << "\nconf_id = " << msgf.conf_id 
    //<< "\nhave_attch = " << msgf.have_attachment << "\nattch_type = " << msgf.attachment_type <<"\n--------------------------------\n";
    	
    	if(!msgf.is_conf)
    	{
    		//запрещаем боту реагировать на самого себя и Павла Дурова
    		if((msgf.user_id == this->selfPageId) || (msgf.user_id == "0") || (msgf.user_id == "1"))
    		{
    			statistics->IncrementInRAM(FETCHER_CACHE_USAGE, FETCHER, connector->cach_lookups);
				statistics->IncrementInRAM(FETCHER_MESSAGES_LOAD, FETCHER, connector->mess_lookups);
				statistics->IncrementInRAM(FETCHER_TIME, FETCHER, end_time - start_time);
				
				//std::cout << "No new msgs!\n";
				return false;
    		}
    	}


    	if(this->interfaceSettings.confchatsOnly == 1)
    	{
    		if(!msgf.is_conf)
    		{
    			statistics->IncrementInRAM(FETCHER_CACHE_USAGE, FETCHER, connector->cach_lookups);
				statistics->IncrementInRAM(FETCHER_MESSAGES_LOAD, FETCHER, connector->mess_lookups);
				statistics->IncrementInRAM(FETCHER_TIME, FETCHER, end_time - start_time);
				
				//std::cout << "No new msgs!\n";
				return false;
    		}
    	}

    	BIGINT chatid;
    	
    	if(msgf.is_conf)
    	{
    		chatid = atoll(msgf.conf_id.c_str());
    	}
    	else
    	{
    		chatid = atoll(msgf.user_id.c_str());
    	}

    	//Запрос к сохраненному методу API VK
    	std::string apiurl;
    	bool vkgroupMembercheckEnabled = false;

    	if(!isGroupBot)
    	{
			apiurl = "https://api.vk.com/method/execute.prepareSending?v=5.62&chid="+std::to_string(chatid)+"&usid="
									+msgf.user_id+"&access_token=" + VKONTAKTE_ACCESS_TOKEN;
		}
		else
		{
			if(this->interfaceSettings.vkgroupId != std::string())
			{
				vkgroupMembercheckEnabled = true;
			}

			if(vkgroupMembercheckEnabled)
			{
				apiurl = "https://api.vk.com/method/groups.isMember?v=5.65&user_id=" + msgf.user_id + "&group_id="+this->interfaceSettings.vkgroupId
									+ "&access_token=" + VKONTAKTE_ACCESS_TOKEN;
			}
			else
			{
				//apiurl = "https://api.vk.com/method/messages.markAsRead?v=5.62&peer_id="+std::to_string(chatid)+"&access_token=" + VKONTAKTE_ACCESS_TOKEN;
				//apiurl for groups to fetch users info
				apiurl = "https://api.vk.com/method/users.get?v=5.80&user_ids=" + msgf.user_id + "&access_token=" + VKONTAKTE_ACCESS_TOKEN;
			}			
		}

		//KOSTYL!!!
		NetworkResult usrinf;
		//if(!isGroupBot) 
			usrinf = pNetwork->network_HTTPS_GET(apiurl.c_str());

		if(!isGroupBot)
		{
			char* err = miscFindAndCopy(usrinf.DATA.c_str(), "status", "ALL_OK");
			if(err == NULL)
			{
				std::cout << "User info error\n"; 
				AlertDevelopers(GetError(usrinf.DATA) + "\n\nERROR DURING HANDLE USER INFO\nTriggered by: " + INTERFACE_SIMPLE_NAME);
				return false;
			}
			else
			{
				delete[] err;
			}
		}

		std::string sfirst, slast, sgrp, scnfname, scnfcount;
		int icnfcount = 0;
		//std::cout << usrinf.DATA << std::endl;

		if(!isGroupBot)
		{
			char* firstname = miscFindAndCopy(usrinf.DATA.c_str(), "\"name\":\"", "\"");
			char* lastname = miscFindAndCopy(usrinf.DATA.c_str(), "\"last\":\"", "\"");	
			char* group = miscFindAndCopy(usrinf.DATA.c_str(), "\"grpmbr\":", ",");	
			if(firstname != NULL) { sfirst = std::string(firstname); delete[] firstname; } else { AlertDevelopers("Error: Bad (null) account name"); return false; }
			if(lastname != NULL) { slast = std::string(lastname); delete[] lastname; } else { AlertDevelopers("Error: Bad (null) account lastname"); return false; }
			if(group != NULL) { sgrp = std::string(group); delete[] group; } else { AlertDevelopers("Error: Bad (null) account group"); return false; }

			//работа с конфами
			scnfname = miscFindAndCopyString(usrinf.DATA.c_str(), "\"chatname\":\"", "\",");
			scnfcount = miscFindAndCopyString(usrinf.DATA.c_str(), "\"chatusers\":", ",");
			if(scnfcount == std::string()) scnfcount = "1";	

			try
			{
				icnfcount = std::stoi(scnfcount);
			}
			catch(...)
			{
				AlertDevelopers("Some errors during getting info about dialogs! Conf data was: " + scnfcount + "\n\n Triggered by: " + 
					INTERFACE_SIMPLE_NAME);
				icnfcount = 1;
			}
		}
		else
		{
			sfirst = miscFindAndCopyString(usrinf.DATA.c_str(), "\"first_name\":\"", "\"");
			slast = miscFindAndCopyString(usrinf.DATA.c_str(), "\"last_name\":\"", "\"");
			
			//std::cout << sfirst << " " << slast << std::endl;
		}


		//костыль, отключающий проверку на подписку в группе у "коммерческих" ботов
		//if(this->interfaceSettings.official != 1)
		sgrp="1";

		Person autor;
		autor.userID = atoll(msgf.user_id.c_str());
		autor.IDENTIFICATOR = msgf.user_id;
		autor.Name = sfirst;
		autor.Last = slast;
		autor.vkIsPlatformMember = sgrp;
		
		CONFIGURATION config;
		config.BOT = this->thisBot;
		config.INTERFACE_UNIQUE_IDENTIFICATOR = this->INTERFACE_IDENTIFICATOR;
		config.isConf = msgf.is_conf;
		config.confId = chatid;
		config.confUsersCount = icnfcount;
		config.confTitle = scnfname;
		config.attachment = (msgf.have_attachment) ? msgf.attachment_type : std::string("null");
		config.attachment_token = msgf.attachment_token;
		config.attachment_link = msgf.attachment_link;
		config.attachment_jpeg = msgf.attachment_jpeg;
		config.web_ip = "";

		//std::cout << config.attachment_link << " resolved to " << config.attachment_jpeg << std::endl;
		

		if(!isGroupBot)
		{
			config.INTERFACE = std::string("networking_vkcom");
		}
		else
		{
			config.INTERFACE = std::string("networking_vkgroup");
		}

		logs->CommitGeneric(F, L, "Calling general message processing function");

		bot* BOT = static_cast<bot*>(this->thisBot);
		BOT->OnMessage(msgf.text, autor, config);



		statistics->IncrementInRAM(FETCHER_CACHE_USAGE, FETCHER, connector->cach_lookups);
		statistics->IncrementInRAM(FETCHER_MESSAGES_LOAD, FETCHER, connector->mess_lookups);
		statistics->IncrementInRAM(FETCHER_TIME, FETCHER, end_time - start_time);

		return true;
	}
	else
	{
		//Нет новых сообщений

		if(msgf.haveEvent) //Но может есть новые события?
		{
			if(!msgf.event.isEmpty)
			{
				std::string userapi = "https://api.vk.com/method/users.get?v=5.69&user_ids="+msgf.event.userId+"&access_token=" + VKONTAKTE_ACCESS_TOKEN;
				NetworkResult userdt = pNetwork->network_HTTPS_GET(userapi.c_str());
				std::string firstname = miscFindAndCopyString(userdt.DATA.c_str(), "\"first_name\":\"", "\"");
				std::string lastname = miscFindAndCopyString(userdt.DATA.c_str(), "\"last_name\":\"", "\"");

				if(firstname == std::string() || lastname == std::string())
				{
					AlertDevelopers("Error during handle Event user info! users.get failed for vk_uid="+msgf.event.userId+"\n\nVK Says: "+userdt.DATA);
				}
				else
				{
					Person pers;
					pers.userID = atoll(msgf.event.userId.c_str());
					pers.IDENTIFICATOR = msgf.event.userId;
					pers.Name = firstname;
					pers.Last = lastname;
					pers.vkIsPlatformMember = "1";

					CONFIGURATION config;
					config.BOT = this->thisBot;
					config.INTERFACE_UNIQUE_IDENTIFICATOR = this->INTERFACE_IDENTIFICATOR;
					config.INTERFACE = "networking_vkgroup";
					config.isConf = false;
					config.confId = 0;
					config.confUsersCount = 0;

					bot* BOT = static_cast<bot*>(this->thisBot);
					BOT->OnEvent(msgf.event.eventType, pers, config);

					statistics->IncrementInRAM(FETCHER_CACHE_USAGE, FETCHER, connector->cach_lookups);
					statistics->IncrementInRAM(FETCHER_MESSAGES_LOAD, FETCHER, connector->mess_lookups);
					statistics->IncrementInRAM(FETCHER_TIME, FETCHER, end_time - start_time);
					
					return true;
				}
			}
		}

		//Нет ивентов

		statistics->IncrementInRAM(FETCHER_CACHE_USAGE, FETCHER, connector->cach_lookups);
		statistics->IncrementInRAM(FETCHER_MESSAGES_LOAD, FETCHER, connector->mess_lookups);
		statistics->IncrementInRAM(FETCHER_TIME, FETCHER, end_time - start_time);
		
		//std::cout << "No new msgs!\n";
		return false;
	}

	return true;
}


/*

Функция отправлет сгенерированные сообщения пользователю Вконтакте

*/

void networking_vkcom::pushNewMessages()
{
	logs->CommitGeneric(F, L, "Pushing new messages");

	std::string msg = pNetwork->urlencode(this->interfaceAnswer.messageText);
	std::string att = "";

	
	if(this->interfaceAnswer.messageAttachment != "NONE" && this->interfaceAnswer.messageAttachment != "" && this->interfaceAnswer.messageAttachment != "null")
	{
		att = "&attachment=" + this->interfaceAnswer.messageAttachment;
		logs->CommitGeneric(F, L, "Message with attachment (" + this->interfaceAnswer.messageAttachment + ")");
	}
	else
	{
		if(/*!isGroupBot && */this->interfaceSettings.official == 1)
		{
			promotion_timer++;

			//std::cout << promotion_timer << std::endl;
			//std::cout << this->interfaceAnswer.userID << std::endl;
			//std::cout << msg << std::endl;

			if(promotion_timer >= 200) //временно, потом сделать 400
			{
				//promotion_timer = 0;
				
				if(promotion_timer == 200) //временно, потом сделать 400
				{
					isOnPromotionStreak = true;
					promotion_cache.clear();
				}

				bool isUniquieConf = false;

				if(this->interfaceAnswer.userID >= 2000000000)
				{
					isUniquieConf = true;
				}

				for(int i = 0; i < promotion_cache.size(); i++)
				{
					if(promotion_cache[i] == this->interfaceAnswer.userID)
					{
						isUniquieConf = false;
					}
				}

				promotion_cache.push_back(this->interfaceAnswer.userID);

				if(isUniquieConf)
				{
					att = "&attachment=" + getNextPromotion();
					logs->CommitGeneric(F, L, "Message with promotion (" + att + ")");

					promotion_timer = getPromotionTimer(promotion_timer);
					promotion_sent++;
				}
			}
			else //персональные промоушены
			{
				if(promotion_timer == 100 && this->interfaceSettings.personalPromotion.length() > 3)
				{
					att = "&attachment=" + this->interfaceSettings.personalPromotion;
					logs->CommitGeneric(F, L, "Message with promotion (" + att + ")");
					promotion_sent++;
				}

			}	//персональные промоушены
		}
	}

	std::string apiurl = "https://api.vk.com/method/messages.send";
	std::string postkey = "peer_id="+std::to_string(this->interfaceAnswer.userID) + "&message=" + msg + att + "&v=5.50&access_token="+VKONTAKTE_ACCESS_TOKEN;

	//Dumping here
	//logs->Update(ECMA_OUT, std::string(apiurl + "\n" + postkey).c_str());
	
	NetworkResult res = pNetwork->network_HTTPS_POST(apiurl.c_str(), postkey.c_str(), "(none)", 0);
	
	this->interfaceAnswer.HaveMessage = false;
	
	//Errors check
	//std::cout << res.DATA << std::endl;
	char* err = miscFindAndCopy(res.DATA.c_str(), "error", "}");
	if(err != NULL)
	{
		std::string errortext = GetError(res.DATA);

		if(errortext == "Flood control: same message already sent")
		{
			this->interfaceAnswer.HaveMessage = true;
			this->interfaceAnswer.messageText += " &#8194; "; //std::to_string(rand() % 100); 
			//currentAnswer->messageText->append();
		}
		else
		{
			logs->CommitGeneric(F, L, "Message sending failed!!!\n\nError Text = " + res.DATA);
			
			if(errortext == "Captcha needed")
			{
				if(this->interfaceSettings.anticaptchaEnabled)
				{
					bool cres = captcha(res.DATA);

					if(cres)
					{
						std::string apiurl = "https://api.vk.com/method/messages.send";
						std::string postkey = "peer_id="+std::to_string(this->interfaceAnswer.userID) + "&message=" + msg + att 
							+ "&captcha_sid="+ captcha_sid +"&captcha_key="+ captcha_answer + "&v=5.50&access_token="+VKONTAKTE_ACCESS_TOKEN;

						NetworkResult res = pNetwork->network_HTTPS_POST(apiurl.c_str(), postkey.c_str(), "(none)", 0);
					}
					else
					{
						logs->CommitGeneric(F, L, "Captcha failed!!!\n\nError Text = " + res.DATA);
					}
				}
			}
			else
			{
				if( (errortext == "Flood control: too much messages sent to user" && this->interfaceAnswer.userID == 2000000215)  
					|| errortext == "Can't send messages for users without permission")
				{
					//bochok potik =(
				}
				else
				{
					AlertDevelopers("Error: " + errortext + "\n\nERROR WHILE SENDING MESSAGE!\nVK UNEXPECTED ANSWER\n\n" 
					+ "Message Text: " + this->interfaceAnswer.messageText + "\n" 
					+ "Attachment: " + att + "\n" 
					+ "Peer: " + std::to_string(this->interfaceAnswer.userID) + "\n" 
					+ "Triggered by: " + INTERFACE_SIMPLE_NAME);
				}
			}
		}

		delete[] err;
	}


	if(this->interfaceAnswer.selfKick)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(300));

		std::string localconfid = std::to_string(interfaceAnswer.userID - 2000000000);

		AlertDevelopers("Self kicking from chat! chatid = "+localconfid+"\n\n Triggered by: " + INTERFACE_SIMPLE_NAME);

		std::string userNumeric;
		userNumeric = this->selfPageId;

		std::string apiurlkick = "https://api.vk.com/method/messages.removeChatUser?chat_id="+localconfid+"&user_id="+userNumeric+"&v=5.68&access_token="+VKONTAKTE_ACCESS_TOKEN;
		NetworkResult r = pNetwork->network_HTTPS_GET(apiurlkick.c_str());

		//std::cout << r.DATA << std::endl;
	}

	//apiurl = "https://api.vk.com/method/account.setOnline?voip=0&v=5.50&access_token="+VKONTAKTE_ACCESS_TOKEN;
	//pNetwork->network_HTTPS_GET(apiurl.c_str());
}

