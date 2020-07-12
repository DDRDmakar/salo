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
