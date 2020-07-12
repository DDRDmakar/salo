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

#ifndef VKFETCHER
#define VKFETCHER

#include "../../headers/conveer.h"
#include "../../headers/network.h"
#include "vkcom_connector.h"

class Fetcher : public Connector
{
private:
	std::vector<unsigned int> CACHE_ID;

	void addToCache(const unsigned int& id);
	bool isInCache(const unsigned int& id);
	bool haveCorrectGreeting(const std::string& messageText);

	void vkDownload(const std::string& vkAnswer);

	std::vector<vkcomBasicMessage> CURRENT_MESSAGES;
	int processed_messages;

	bool initial_tick;

	Convspace::Conveer* fetcherConveer;
	network* fetcherNetwork; 

	std::string botname;
	std::string& token;

	int accoutErrorsCount;

public:
	vkcomBasicMessage go();
	
	Fetcher(void* fetcherBot, network* vkNetwork, std::string& fetcherToken, bool isGroupBot);
	~Fetcher();
	
};

#endif