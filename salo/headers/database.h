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

#ifndef sequence_check_database
#define sequence_check_database null

#include <iostream>
#include <mysql.h>
#include <string.h>
#include <vector>

#include "security.h"

struct DatabaseResult
{
	std::vector<std::string> RowStrings;
	bool is_empty;

	unsigned int size_bymysql;
	unsigned int size;
};




class dbController
{
private:
	MYSQL* mysql_desc;

	std::string db_address;
	int db_port;

	void Auth();
	void SaveAccount(const std::string& login, const std::string& passwd);
	int Connect(const std::string& LOGIN, const std::string& PASSWD);
	void Disconnect();
	void initial_utf8();

	SecurityGeneric* secProvider;
	std::string databaseFingerprint;
	bool savingRequest;

	bool databaseServiceAlive = false;
public:

	dbController(SecurityGeneric* securityProvider);
	~dbController();

	std::string getFingerprint() { return databaseFingerprint; };

	std::string FilterString(const std::string& str);
	
	bool           database_simpleQuery(const std::string& SQL);
	DatabaseResult database_returnQuery(const std::string& SQL);
};


extern dbController* database;

#endif