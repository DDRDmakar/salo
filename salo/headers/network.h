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

#ifndef NETW
#define NETW

#include <curl/curl.h>
#include "console.h"
#include <string.h>
#include <thread>
#include <chrono>

class NetworkResult
{
public:
	std::string url;
	std::string HEAD;
	std::string DATA;
	std::string cookie;
	int size;
	int hsize;
};



class network
{
private:
CURL* curl;

void easyinit();

bool silentStart = false;

public:
	network(bool silent = false);
	~network();

	int DebugMode;
	int OfflineMode;

	NetworkResult network_HTTPS_GET(const char* address, const std::string& cookiekeys = "(none)", int CRITICAL = 0, int timeout = 10);
	NetworkResult network_HTTPS_POST(const char* address, const char* postkeys, const std::string& cookiekeys = "(none)", int CRITICAL = 0);

	std::string urlencode(std::string what);	

};

#endif