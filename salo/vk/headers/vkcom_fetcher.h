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