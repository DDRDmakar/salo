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

#ifndef VKCALLBACKAPI
#define VKCALLBACKAPI

#include "vkcom_connector.h"
#include "../../headers/conveer.h"


struct vkCallbackObject
{
	unsigned int saloId;
	std::string jsonData;
	bool isEmpty;
};


class CallbackApi : public Connector
{
private:
	std::string& selfId;
	std::string botname;
	bool initial_tick;
	Convspace::Conveer* callbackConveer;
	std::vector<unsigned int> toBeDeleted;
	unsigned int cacheCheckTimer = 0;

	bool haveCorrectGreeting(const std::string& messageText);

	vkCallbackObject decryptMessage(const std::string& message);
	vkcomBasicMessage processMessageObject(const std::string& jsonMessageObj);

	vkcomBasicEvent getEvent();
public:
	vkcomBasicMessage go();	

	CallbackApi(void* callbackedBot, std::string& vkSelfPageId);
	~CallbackApi();
};

#endif