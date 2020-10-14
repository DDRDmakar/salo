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
#include <mutex>
#include <time.h>

#include "headers/external_api.h"
#include "headers/network.h"
#include "headers/misc.h"
#include "headers/json.h"
#include "headers/message.h"
#include "headers/config.h"
#include "headers/log.h"

int INITIALIZED = 0;
network* pApiNetwork;
std::string WEATHER_APPID;

std::mutex EXTERNAL_MUTEX;


/*

Функция инициализирует внешние API, вызывается один раз при старте

*/

void initapi()
{
	std::cout << "EXTERNAL API: ";
	pApiNetwork = new network();
	WEATHER_APPID = miscGetStringFromFile("resources/weather_appid.conf");
	INITIALIZED = 1;
}


/*

Функция ищет по википедии, возвращает пустую строку если ничего не найдено

*/

std::string wikipedia(const std::string& query)
{
	EXTERNAL_MUTEX.lock();

	srand(time(NULL));

	int callid = rand() % 256;
	logs->CommitGeneric(F, L, "wikipedia external_api called, call id = " + std::to_string(callid));    
	logs->CommitGeneric(F, L, "wikipedia query for call id = " + std::to_string(callid) + " is \"" + query + "\"");   

	if(INITIALIZED == 0)
	{
		initapi();
	}

	if(query.length() <= 2 || query.length() >= 220)
	{
		AlertDevelopers("Wikipedia: Mediawiki API: Request too long.");
		logs->CommitGeneric(F, L, "Wikipedia finished, call id = " + std::to_string(callid) + " closed");
		EXTERNAL_MUTEX.unlock();
		return std::string();
	}

	std::string api = "https://ru.wikipedia.org/w/api.php?action=opensearch&format=json&utf8=1&redirects=resolve&search=";
	std::string urlquery = pApiNetwork->urlencode(query);
	api += urlquery;

	NetworkResult res = pApiNetwork->network_HTTPS_GET(api.c_str());

	//std::cout << res.DATA << std::endl;
	
	rapidjson::Document d;
	ParseResult ok = d.Parse(res.DATA.c_str());

	if(!ok || !d.IsArray()) 
	{
		fprintf(stderr, "JSON parse error: %s (%u)", GetParseError_En(ok.Code()), ok.Offset());
		std::string rapiderr = std::string("JSON parse error: " + std::string(GetParseError_En(ok.Code())) 
			+ " -- " + std::to_string(ok.Offset()));
		AlertDevelopers("Wikipedia: Mediawiki API: JSON object not created!\n\nMediawiki says: " + res.DATA
			+ "\n\nRapidjson says: " + rapiderr);

		logs->CommitGeneric(F, L, "Wikipedia finished, call id = " + std::to_string(callid) + " closed");    	
		EXTERNAL_MUTEX.unlock();
		return std::string();
	}

	if(d.Size() < 3)
	{
		fprintf(stderr, "JSON parse error: %s (%u)", GetParseError_En(ok.Code()), ok.Offset());
		std::string rapiderr = std::string("JSON parse error: " + std::string(GetParseError_En(ok.Code())) 
			+ " -- " + std::to_string(ok.Offset()));
		AlertDevelopers("Wikipedia: Mediawiki API: JSON object not created!\n\nMediawiki says: " + res.DATA
			+ "\n\nRapidjson says: " + rapiderr);

		logs->CommitGeneric(F, L, "Wikipedia finished, call id = " + std::to_string(callid) + " closed");       
		EXTERNAL_MUTEX.unlock();
		return std::string();
	}

	const rapidjson::Value& items = d[2];

	if(!items.IsArray())
	{
		AlertDevelopers("Wikipedia: Mediawiki API: JSON object with definition list not created!\n\nMediawiki says: " + res.DATA);
		logs->CommitGeneric(F, L, "Wikipedia finished, call id = " + std::to_string(callid) + " closed");
		EXTERNAL_MUTEX.unlock();
		return std::string();
	}

	std::string response;
	int variants = items.Size();

	if(variants == 0)
	{
		//AlertDevelopers("Wikipedia: Mediawiki API: definition list have 0 size (maybe, its just not found)\n\nRequest was: " + query
		//	+ "\n\nMediawiki says: " + res.DATA);
		logs->CommitGeneric(F, L, "Wikipedia finished, call id = " + std::to_string(callid) + " closed");
		EXTERNAL_MUTEX.unlock();
		return std::string();
	}

	srand(time(NULL));    
	int variant = 0;

	if(!items[0].IsString())
	{
		AlertDevelopers("Wikipedia: Mediawiki API: IsString() returned false, aborting. \n\nMediawiki says: " + res.DATA);
		logs->CommitGeneric(F, L, "Wikipedia finished, call id = " + std::to_string(callid) + " closed");
		EXTERNAL_MUTEX.unlock();
		return std::string();
	}

	std::string trivial = std::string(items[0].GetString());
	std::size_t isTrivial = trivial.find(':');

	if(variants > 1 && isTrivial != std::string::npos)
	{
		variant = (rand() % (variants-1)) + 1;
		logs->CommitGeneric(F, L, "Wikipedia: Mediawiki API: No trivial definition detected, choosing randomly!");
		logs->CommitGeneric(F, L, "Wikipedia: Mediawiki API: Choosed up definition " + std::to_string(variant) + " from " + std::to_string(variants) + " definitions\n");
		
			if(!items[variant].IsString())
			{
				AlertDevelopers("Wikipedia: Mediawiki API: IsString() returned false, aborting. \n\nMediawiki says: " + res.DATA);
				logs->CommitGeneric(F, L, "Wikipedia finished, call id = " + std::to_string(callid) + " closed");
				EXTERNAL_MUTEX.unlock();
				return std::string();
			}
	}
	else
	{
		logs->CommitGeneric(F, L, "Wikipedia: Mediawiki API: Found definition successfully.");
		variant = 0;
	}

	response = std::string(items[variant].GetString()); 
   
	//std::cout << response << std::endl;

	logs->CommitGeneric(F, L, "Wikipedia finished, call id = " + std::to_string(callid) + " closed");
	EXTERNAL_MUTEX.unlock();
	return response;
}


