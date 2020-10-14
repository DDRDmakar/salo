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

#include <vector> 
#include <string>
#include <chrono>
#include "headers/vkcom.h"
#include "../headers/misc.h"
#include "../headers/database.h"
#include "../headers/salostring.h"
#include "../headers/bots.h"
#include "../headers/config.h"
#include "../headers/security.h"
 


/*

Функция заполняет форму для логина, параметрами полученными со страницы для входа 

*/

void networking_vkcom::parseVkLoginPage(const char* pagedata, const char* headers)
{
	std::cout << "Prepearing authentification request..." << std::endl; 
	
	char* origin = miscFindAndCopy(pagedata, "name=\"_origin\" value=\"", "\"");
	char* iph = miscFindAndCopy(pagedata, "name=\"ip_h\" value=\"", "\"");
	char* lgh = miscFindAndCopy(pagedata, "name=\"lg_h\" value=\"", "\"");
	char* to = miscFindAndCopy(pagedata, "name=\"to\" value=\"", "\"");
	char* lhk = miscFindAndCopy(headers, "remixlhk=", ";");

	if(origin == NULL || iph == NULL || lgh == NULL || to == NULL || lhk == NULL)
	{
		throw 1;
	}
	else
	{
		FORM._origin = std::string(origin);
		FORM.ip_h = std::string(iph);
		FORM.lg_h = std::string(lgh);
		FORM.to = std::string(to);
		FORM.cookie_remixlhk = std::string(lhk);
	}
}


/*

Функция ищет токен в заголовках ответа сервера
Если токен не найден, значит авторизация не удалась - бросается исключение

*/

std::string networking_vkcom::findToken(const char* headerdata)
{
	std::cout << "Trying to get an access token..." << std::endl;

	std::string res;
	char* token = miscFindAndCopy(headerdata, "access_token=", "&");
	
	if(token == NULL) 
	{
		throw 4;
	}
	else
	{
		res = std::string(token);
	}
	
	return std::string(token);
}


/*

Функция возвращает время жизни выданного токена в секундах
Ищет в заголовках ответа, бросает исключение в случае ошибки

*/

int networking_vkcom::findLifetime(const char* headerdata)
{
	int secs;
	char* tm = miscFindAndCopy(headerdata, "expires_in=", "&");
	
	if(tm == NULL) 
	{
		throw 5;
	}
	else
	{
		secs = atoi(tm);
	}
	
	return secs;
}

/*

Функция ищет URL по которому производится подтверждение прав доступа

*/

std::string networking_vkcom::findPermissionUrl(const std::string& PermissionPage)
{

	char* tokenGrantUrl = miscFindAndCopy(PermissionPage.c_str(), " action=\"", "\">");

	if(tokenGrantUrl == NULL)
	{
		delete[] tokenGrantUrl;
		throw 8;
	}

	std::string tokenUrl(tokenGrantUrl);
	
	delete[] tokenGrantUrl;
	return tokenUrl;
}


/*

Функция ищет в заголовках cookie для подтверждения прав

*/

std::string networking_vkcom::findPermissionCookies(const std::string& PermissionHeader)
{
	std::string result;

	char* cookie_l = miscFindAndCopy(PermissionHeader.c_str(), "Set-Cookie: l=", ";");
	char* cookie_p = miscFindAndCopy(PermissionHeader.c_str(), "Set-Cookie: p=", ";");

	if(cookie_l != NULL && cookie_p != NULL)
	{
		result = "h=1; s=1; l=" + std::string(cookie_l) + "; p=" + std::string(cookie_p) + ";";
	}
	else
	{
		delete[] cookie_l;
		delete[] cookie_p;
		throw 7;
	}

	delete[] cookie_l;
	delete[] cookie_p;

	return result;
}


/*

Функция посимвольного сравнения строк

*/

bool compare(std::string templ, std::string userstring)
{	
	if(userstring.length() < templ.length())
	{
		return false;
	}

	if(userstring.length() > templ.length())
	{
		if(userstring[templ.length()] == '1' || userstring[templ.length()] == '2' || userstring[templ.length()] == '0')
		{
			return false;
		}
	}

	for(int i = 0; i < templ.length(); i++)
	{
		if(templ[i] != userstring[i])
		{
			return false;
		}
	}

	return true;
}


