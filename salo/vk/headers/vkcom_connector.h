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

#ifndef VKCONNECTOR
#define VKCONNECTOR


struct vkcomBasicEvent
{
	std::string eventType;
	std::string userId;
	bool isEmpty = true;
};


struct vkcomBasicMessage
{
	bool have_message;
	unsigned int id;
	std::string user_id;
	std::string text;
	bool is_conf;
	std::string conf_id;
	bool have_attachment;
	std::string attachment_type;
	std::string attachment_link;
	std::string attachment_token;
	std::string attachment_jpeg;

	bool loginError = false;
	
	bool haveEvent = false;
	vkcomBasicEvent event;
};


//абстрактный класс для fetcher-legacy и callbackapi-коннектора
class Connector
{
public:
	virtual vkcomBasicMessage go() = 0;

	std::string connectorType;

	int mess_lookups;
	int cach_lookups;
};

#endif