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
#include "headers/translate.h"
#include "headers/misc.h"

//constr
Translate::Translate()
{	
	TranslatorEnabled = false;

	if(TranslatorEnabled)
	{
		std::cout << std::endl << "Yandex Transalte API init..." << std::endl;

		API_KEY = miscGetStringFromFile("resources/yandex_apikey.conf");

		std::cout << "Transalte key loaded..." << std::endl << "YANDEX: ";
		pYaNetwork = new network();
	}
}

//destr
Translate::~Translate()
{
	if(TranslatorEnabled)
	{
		std::cout << "Yandex Transalte API stopping..." << std::endl;
		delete pYaNetwork;
	}
}


/*

Функция вырезает все заэкраированые символы

*/

std::string preprocess(const std::string& Text)
{	
	std::string r;

	for(int i = 0; i < Text.length(); i++)
	{
		if(Text[i] == '\\')
		{
			r.push_back(' ');
			r.push_back(' ');
			i++;
		}
		else
		{
			r.push_back(Text[i]);
		}
	}

	return r;
}


/*

Фунция определения языка вводимого сообщения

*/

std::string Translate::DetectLanguage(const std::string& Text)
{
	if(!TranslatorEnabled)
	{
		return std::string("TRANSLATOR_FAILED");
	}

	std::string TextP = preprocess(Text);

	if(Text.length() > 5000)
	{
		return std::string("TRANSLATOR_FAILED");
	}

	std::string txt = pYaNetwork->urlencode(TextP);
	std::string api = "https://translate.yandex.net/api/v1.5/tr/detect?key=" + API_KEY; //+ "&text=" + txt;
	std::string post = "text=" + txt;

	NetworkResult result = pYaNetwork->network_HTTPS_POST(api.c_str(), post.c_str());

	//std::cout << result.DATA << std::endl;

	char* langcode = miscFindAndCopy(result.DATA.c_str(), "lang=\"", "\"");
	
	if(langcode == NULL)
	{
		return std::string("TRANSLATOR_FAILED");
	}

	std::string res = std::string(langcode);
	delete[] langcode;
	return res;
}


/*

Функция переводит текст между двумя языками

*/

std::string Translate::TranslateText(const std::string& Text, const std::string SourceLang, const std::string DestLang)
{
	if(!TranslatorEnabled)
	{
		return std::string("TRANSLATOR_FAILED");
	}

	std::string TextP = preprocess(Text);

	if(Text.length() > 5000)
	{
		return std::string("TRANSLATOR_FAILED");
	}

	std::string txt = pYaNetwork->urlencode(TextP);
	std::string api = "https://translate.yandex.net/api/v1.5/tr/translate?key=" + API_KEY + //+ "&text=" + txt;
	+ "&lang=" + SourceLang + "-" + DestLang;
	std::string post = "text=" + txt;

	NetworkResult result = pYaNetwork->network_HTTPS_POST(api.c_str(), post.c_str());

	//std::cout << result.DATA << std::endl;

	char* translation = miscFindAndCopy(result.DATA.c_str(), "<text>", "</text>");

	if(translation == NULL)
	{
		return std::string("TRANSLATOR_FAILED");
	}

	std::string res = std::string(translation);

	delete[] translation;
	return res;
}