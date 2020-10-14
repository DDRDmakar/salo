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