/*

Функция ищет в ответе сервера id пользователей, которым нужно отправить поздравления 

*/

std::vector<std::string> networking_vkcom::findBirthdateUsers(std::string userdata, std::string today, std::string tomonth)
{
	std::vector<std::string> res;
	std::string todate = today + "." + tomonth;
	//std::cout << userdata << std::endl;

	char* oString = miscFindAndCopy(userdata.c_str(), "\"items\":[", "]");
	char* pString = oString;

	int ex = 0;

	while(ex == 0)
	{
		pString = strchr(pString, '{');

		if(pString == NULL)
		{
			ex = 1;
		}
		else
		{
			char* user_block = miscFindAndCopy(pString, "{", "}");

			if(user_block == NULL)
			{
				ex = 1;
			}
			else
			{
				char* user_bdate = miscFindAndCopy(user_block, "\"bdate\":\"", "\"");

				if(user_bdate != NULL)
				{
					if(compare(todate, std::string(user_bdate)))
					{
						//у этого юзера др, два чая ему
						char* user_id = miscFindAndCopy(user_block, "\"id\":", ",");
						res.push_back(std::string(user_id));
						delete[] user_id;
					}
					delete[] user_bdate;
				}

				//std::cout << "+";
				delete[] user_block;
				pString++;
			}
		}
	}

	delete[] oString;

	return res;
}


int networking_vkcom::getPromotionTimer(int timer)
{
	if(isOnPromotionStreak)
	{
		return timer;
	}
	else
	{
		return 0;
	}
}

/*

Возвращает следующее промо-сообщение.

*/

int NEXT_PROMOTION = -1;
std::string networking_vkcom::getNextPromotion()
{
	NEXT_PROMOTION++;
	std::vector<std::string> promotions = workdat->get_group("promotion_link"); 

	if(NEXT_PROMOTION == promotions.size())
	{
		NEXT_PROMOTION = -1;
		isOnPromotionStreak = false;
		return std::string("");
	}

	return promotions[NEXT_PROMOTION];
}


/*

Удаляет заглавные буквы

*/

inline void lowercaseVK(std::wstring &current_line) 
{
	for(int i = 0; i < current_line.length(); ++i)
	{
		if(current_line [i] >= L'A' && current_line [i] <= L'Z') current_line [i] += L'z' - L'Z';
		if(current_line [i] >= L'А' && current_line [i] <= L'Я') current_line [i] += L'я' - L'Я';
		if(current_line [i] == L'Ё' ) current_line [i] = L'ё';
	}
}


/*

Возвращает ture, если перед сообщением стоит правильное приветсвие

*/

bool Fetcher::haveCorrectGreeting(const std::string& messageText)
{
	std::vector<std::string> Greetings = this->fetcherConveer->get_fetcher_keywords(); 
	std::string mtxt = std::string(messageText);
	std::string tmp;
	
	wchar_t* wst = MultibyteToWide(mtxt.c_str());
	std::wstring wmtxt = std::wstring(wst);

	if(wmtxt.length() > 20)
	{
		wmtxt = wmtxt.substr(0, 20);
	}

		lowercaseVK(wmtxt);

	char* mbt = WideToMultibyte(wmtxt.c_str());
	mtxt = std::string(mbt);

	delete[] wst;
	delete[] mbt;

	for(int i = 0; i < Greetings.size(); i++)
	{
		if(Greetings[i].length() > mtxt.length()) { continue; }
		tmp = mtxt.substr(0, Greetings[i].length());
		if(tmp == Greetings[i]) { return true; }
	}

	return false;
}


/*

Возвращает ture, если перед сообщением стоит правильное приветсвие

*/

