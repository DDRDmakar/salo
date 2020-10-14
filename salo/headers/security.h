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

#include <string>

#ifndef SECGEN
#define SECGEN

struct DatabaseAccountData
{
	std::string DatabaseAddress;
	std::string DatabasePort;
	std::string DatabaseUser;
	std::string DatabasePassword;

	bool isEmpty = true;
};


struct VKAccountData
{
	std::string login;
	std::string passw;

	bool isEmpty = true;
};


struct TGAccountData
{
	std::string token;

	bool isEmpty = true;
};


class SecurityGeneric 
{
private:
	std::string path;

public:
	
	//Database
	void SaveDB(const DatabaseAccountData& data);
	DatabaseAccountData LoadDB();
	
	//vk
	void SaveVK(const VKAccountData& data, const std::string& vkpath);
	VKAccountData LoadVK(const std::string& vkpath);
	
	//telegram 
	void SaveTG(const TGAccountData& data, const std::string& tgpath);
	TGAccountData LoadTG(const std::string& tgpath);

	SecurityGeneric(const std::string& pth);
	~SecurityGeneric();
};

extern SecurityGeneric* sec;

#endif