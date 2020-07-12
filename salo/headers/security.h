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