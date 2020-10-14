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

#include "headers/security.h"
#include "headers/misc.h"
#include "headers/cryptm.h"
#include <iostream>

//constr
SecurityGeneric::SecurityGeneric(const std::string& pth)
{
	path = pth;
	std::cout << "Salo Accout Security initialized [generic]" << std::endl; 
}

//destr
SecurityGeneric::~SecurityGeneric()
{
	std::cout << "Salo Accout Security stopped [generic]" << std::endl; 
}


/*

Читает из файла данные аккаунта

*/

DatabaseAccountData SecurityGeneric::LoadDB()
{
	DatabaseAccountData res;
	char* Server, *Port, *User, *Pass;

	if(!miscFileExists(path.c_str()))
	{
		res.isEmpty = true;
		return res;
	}

	std::string src = miscGetFile(path.c_str(), 1);
		
	char* csrc = cryptmdec(src.c_str()); 	
	src = std::string(csrc);
	delete[] csrc;

	try 
	{
		Server = miscFindAndCopy(src.c_str(), "srv='", "';");
		if(Server == NULL) throw 1;
		res.DatabaseAddress = std::string(Server);

		Port = miscFindAndCopy(src.c_str(), "prt='", "';");
		if(Port == NULL) throw 1;
		res.DatabasePort = std::string(Port);

		User = miscFindAndCopy(src.c_str(), "usr='", "';");
		if(User == NULL) throw 1;
		res.DatabaseUser = std::string(User);

		Pass = miscFindAndCopy(src.c_str(), "pwd='", "';");
		if(Pass == NULL) throw 1;
		res.DatabasePassword = std::string(Pass);

		res.isEmpty = false;
	} 
	catch (int e) { res.isEmpty = true; }	

	if(Pass != NULL)
	{
		char* cpas = cryptmdec(res.DatabasePassword.c_str());
		res.DatabasePassword = std::string(cpas);
		delete[] cpas;
		delete[] Pass;
	}

	if(Server != NULL) delete[] Server;
	if(Port != NULL) delete[] Port;
	if(User != NULL) delete[] User;

	return res;
}



/*

Сохраняет в файл данные аккаунта

*/

void SecurityGeneric::SaveDB(const DatabaseAccountData& data)
{
	std::string sva = std::string();

	sva += "srv='"+data.DatabaseAddress+"';";
	sva += "prt='"+data.DatabasePort+"';";
	sva += "usr='"+data.DatabaseUser+"';";
	char* cpw = cryptmenc(data.DatabasePassword.c_str());
	sva += "pwd='"+ std::string(cpw) +"';";

	char* cal = cryptmenc(sva.c_str());
	sva = std::string(cal);

	delete[] cpw;
	delete[] cal;

	miscSetFile(path.c_str(), sva);
}


/*

Читает из файла данные аккаунта Вконтакте

*/

VKAccountData SecurityGeneric::LoadVK(const std::string& vkpath)
{
	VKAccountData res;
	char* User, *Pass;

	if(!miscFileExists(vkpath.c_str()))
	{
		res.isEmpty = true;
		return res;
	}

	std::string src = miscGetFile(vkpath.c_str(), 1);
		
	char* csrc = cryptmdec(src.c_str()); 	
	src = std::string(csrc);
	delete[] csrc;

	try 
	{
		User = miscFindAndCopy(src.c_str(), "log='", "';");
		if(User == NULL) throw 1;
		res.login = std::string(User);

		Pass = miscFindAndCopy(src.c_str(), "pwd='", "';");
		if(Pass == NULL) throw 1;
		res.passw = std::string(Pass);

		res.isEmpty = false;
	} 
	catch (int e) { res.isEmpty = true; }	

	if(Pass != NULL)
	{
		char* cpas = cryptmdec(res.passw.c_str());
		res.passw = std::string(cpas);
		delete[] cpas;
		delete[] Pass;
	}
	
	if(User != NULL) delete[] User;

	return res;
}


/*

Сохраняет в файл данные аккаунта Вконтакте

*/

void SecurityGeneric::SaveVK(const VKAccountData& data, const std::string& vkpath)
{
	std::string sva = std::string();

	sva += "log='"+data.login+"';";
	char* cpw = cryptmenc(data.passw.c_str());
	sva += "pwd='"+ std::string(cpw) +"';";

	char* cal = cryptmenc(sva.c_str());
	sva = std::string(cal);

	delete[] cpw;
	delete[] cal;

	miscSetFile(vkpath.c_str(), sva);
}


/*

Читает из файла данные аккаунта Телеграма

*/

TGAccountData SecurityGeneric::LoadTG(const std::string& tgpath)
{
	TGAccountData res;
	char* Pass;

	if(!miscFileExists(tgpath.c_str()))
	{
		res.isEmpty = true;
		return res;
	}

	std::string src = miscGetFile(tgpath.c_str(), 1);
		
	char* csrc = cryptmdec(src.c_str()); 	
	src = std::string(csrc);
	delete[] csrc;

	try 
	{
		Pass = miscFindAndCopy(src.c_str(), "tok='", "';");
		if(Pass == NULL) throw 1;
		res.token = std::string(Pass);

		res.isEmpty = false;
	} 
	catch (int e) { res.isEmpty = true; }	

	if(Pass != NULL)
	{
		char* cpas = cryptmdec(res.token.c_str());
		res.token = std::string(cpas);
		delete[] cpas;
		delete[] Pass;
	}
		
	return res;
}


/*

Сохраняет в файл данные аккаунта

*/

void SecurityGeneric::SaveTG(const TGAccountData& data, const std::string& tgpath)
{
	std::string sva = std::string();

	char* cpw = cryptmenc(data.token.c_str());
	sva += "tok='"+ std::string(cpw) +"';";

	char* cal = cryptmenc(sva.c_str());
	sva = std::string(cal);

	delete[] cpw;
	delete[] cal;

	miscSetFile(tgpath.c_str(), sva);
}