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
#include "../headers/misc.h"
#include "../headers/security.h"
#include "../headers/bots.h"


/*

Форма ввода логина/пароля от вк
Автологинится если есть возможность

*/

void networking_vkcom::showAuthForm()
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

	bool autologin = true;
	std::string login;
	std::string passwd;

	VKAccountData dat = sec->LoadVK(path);
	
	std::cout << std::endl << "+============[ vk.com AUTH ]============+" << std::endl;
	if(dat.isEmpty)
	{
		if(!isGroupBot)
		{
			std::cout << "Login: ";
			std::cin >> login;
			std::cout << "Password: ";
			passwd = miscInputPassword();
		}
		else
		{
			std::cout << "Group Token: ";
			std::cin >> passwd;
			login = "group";
		}
		
		autologin = false;
	}
	else
	{
		std::cout << "\nFound saved data, logging in automatically.\n";
		std::cout << "Found in: " << path << "\n";
		login = dat.login;
		passwd = dat.passw;
		autologin = true;
	}
	std::cout << std::endl << "+=======================================+" << std::endl << std::endl;

	int result;

	if(!isGroupBot)
	{
		result = auth_login(login, passwd);
	}
	else
	{
		VKONTAKTE_ACCESS_TOKEN = passwd;
		INTERFACE_IDENTIFICATOR = passwd.substr(passwd.length() - 16);
		std::cout << "Unique identifier = " << INTERFACE_IDENTIFICATOR << std::endl;
		LoggedIn = true;
		result = 1;

		//вошли успешно, нужен id группы
		std::string userNumeric;
		std::string apiurl = "https://api.vk.com/method/groups.getById?v=5.62&access_token=" + VKONTAKTE_ACCESS_TOKEN;
		NetworkResult res = this->pNetwork->network_HTTPS_GET(apiurl.c_str());
		userNumeric = miscFindAndCopyString(res.DATA.c_str(), "\"id\":", ",");
		if(userNumeric == std::string()) { result = 0; LoggedIn = false; }
		this->selfPageId = userNumeric;
	}
	

	if(result)
	{
		if(!autologin)
		{
			VKAccountData sav;
			sav.login = login;
			sav.passw = passwd;
			sec->SaveVK(sav, path);
		}

	}
}


/*

Функция ловит бросаемые исключения 

*/

int networking_vkcom::auth_login(const std::string& login, const std::string& passwd)
{

	//TODO: не падать при ошибке авторизации

	int r = 0;
	logs->CommitGeneric(F, L, "auth_login called");

	try
	{
		r = __auth_login(login, passwd);
	}
	catch(int ex)
	{
		switch(ex)
			{
				case 1: std::cout << "Error while vk page parsing!" << std::endl; break;
				case 4: std::cout << "Authentification failed, check login/password and connection." << std::endl; break;
				case 5: std::cout << "Authentification failed, check login/password and connection." << std::endl; break;
				case 6: std::cout << "Authentification failed, check connection." << std::endl; break;
				case 7: std::cout << "Invalid login/password, or permission grant failed." << std::endl; break;
				case 8: std::cout << "Permission grant failed." << std::endl; break;
				case 9: std::cout << "Networking failed while authentificating, please re-auth later." << std::endl; break;
			}
		
		//ServerAlive = 0;
		
		//*(this->state) = false;
		//thisState = false;

		return 0;
	}

	return r;
}


/*

Основная функция авторизации
Скачивает страницу авторизации, заполняет форму, делает post-запрос, находит токен и время жизни и сохраняет
Не должна вызыватся напрямую, неотловленные исключения

*/