bool CallbackApi::haveCorrectGreeting(const std::string& messageText)
{
	//std::cout << "[club"+selfId+"|" << std::endl;
	if (messageText.find("[club"+selfId+"|") != std::string::npos)
	{
		return true;
	}
	
	std::vector<std::string> Greetings = this->callbackConveer->get_fetcher_keywords(); 
	std::string mtxt = std::string(messageText);
	std::string tmp;
	
	wchar_t* wst = MultibyteToWide(mtxt.c_str());
	std::wstring wmtxt = std::wstring(wst);

	if(wmtxt.length() > 20)
	{
		wmtxt = wmtxt.substr(0, 20);
	}

		lowercaseVK(wmtxt);

	char* mbt = WideToMultibyte(wmtxt.c_str());
	mtxt = std::string(mbt);

	delete[] wst;
	delete[] mbt;

	for(int i = 0; i < Greetings.size(); i++)
	{
		if(Greetings[i].length() > mtxt.length()) { continue; }
		tmp = mtxt.substr(0, Greetings[i].length());
		if(tmp == Greetings[i]) { return true; }
	}

	return false;
}

/*

Возвращает fingerprint для SCRS

*/

std::string networking_vkcom::getSCRSfingerprint()
{
	std::string path;
	bot* Bot = static_cast<bot*>(this->thisBot);

	if(!isGroupBot)
	{
		path = "bots/"+this->SnapshotName+"/"+Bot->getName()+"/i0_"+this->INTERFACE_SIMPLE_NAME+".account";
	}
	else
	{
		path = "bots/"+this->SnapshotName+"/"+Bot->getName()+"/i3_"+this->INTERFACE_SIMPLE_NAME+".account";
	}

	VKAccountData dat = sec->LoadVK(path);

	if(dat.isEmpty)
		return std::string();


	if(this->isGroupBot)
	{
		//1: user prefix
		std::string userPrefix;
		userPrefix = "g";

		//2: user numeric id
		
	 	std::string userNumeric = this->selfPageId;

		//std::string apiurl = "https://api.vk.com/method/groups.getById?v=5.62&access_token=" + VKONTAKTE_ACCESS_TOKEN;
		//NetworkResult res = this->pNetwork->network_HTTPS_GET(apiurl.c_str());

		//userNumeric = miscFindAndCopyString(res.DATA.c_str(), "\"id\":", ",");

		//ошибка получения id
		//if(userNumeric == std::string())
		//	return std::string();



		//3: pause, add, return

		//std::this_thread::sleep_for(std::chrono::milliseconds(300));
		std::string fingerprint = userPrefix+userNumeric+"zz"+dat.passw;

		return fingerprint;
	}
	else
	{
		//1: user prefix
		std::string userPrefix;
		userPrefix = "u";

		//2: user numeric id
		std::string userNumeric = this->selfPageId;

		//std::string apiurl = "https://api.vk.com/method/users.get?v=5.62&access_token=" + VKONTAKTE_ACCESS_TOKEN;
		//NetworkResult res = this->pNetwork->network_HTTPS_GET(apiurl.c_str());

		//userNumeric = miscFindAndCopyString(res.DATA.c_str(), "\"id\":", ",");

		//ошибка получения id
		//if(userNumeric == std::string())
		//		return std::string();



		//3: pause, add, return

		//std::this_thread::sleep_for(std::chrono::milliseconds(300));
		std::string fingerprint = userPrefix+userNumeric+"zz"+dat.login+"@"+dat.passw;

		return fingerprint;
	}


}


/*

Репост указанного поста. 

*/

std::string networking_vkcom::makeRepost(const std::string& postId, bool pin)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	std::string apiurl = "https://api.vk.com/method/wall.repost?object="+postId+"&v=5.62&access_token=" + VKONTAKTE_ACCESS_TOKEN;
	NetworkResult res = this->pNetwork->network_HTTPS_GET(apiurl.c_str());

	//std::cout << res.DATA << std::endl;

	if(pin)
	{
		std::string newpostid = miscFindAndCopyString(res.DATA.c_str(), "\"post_id\":", ",");

		apiurl = "https://api.vk.com/method/wall.pin?post_id="+newpostid+"&v=5.62&access_token=" + VKONTAKTE_ACCESS_TOKEN;
		res = this->pNetwork->network_HTTPS_GET(apiurl.c_str());
	}

	return "Done.";
}

