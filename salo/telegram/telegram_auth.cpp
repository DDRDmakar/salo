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