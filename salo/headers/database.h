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