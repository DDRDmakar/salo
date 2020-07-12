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
#include <sstream>
#include "headers/telegram.h"
#include "../headers/misc.h"
#include "../headers/salostring.h"
#include "../headers/config.h"
#include "../headers/bots.h"


/*

Проверяет наличие метки OK в ответе Telegram API

*/

bool networking_telegram::IsSuccessful(const std::string& response)
{
	char* ALL_OK = miscFindAndCopy(response.c_str(), "{\"ok\":true", "}"); //{"ok":true

	if(ALL_OK == NULL)
	{
		return false;
	}
	else
	{
		delete[] ALL_OK;
		return true;
	}
}


/*

Раскодирование последовотельностей UTF-16 в готовые 4-байтные символы и запись их в мультибайт

*/

std::string networking_telegram::getUnicode(const std::string& str)
{
	return getUnicodeFromCodes(str);
}


/*

Препроцессинг инлайновых команд телеграма

*/

std::string networking_telegram::inlineToConveerCommands(const std::string& str)
{
	//std::cout << "inliner called" << std::endl;
	//все инлайновые команды начинаются с слэша
	if(str[0] == '/')
	{
		//в инлайновых командах телеграма запрещены пробелы
		if(str.find(" ") == std::string::npos)
		{			
			std::string word = str.substr(1);
		
			if(word.find("[") == std::string::npos && word.find("]") == std::string::npos
			&& word.find("/") == std::string::npos && word.find("\\") == std::string::npos)
			{
				if(workdat->there_is_element(std::string("telegram_inline_") + word))
				{
					//std::cout << "inliner detected command!" << std::endl;
					return workdat->get_keyword(std::string("telegram_inline_") + word);
				}
				else
				{
					return str;
				}
			}
			else
			{
				return str;
			}
		}
		else
		{
			return str;
		}
	}
	else
	{
		return str;
	}

}


/*

Удаляет заглавные буквы

*/

inline void lowercaseTG(std::wstring &current_line) 
{
	for(int i = 0; i < current_line.length(); ++i)
	{
		if(current_line [i] >= L'A' && current_line [i] <= L'Z') current_line [i] += L'z' - L'Z';
		if(current_line [i] >= L'А' && current_line [i] <= L'Я') current_line [i] += L'я' - L'Я';
		if(current_line [i] == L'Ё' ) current_line [i] = L'ё';
	}
}


/*

Проверяет наличие приветсвтия перед сообщением

*/

bool networking_telegram::haveCorrectGreeting(const std::string& message)
{
	bot* BOT = static_cast<bot*>(this->thisBot);
	std::vector<std::string> Greetings = BOT->getConveerPtr()->get_fetcher_keywords();

	std::string mtxt;

	if(message[0] != '/') mtxt = std::string(message); else mtxt = std::string(message.substr(1)); 
	std::string tmp;
	
	wchar_t* wst = MultibyteToWide(mtxt.c_str());
	std::wstring wmtxt = std::wstring(wst);

	if(wmtxt.length() > 20)
	{
		wmtxt = wmtxt.substr(0, 20);
	}

		lowercaseTG(wmtxt);

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
