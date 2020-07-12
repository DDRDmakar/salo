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


#include <mutex>
#include "headers/vkcom.h"
#include "headers/vkcom_blacklist.h"
#include "../headers/log.h"
#include "../headers/bots.h"
#include "../headers/database.h"
#include "../headers/json.h"
#include "../headers/config.h"


//alert devs
#include "../headers/message.h"


std::mutex BAN_MUTEX;


/*

Функции бана разбана в С формате - обертка для конвеера, переводящая вызов в 
обращение к методу конкретному объекта вк интерфейса

*/


/*

Функция бана пользователя, обернутая для Salo 2

*/

int vkban(const std::string& id, void* botz, const std::string& interface)
{
	BAN_MUTEX.lock();

	bot* externalBot = static_cast<bot*>(botz);
	
	if(externalBot == NULL) 
	{ 
		AlertDevelopers("BAN ERROR - BOT POINTER IS NULL!!!\n"); BAN_MUTEX.unlock(); 
		return 0; 
	}
	
	networking_vkcom* vk;
	vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(interface));
	
	if(vk == NULL) 
	{ 
		AlertDevelopers("BAN ERROR - INTERFACE POINTER IS NULL!!!\n\nBot = " + externalBot->getName() + "; Interface = " + interface); 
		BAN_MUTEX.unlock(); 
		return 0; 
	}

	int res = vk->banAction(id, true);

	BAN_MUTEX.unlock();

	return res;
}


/*

Функция разбана пользователя, обернутая для Salo 2

*/

int vkunban(const std::string& id, void* botz, const std::string& interface)
{
	BAN_MUTEX.lock();

	bot* externalBot = static_cast<bot*>(botz);
	if(externalBot == NULL) 
	{ 
		AlertDevelopers("UNBAN ERROR - BOT POINTER IS NULL!!!\n"); 
		BAN_MUTEX.unlock(); 
		return 0; 
	}

	networking_vkcom* vk;
	vk = dynamic_cast<networking_vkcom*>(externalBot->getInterfaceByUniqueName(interface));
	
	if(vk == NULL) 
	{ 
		AlertDevelopers("UNBAN ERROR - INTERFACE POINTER IS NULL!!!\n\nBot = " + externalBot->getName() + "; Interface = " + interface); 
		BAN_MUTEX.unlock(); 
		return 0; 
	}

	int res = vk->banAction(id, false);

	BAN_MUTEX.unlock();

	return res;
}


/*

Функция, производящая действия бана или разбана

*/

int networking_vkcom::banAction(const std::string& id, bool toBan) 
{	
	if(isGroupBot) return 0;
	
	if(toBan)
	{
		//std::cout << "blacklist, id = " << id + "\n"; 
		logs->CommitGeneric(F, L, "blacklist called, id =" + id);
		std::string api = "https://api.vk.com/method/account.banUser?user_id="+id+"&v=5.50&access_token="+this->VKONTAKTE_ACCESS_TOKEN;
		NetworkResult r = this->pNetwork->network_HTTPS_GET(api.c_str());
		//AlertDevelopers("ban/unban debug: BAN USER "+id+" ON INTERFACE "+INTERFACE_SIMPLE_NAME+"\n\nVK says: " + r.DATA);

		if(r.DATA != "{\"response\":1}")
		{
			//An Error!
			AlertDevelopers("ERROR: BAN USER "+id+" ON INTERFACE "+INTERFACE_SIMPLE_NAME+"\n\nVK says: " + r.DATA);
			return 0;
		}
		else
		{
			logs->CommitGeneric(F, L, "Done.");
			return 1;
		}
	}
	else
	{
		//std::cout << "UNblacklist, id = " << id + "\n"; 
		logs->CommitGeneric(F, L, "UNblacklist called, id =" + id);
		std::string api = "https://api.vk.com/method/account.unbanUser?user_id="+id+"&v=5.50&access_token="+this->VKONTAKTE_ACCESS_TOKEN;
		NetworkResult r = this->pNetwork->network_HTTPS_GET(api.c_str());
		//AlertDevelopers("ban/unban debug: UNBAN USER "+id+" ON INTERFACE "+INTERFACE_SIMPLE_NAME+"\n\nVK says: " + r.DATA);

		if(r.DATA != "{\"response\":1}")
		{
			//An Error!
			AlertDevelopers("ERROR: UNBAN USER "+id+" ON INTERFACE "+INTERFACE_SIMPLE_NAME+"\n\nVK says: " + r.DATA);
			return 0;
		}
		else
		{
			logs->CommitGeneric(F, L, "Done.");
			return 1;
		}
	}
}


