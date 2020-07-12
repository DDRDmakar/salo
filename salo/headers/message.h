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

#ifndef sequence_check_message
#define sequence_check_message null

#include <iostream>
#include <string>
#include <stack>

typedef long long int BIGINT;

struct Person
{
	BIGINT userID; //id
	std::string Name; //имя
	std::string Last; //фамилия
	std::string vkIsPlatformMember; //вступил ли в группу Сала

	std::string IDENTIFICATOR; //hash или id пользователя
	std::string saloapi_login; // Логин в SaloAPI
};

struct CONFIGURATION
{
	void* BOT; //указатель на бота
	std::string INTERFACE; //интерфейс бота
	std::string INTERFACE_UNIQUE_IDENTIFICATOR; //уникальный идентификатор интерфейса

	bool isConf; //является ли сообщением из конфы
	BIGINT confId; //id конфы
	int confUsersCount; //количество пользователей в конфе ВК
	std::string confTitle; //название конфы ВК
	std::string attachment; //вложение
	std::string attachment_link; //ссылка на вложение
	std::string attachment_token; //дополнительный токен доступа вложения
	std::string attachment_jpeg; //ссылка на жпег
	
	std::string web_userhash; //хэш
	std::string web_ip; //ip
	std::string web_useragent; //данные об ОС и браузере
};

struct Answer
{
 	std::string messageText;
 	std::string messageAttachment;
 	BIGINT userID = 0;
 	bool HaveMessage = false;

 	//selfKick
 	bool selfKick = false;

 	//For error reporting
 	std::string errorText;
 	BIGINT errorID = 0;
 	bool HaveError = false;
}; 

struct AnswerWeb
{
	std::string messageText;
	std::string userHash;
	std::string cid;
	bool HaveMessage = false;
};

struct AnswerTelegram
{
 	std::string messageText;
 	std::string messageAttachment;
 	BIGINT chatID = 0;
 	int attachtype = 0;
 	bool HaveMessage = false;

 	//For error reporting
 	std::string errorText;
 	BIGINT errorID;
 	bool HaveError = false;
};


struct AnswerError
{
	//For error reporting
 	std::string errorText;
 	BIGINT errorID = 0;
 	bool HaveError = false;
};


extern std::stack<AnswerError> ErrorStack;


void AlertDevelopers(std::string messageText);

#endif