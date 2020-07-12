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

#include <iostream>
#include "headers/telegram.h"
#include "../headers/console.h"
#include "../headers/security.h"
#include "../headers/bots.h"


/*

Диалог с пользователем, ввод токена доступа

*/

void networking_telegram::showAuthForm()
{
	bool autologin = true;

	bot* Bot = static_cast<bot*>(this->thisBot);
	std::string path = "bots/"+this->SnapshotName+"/"+Bot->getName()+"/i2_"+this->INTERFACE_SIMPLE_NAME+".account";

	TGAccountData dat = sec->LoadTG(path);

	std::string token;
	
	//<< "Or type here \"skip\" if you dont whant to enable telegram in this session." << std::endl;
	std::cout << std::endl << "+===========[ Telegram AUTH ]===========+" << std::endl;
	if(dat.isEmpty)
	{
		std::cout << "Please paste here unique Telegram Bot access token." << std::endl; 
		std::cout << "TOKEN: ";
		std::cin >> token;
		autologin = false;
	}
	else
	{
		std::cout << "\nFound saved data, logging in automatically.\n";
		std::cout << "Found in: " << path << "\n\n";
		token = dat.token;
		autologin = true;
	}
	std::cout <<  "+=======================================+" << std::endl << std::endl;

	int result = Auth(token);

	if(result)
	{
		if(!autologin)
		{
			TGAccountData sav;
			sav.token = token;
			sec->SaveTG(sav, path);
		}

	}

	//return token;
}


/*

Главная функция для аутентификации

*/

int networking_telegram::Auth(const std::string& token)
{
	ACCESS_TOKEN = token;

	if(ACCESS_TOKEN == std::string("skip"))
	{
		this->_ENABLED = false;
		return 0;
	}
	else
	{
		this->_ENABLED = true;
		API_STRING = "https://api.telegram.org/bot" + ACCESS_TOKEN + "/";
		std::string api = API_STRING + "getMe";
		NetworkResult res = ptNetwork->network_HTTPS_GET(api.c_str());
		//std::cout << "\n\n" << res.DATA << "\n\n";

		if(IsSuccessful(res.DATA))
		{
			std::cout << "Access token checked successfully. Logged in." << std::endl;
			INTERFACE_IDENTIFICATOR = ACCESS_TOKEN.substr(ACCESS_TOKEN.length() - 16);
			std::cout << "Unique identifier = " << INTERFACE_IDENTIFICATOR << std::endl;
			return 1;
		}
		else
		{
			std::cout << std::endl<< "\tTelegram access token check failed!" << std::endl;
			//ServerAlive = 0;
			this->_ENABLED = false;
			return 0;
		}

	}

	return 0;
}