/*

Мьютекс, чтобы баны не выполнялись параллельно. 
Вообще, по логике вещей дата-рейсов быть не должно,
но на всякий случай пусть будет

*/

std::mutex UNBANNER_MUTEX;


/*

Функция для принудительной поверки бан-листа

*/

std::string networking_vkcom::forceBanlistRefresh(const std::string& botname, const std::string& dbname)
{
	if((!this->LoggedIn))
	{
		return "[OLD INTERNAL UNBANNER!]\n\nError while sync banlists! account not logged in!";
	}

	UNBANNER_MUTEX.lock();

	//Задержка нужна, чтобы не вылететь из-за спама запросами
	std::this_thread::sleep_for(std::chrono::milliseconds(500)); 

	std::cout << "Downlading list ... " << std::endl;
	
	std::string apiurl = "https://api.vk.com/method/account.getBanned?v=5.60&offset=0&count=199&access_token=" + VKONTAKTE_ACCESS_TOKEN;
	NetworkResult res = pNetwork->network_HTTPS_GET(apiurl.c_str());

	std::cout << res.DATA << std::endl;

	//parsing JSON
	Document d;
	d.Parse(res.DATA.c_str());

	if(!d.IsObject() || !d.HasMember("response") || !d["response"].HasMember("items"))
	{
	  	std::cout << "Error!!!\n";
	   	AlertDevelopers("BANLISTS SYNC FAILED: VK SAYS:\n\n" + res.DATA);

    	UNBANNER_MUTEX.unlock();
    	return "[OLD INTERNAL UNBANNER!]\n\nError while sync banlists! Please check consoles and logs!";
    }

    const Value& items = d["response"]["items"];
 	int noBan = 0;

	for(int i = 0; i < items.Size(); i++)
	{
	   	std::cout << i << ") " << items[i]["id"].GetInt() << " -- " << items[i]["first_name"].GetString() << " " << items[i]["last_name"].GetString() << " -- P ";
		
		std::this_thread::sleep_for(std::chrono::milliseconds(360)); 

		std::cout << "P -- ";

		//perevirka
		std::string sql = "SELECT * FROM `"+dbname+"`.`ban_list` WHERE `id` = '"+botname+"_networking_vkcom_"+std::to_string(items[i]["id"].GetInt())+"' LIMIT 1";
		DatabaseResult r = database->database_returnQuery(sql);

		if(r.is_empty)
		{
			std::cout << "NOT IN BAN LIST -- ";
			noBan++;

			//Разбаниваем
			printf("a");
			std::string apiurl1 = "https://api.vk.com/method/account.unbanUser?v=5.60&user_id=";
			printf("b");
			apiurl1 += std::to_string(items[i]["id"].GetInt());
			printf("c");
			apiurl1 += "&access_token=" + this->VKONTAKTE_ACCESS_TOKEN;
			printf("d");
			pNetwork->network_HTTPS_GET(apiurl1.c_str());
			printf("e");
			printf("f\n");
		}
		else
		{
			std::cout << "IN BAN LIST" << std::endl;
		}
    }

	std::cout << "\nNot in Ban list: " << noBan << "\n";

	UNBANNER_MUTEX.unlock();
	return "[OLD INTERNAL UNBANNER!]\n\nBanned accounts synchronization ended: unblacklisted " + std::to_string(noBan) + " accounts.";
}



std::string networking_vkcom::forceBanlistRefreshPy(const std::string& botname, const std::string& dbname)
{
	UNBANNER_MUTEX.lock();

	if((!this->LoggedIn))
	{
		UNBANNER_MUTEX.unlock();
		return "Error while sync banlists! account not logged in!";
	}

	std::string crtoken = miscExecSystem(std::string("../libcryptm/netcryptm --encrypt \"" + VKONTAKTE_ACCESS_TOKEN + "\"").c_str()); 
	std::string crquery = miscExecSystem(std::string("../libcryptm/netcryptm --encrypt \"" + botname+";"+dbname + "\"").c_str());
	std::string crdbase = database->getFingerprint();

	std::string args = crtoken + " " + crdbase + " " + crquery;

	std::string output = miscExecSystem(std::string("python3 unbanner/unbanner.py " + args).c_str());

	std::cout << output << std::endl;

	std::string res = miscFindAndCopyString(output.c_str(), "---->", "<----");

	UNBANNER_MUTEX.unlock();
	return "Banned accounts sync by external unbanner: " + res;
}