/*

Функция выдает строку с текущей погодой

*/

std::string weather(const std::string& query)
{
	EXTERNAL_MUTEX.lock();

	int callid = rand() % 256;
	logs->CommitGeneric(F, L, "openweathermap external_api called, call id = " + std::to_string(callid));    
	logs->CommitGeneric(F, L, "openweathermap query for call id = " + std::to_string(callid) + " is \"" + query + "\"");

	if(INITIALIZED == 0)
	{
		initapi();
	}

	std::string urlquery = pApiNetwork->urlencode(query);
	std::string api = "http://api.openweathermap.org/data/2.5/weather?q="+urlquery+"&appid="+WEATHER_APPID+"&lang=ru";
	NetworkResult res = pApiNetwork->network_HTTPS_GET(api.c_str());

	//std::cout << res.DATA << std::endl;

	rapidjson::Document d;
	ParseResult ok = d.Parse(res.DATA.c_str());

	if(!ok || !d.IsObject()) 
	{
		fprintf(stderr, "JSON parse error: %s (%u)", GetParseError_En(ok.Code()), ok.Offset());
		std::string rapiderr = std::string("JSON parse error: " + std::string(GetParseError_En(ok.Code())) 
			+ " -- " + std::to_string(ok.Offset()));
		AlertDevelopers("OpenWeatherMap API: JSON object not created!\n\nOpenWeatherMap says: " + res.DATA
			+ "\n\nRapidjson says: " + rapiderr);

		logs->CommitGeneric(F, L, "openweathermap finished, call id = " + std::to_string(callid) + " closed");       	
		EXTERNAL_MUTEX.unlock();
		return std::string("Нет погодных данных или ошибка сервиса погоды");
	}

	if(d["cod"].IsString()) 
	{ 
		if(std::string(d["cod"].GetString()) != "200") 
		{
			if(std::string(d["cod"].GetString()) != "404")
			{
				AlertDevelopers("OpenWeatherMap API: bad call status (HTTP status != 200 && != 404)!\n\nOpenWeatherMap says: " + res.DATA);
			}

			logs->CommitGeneric(F, L, "openweathermap finished, call id = " + std::to_string(callid) + " closed");
			EXTERNAL_MUTEX.unlock(); 
			
				return std::string("Нет погодных данных или указан неверный город. \n\n \
				Мы испытваем временные трудности с русскими и украинскими названиями городов! \n \
				Попробуйте ввести название города используя международное английское название: например Moscow, Saint Petersburg, Kiev\n\
				\nПриносим извинения за доставленные трудности! См. группу для получения большей информации."); 
		}
	}
	else
	{
		if(d["cod"].IsInt())
		{
			if(d["cod"].GetInt() != 200)
			{
				if(d["cod"].GetInt() != 404)
				{
					AlertDevelopers("OpenWeatherMap API: bad call status (HTTP status != 200 && != 404)!\n\nOpenWeatherMap says: " + res.DATA);
				}

				logs->CommitGeneric(F, L, "openweathermap finished, call id = " + std::to_string(callid) + " closed");
				EXTERNAL_MUTEX.unlock(); 
				
					return std::string("Нет погодных данных или указан неверный город. \n\n \
					Мы испытваем временные трудности с русскими и украинскими названиями городов! \n \
					Попробуйте ввести название города используя международное английское название: например Moscow, Saint Petersburg, Kiev\n\
					\nПриносим извинения за доставленные трудности! См. группу для получения большей информации."); 
			}	
		}
		else
		{
			AlertDevelopers("OpenWeatherMap API: call status is null or undefined!\n\nOpenWeatherMap says: " + res.DATA);
			logs->CommitGeneric(F, L, "openweathermap finished, call id = " + std::to_string(callid) + " closed");
			EXTERNAL_MUTEX.unlock(); 
			return std::string("Нет погодных данных или указан неверный город."); 
		}
	}


	
	const rapidjson::Value& wparams = d["main"];
	const rapidjson::Value& witems = d["weather"][0];
	const rapidjson::Value& wwind = d["wind"];

	if(!wparams.IsObject() || !witems.IsObject() || !wwind.IsObject())
	{
		AlertDevelopers("OpenWeatherMap API: JSON object not created!\n\nOpenWeatherMap says: " + res.DATA);
		logs->CommitGeneric(F, L, "openweathermap finished, call id = " + std::to_string(callid) + " closed");    	
		EXTERNAL_MUTEX.unlock();
		return std::string("Нет погодных данных или ошибка сервиса погоды");
	}

	//Погодные данные
	float temeratureK = wparams["temp"].GetFloat(); //K 
	float temeratureC = temeratureK - 273.15; //C
	float pressure = 0.75006375541921 * wparams["pressure"].GetFloat(); //mmHg
	int humidity = wparams["humidity"].GetInt(); //%

	//Ветер
	float windSpeed = wwind["speed"].GetFloat(); //meter/second

	//Описание погоды
	int weatherCode = witems["id"].GetInt(); //code
	std::string weatherDesc = std::string(witems["description"].GetString()); //Описание
	std::string weatherPhrase = workdat->get_group_random("weather" + std::to_string(weatherCode));

	//Погодная станция:
	std::string weatherStation = std::string(d["name"].GetString());

	//Вывод
	std::string t;

	t += weatherPhrase + "\n\n";
	t += "На улице: " + weatherDesc + "\n";
	t += "Темература: " + miscStringDouble(temeratureC, 1) + " °C (" + miscStringDouble(temeratureK, 2) + " по Кельвину)\n";
	t += "Давление: " + miscStringDouble(pressure, 3) + " мм рт. ст.\n";
	t += "Влажность: " + std::to_string(humidity) + "%\n";
	t += "Ветер: " + miscStringDouble(windSpeed, 1) + " м/c\n";
	t += "Ближайшая метеорологическая станция: " + weatherStation + "\n";

	//t += "Код состояния: " + std::to_string(weatherCode) + "\n"; 

	logs->CommitGeneric(F, L, "openweathermap finished, call id = " + std::to_string(callid) + " closed");
	EXTERNAL_MUTEX.unlock();
	return std::string(t);
}


/*

Функция выдает строку с речью на n предложений

*/

std::string generateSpeech(int n)
{
	if(n < 1 || n > 25)
	{
		return std::string("Количество предложений в речи должно быть от 1 до 25!");
	}

	std::string res;
	srand(time(NULL));

	//вступительное предложение
	res += workdat->get_group_random("speechmaker0") + "\n\n";

	for(int i = 0; i < n; i++)
	{
		//составляем предложение из рандомных частей 
		//универсального кода речей
		
		res += workdat->get_group_random("speechmaker1");
		res += workdat->get_group_random("speechmaker2");
		res += workdat->get_group_random("speechmaker3");
		res += workdat->get_group_random("speechmaker4");

		//с вероятностью 20% новый параграф.
		if((rand() % 10) < 3)
		{
			res += "\n\n";
		}
	}

	//заключитльное предложение
	res += "\n\n" + workdat->get_group_random("speechmaker5");

	return res;
}