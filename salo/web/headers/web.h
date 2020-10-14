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
#include <thread>
#include <mutex>

#include "../../headers/interface.h"
#include "../../headers/message.h"

class webinterface : public interface
{
private:
	bool ProcessMessage(const std::string& fname);
	void PushMessage();

	void updateStats();
	int k;

	std::thread spawn_updater() { return std::thread([this] { this->updateStats(); }); }
	std::thread serviceThread;

	std::mutex SERVICE_MUTEX;

public:
	webinterface();
	~webinterface();
	
	void OnUpdate();
	void Tick() override;

	bool enabled;

	AnswerWeb interfaceAnswerWeb;
};
