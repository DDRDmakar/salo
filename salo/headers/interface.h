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

#ifndef INTERF
#define INTERF

#include "statistics.h"

class InterfaceSettings
{
public:
	int official = 0;
	int anticaptchaEnabled = 0;
	std::string anticaptchaApiKey;
	int alertdevelopersEnabled = 0;
	std::string vkgroupId;
	int vkgroupMemberscheckDelay = 0;
	int ignoreFriendsReq = 0;
	int ignoreBirthdays = 0;
	int confchatsOnly = 0;
	std::string customGroupLink;
	int useCallbackApiConnector = 0;
	std::string personalPromotion;
};


class interface
{
public:
	int interface_id;
	int interface_type;
	bool* state;
	bool thisState = false;
	bool toBeDeleted = false;
	std::string INTERFACE_SIMPLE_NAME;
	std::string INTERFACE_IDENTIFICATOR;
	void* thisBot;
	interfaceStatistics* iStatistics;
	std::string getInfo(); 
	std::string SnapshotName;
	InterfaceSettings interfaceSettings;


	virtual void Tick() = 0; //Абстрактный метод
	
	//вирутальный деструктор, для нормального удаления потомка
	virtual ~interface() { delete iStatistics; }; 	 

};

#endif