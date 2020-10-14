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

//Снапшоты - это сохраненные наборы ботов.

#ifndef SNAPS
#define SNAPS


#include <vector>
#include "bots.h"

class snapshotManager 
{
	std::string active_snapshot;
	std::vector<bot*> bots;

	bool unsaved_changes = false;

public:

	//manage snapshots itself
	std::string getActiveSnapshot() { return active_snapshot; };
	std::string listSnapshot(); //active snapshot
	std::string listSnapshots(); //all Snapshots
	void createSnapshot(const std::string& snap);

	//manage bots
	void addBot(const std::string& botname, bool reuse = false);
	void createBot(const std::string &argline);
	void addInterface(const std::string& botname, int type, const std::string& name, bool reuse = false);
	void deleteBot(const std::string& botname);
	void deleteInterface(const std::string& botname, const std::string& interfacename);
	void pauseBot(const std::string& botname);
	bot* getBotByName(const std::string& name);
	void startBots();
	void stopBots();
	void resetPassword(const std::string& botname, const std::string& interfacename);
	void reloadConveerConfigs();
	void toggleProcessingPause(const std::string& botname);

	//website vk bots info
	std::string getBotsInfo(bool official);

	//save-load system
	void LoadSnapshot(const std::string& snap);
	void ExitSnapshot();
	void setDefault(const std::string& snap); //уставнавливает снэпшот по умолчанию

	//destr
	~snapshotManager();
};

extern snapshotManager* snapManager;

#endif