int networking_vkcom::__auth_login(const std::string& login, const std::string& passwd)
{
	//vkontakte api auth address
	//https://oauth.vk.com/authorize?client_id=4943149&scope=messages&redirect_uri=blank.html&display=page&v=5.33&response_type=token 

	//throw 9;

	//STEP 1 -- скачиваем страницу с формой, парсим форму и заполняем

	std::string authPageUrl = "https://oauth.vk.com/authorize?client_id=" + miscGetStringFromFile("resources/vk_appid.conf")
													+ "&scope=12290&redirect_uri=blank.html&display=page&v=5.50&response_type=token";


	std::cout << "Starting login..." << std::endl;
	NetworkResult page = pNetwork->network_HTTPS_GET(authPageUrl.c_str(), "(none)", 0);
	if(page.hsize == 0) throw 9;
	
	FORM.email = login;
	FORM.pass = passwd;
	
	parseVkLoginPage(page.DATA.c_str(), page.HEAD.c_str());
	std::string POSTQERY = "_origin="+FORM._origin+"&ip_h="+FORM.ip_h+"&lg_h="+FORM.lg_h+"&to="+FORM.to+"&email="+FORM.email+"&pass="+FORM.pass;
	std::string COOKIE = "remixlang=0; remixlhk="+FORM.cookie_remixlhk+";";
	
	//STEP 2 -- отправляем страницу с формой POST-запросом

	std::cout << "Logging in..." << std::endl;
	NetworkResult permisResult = pNetwork->network_HTTPS_POST("https://login.vk.com/?act=login&soft=1&utf8=1", POSTQERY.c_str(), COOKIE, 0);
	if(permisResult.hsize == 0) throw 9;

	//STEP 2.5 -- проверяем необходимы ли разрешения. Если да - соглашаемся.

	bool need_permissions = true;
	NetworkResult authResult;

	try 
	{
		try 
		{
			findPermissionUrl(permisResult.DATA);
		} 
		catch(int e)
		{
			if(e == 8) 
			{
				std::cout << "No access permissions to be accepted in this session... Skipping this step." << std::endl;
				need_permissions = false;
			}
		}
	}
	catch(...)
	{
		logs->CommitGeneric(F, L, "Unhandled exeption occured");
	}

	if(need_permissions)
	{
		std::cout << "Accepting permissions..." << std::endl;
		std::string tokenUrl = findPermissionUrl(permisResult.DATA);
		std::string permissionCookie = findPermissionCookies(permisResult.HEAD) + " " + COOKIE;

		authResult = pNetwork->network_HTTPS_POST(tokenUrl.c_str(), "", permissionCookie, 0);
		if(authResult.hsize == 0) throw 9;

	}	
	else
	{
		authResult = permisResult;
	}

	if(authResult.hsize < 700)
	{
		throw 6;
	}

	VKONTAKTE_ACCESS_TOKEN = findToken(authResult.HEAD.c_str());
	TOKEN_LIFETIME = findLifetime(authResult.HEAD.c_str());

	//STEP 3 -- успешный вход

	std::cout << std::endl << "LOGGED IN SUCCESSFULLY!" << std::endl;
	miscPrintAccessToken(VKONTAKTE_ACCESS_TOKEN); 
	std::cout << "Token valid for " << TOKEN_LIFETIME << " seconds (" << miscSecondsToHours(TOKEN_LIFETIME) << " hours)" << std::endl;
	LoggedIn = true;

	SESSION_START = miscUnixTimeNow();
	logs->CommitGeneric(F, L, "INTERFACE VKCOM LOGGED IN SUCCESSFULLY");

	INTERFACE_IDENTIFICATOR = login.substr(login.length() - 7);
	std::cout << "Unique identifier = " << INTERFACE_IDENTIFICATOR << std::endl;

	//получилось войти, нужно взять id ВК
	std::string userNumeric;
	std::string apiurl = "https://api.vk.com/method/users.get?v=5.62&access_token=" + VKONTAKTE_ACCESS_TOKEN;
	NetworkResult res = this->pNetwork->network_HTTPS_GET(apiurl.c_str());
	userNumeric = miscFindAndCopyString(res.DATA.c_str(), "\"id\":", ",");
	this->selfPageId = userNumeric;
	
	return 1;
}


/*

Функция выхода из профиля

*/

int networking_vkcom::auth_logout()
{
	LoggedIn = false;
	VKONTAKTE_ACCESS_TOKEN = "";
	TOKEN_LIFETIME = 0;

	std::cout << "LOGGED OUT!" << std::endl;
	logs->CommitGeneric(F, L, "LOGGED OUT!");

	return 1;
}


/*

Перезаходит в профиль снова (логаут и логин)
При этом время жизни и токен обновляются
Функцию необходимо вызывать при окончанию времени жизни токена, или если токен внезапно стал невалидным

*/

int networking_vkcom::auth_relogin()
{
	logs->CommitGeneric(F, L, "AuthRelogin Called!");
	relogin_procedure = true;

	auth_logout();
	auth_login(FORM.email, FORM.pass);

	logs->CommitGeneric(F, L, "AuthRelogin success!");
	relogin_procedure = false;

	return 1;
}


/*

Выводит информацию о времени начала текущей сессии, конца сессии и время до рестарта
Вызвается через консольную команду

*/

std::string networking_vkcom::printTimings()
{
	long rightnow = miscUnixTimeNow();
	long timeleft = ((SESSION_START + TOKEN_LIFETIME) - (60*60)) - rightnow;	

	std::string res;

	res += "Session started: " + std::to_string(SESSION_START) + "\n";
	res += "Session lifetime: " + std::to_string(TOKEN_LIFETIME) + "\n";
	res += "Session end: " + std::to_string(SESSION_START + TOKEN_LIFETIME) + "\n";
	res += "Time before next session restart: " + std::to_string(timeleft) + "\n\n";
	res += "IDLE timer: " + std::to_string(idle_timer) + "\n";
	res += "Promotion timer: " + std::to_string(promotion_timer);

	return res;
}


/*

Проверяет, не кончается ли время токена, чтобы вовремя перелогиниться

*/

void networking_vkcom::checkTimings()
{
	long rightnow = miscUnixTimeNow();
	long timeleft = ((SESSION_START + TOKEN_LIFETIME) - (60*60)) - rightnow;	

	if(timeleft <= 0 && LoggedIn && !relogin_procedure)
	{
		logs->CommitGeneric(F, L, "AuthRelogin trigerred by token lifetime end");
		auth_relogin();
	}
}