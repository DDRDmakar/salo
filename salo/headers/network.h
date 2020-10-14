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