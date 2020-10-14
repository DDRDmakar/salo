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

#include "../../headers/network.h"
#include "../../headers/interface.h"
#include "../../headers/message.h"


class networking_telegram : public interface
{
private:
	std::string ACCESS_TOKEN;
	std::string API_STRING;
	network* ptNetwork;

	bool IsSuccessful(const std::string& response);
	std::string getUnicode(const std::string& str);
	std::string inlineToConveerCommands(const std::string& str);
	bool haveCorrectGreeting(const std::string& message);

	int Auth(const std::string& token);
	void showAuthForm();

	int offset;

	bool fetchMessage();
	void pushMessage();

	bool firstFetch = true;
public:
	networking_telegram(void* botptr, const std::string& simnam, const std::string& snapname);
	~networking_telegram();
	
	bool _ENABLED;
	bool TX;

	void OnUpdate();

	void Tick() override;
	AnswerTelegram interfaceAnswerTG;
};