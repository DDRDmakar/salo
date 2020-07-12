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

#include "headers/database.h"
#include "headers/misc.h"
#include "headers/statistics.h"
#include "headers/log.h"
#include "headers/salostring.h"
#include <mutex>

std::mutex MYSQL_MUTEX;


//constructor
dbController::dbController(SecurityGeneric* securityProvider)
{
	MYSQL_MUTEX.lock();
	if(securityProvider == NULL)
	{
		std::cout << "Account Security Error - database needs Security Provider to connect!!!" << std::endl;
		exit(0);
	}
	else
	{
		secProvider = securityProvider;
	}

	databaseFingerprint = "null";
	savingRequest = true;
	mysql_desc = mysql_init(NULL);
	MYSQL_MUTEX.unlock();
	Auth();
}


//destructor
dbController::~dbController()
{
	MYSQL_MUTEX.lock();
	Disconnect();
	MYSQL_MUTEX.unlock();
}


/*

Аутентификация пользователя

*/

void dbController::Auth()
{
	std::string db_user;
	std::string db_password;
	std::string tmp_port; //Temproary string to avoid console-looping bug
	int retry_counter = 0;
	bool auto_login = false;

	loop:

	retry_counter++;
	if(retry_counter == 5)
	{
		std::cout << "Not today." << std::endl;
		exit(0);
	}

	DatabaseAccountData dat = secProvider->LoadDB();

	std::cout << std::endl << "+===========[ mysql connect ]===========+" << std::endl;
	if(dat.isEmpty || retry_counter > 1)
	{
		auto_login = false;
		std::cout << "Server IP: ";
		std::cin >> db_address;
		std::cout << "Enter valid TCP port to connect online server, or enter 0 to connect offline via UNIX socket\nServer Port: ";
		std::cin >> tmp_port;
		std::cout << "Username: ";
		std::cin >> db_user;
		std::cout << "Password: ";
		db_password = miscInputPassword();
	}
	else
	{
		auto_login = true;
		std::cout << "\nFound saved auth data, logging in automatically." << std::endl;
		db_address = dat.DatabaseAddress;
		tmp_port = dat.DatabasePort;
		db_user = dat.DatabaseUser;
		db_password = dat.DatabasePassword;
	}
	std::cout << std::endl << "+=======================================+" << std::endl << std::endl;


	db_port = std::stoi(tmp_port);

	if(Connect(db_user, db_password) != 0)
	{
		//Reinit broken mysql descriptor
		if(mysql_desc == NULL)
		{
			mysql_desc = mysql_init(NULL);
		}

		std::cout << "\n\nMaybe, try again?\n";

		goto loop; //yeah, im so evil...
	}

	//на самом деле мой код плохо комментирован :(

	if(!auto_login)
	{
		std::string ask;
		
		loop2: 
		std::cout << "\nSave your database login data, to login automatically? [y/n]: ";
		std::cin >> ask;

		if(ask != "y" && ask != "n" && ask != "Y" && ask != "N")
		{
			std::cout << "Use only y/n please!" << std::endl;
			goto loop2;
		}

		if(ask == "y" || ask == "Y")
		{
			savingRequest = true;
			std::cout << "\nYou can delete your saved data by deleting generic account file anytime.\n" << std::endl;

			SaveAccount(db_user, db_password);

			std::cout << "Saving login data...." << std::endl; 
		}
		else
		{
			std::cout << "Login data is not saved" << std::endl; 
			savingRequest = false;
		}
	}
	else
	{
		std::cout << "\nYou can delete your saved data by deleting generic account file anytime.\n" << std::endl;
	}

	
	//generate fingerprint
	std::string dt = db_address+";"+tmp_port+";"+db_user+";"+db_password; 
	databaseFingerprint = miscExecSystem(std::string("../libcryptm/netcryptm --encrypt \"" + dt + "\"").c_str());
}


/*

Сохранение на диск

*/

void dbController::SaveAccount(const std::string& login, const std::string& passwd)
{
	DatabaseAccountData dt;

	dt.DatabaseAddress = db_address;
	dt.DatabasePort = std::to_string(db_port);
	dt.DatabaseUser = login;
	dt.DatabasePassword = passwd;

	secProvider->SaveDB(dt);
}


/*

Подключение к базе MySQL

*/

int dbController::Connect(const std::string& LOGIN, const std::string& PASSWD)
{		
	MYSQL_MUTEX.lock();

	mysql_desc = mysql_real_connect(mysql_desc, db_address.c_str(), LOGIN.c_str(), PASSWD.c_str(), NULL, db_port, NULL, 0);

	if (mysql_desc == NULL)
	{
		std::cout << std::endl << "Authentication Error!" << std::endl;
		std::cout << "MYSQL sataus is NULL, possible reasons: wrong account data, no connection to db, mysql devkit not installed!" << std::endl;
		return -1;
	}
	else
	{
		std::cout << "CONNECTED TO DATABASE SUCCESSFULLY" << std::endl;
	}

	if(mysql_select_db(mysql_desc, "SALODATABASE") != 0)
	{
		std::cout << std::endl << mysql_error(mysql_desc) << std::endl << "SALO Server goes down due to database switching error" << std::endl;
		exit(0);
	}

	//mysql_set_server_option(mysql_desc, MYSQL_OPTION_MULTI_STATEMENTS_ON);

	std::cout << "MySQL/MariaDB: client ver. " << mysql_get_client_info() << ", server ver. " << mysql_get_server_info(mysql_desc) << std::endl;
	std::cout << "MySQL/MariaDB status: " << mysql_stat(mysql_desc) << std::endl;
	std::cout << "MYSQL/MariaDB connected to: " << mysql_get_host_info(mysql_desc) << std::endl;

	databaseServiceAlive = true;

	MYSQL_MUTEX.unlock();

	initial_utf8();

	return 0;
}


/*

Отключение от базы

*/

void dbController::Disconnect()
{
	std::cout << "Disconnecting form the database..." << std::endl;
	mysql_close(mysql_desc);  
	databaseServiceAlive = false;
}


/*

Устанавливает UTF-8 в качестве кодировки соединения с сервером

*/

void dbController::initial_utf8()
{
	database_simpleQuery("SET CHARSET utf8");
	database_simpleQuery("SET CHARACTER_SET_CLIENT=utf8");
	database_simpleQuery("SET CHARACTER_SET_RESULTS=utf8");
	database_simpleQuery("SET CHARACTER_SET_CONNECTION=utf8");
}


/*

Экранирует символы небезопасные для SQL. Выделяет память!

*/

std::string dbController::FilterString(const std::string& str)
{
	char* filtered_str = new char[str.length() * 2 + 1];
	mysql_real_escape_string(mysql_desc, filtered_str, str.c_str(), str.length());

	unsigned int l = strlen(filtered_str);
	
	char* final_str = new char[l + 1];
	strncpy(final_str, filtered_str, l);
	final_str[l] = 0;

	std::string result = std::string(final_str);

	delete[] filtered_str;
	delete[] final_str;

	for(int i = 0; i < result.length(); i++)
		{
			if(result[i] == '`') {  result[i] = ' '; }
		}

	return result;
}


/*

Функция, обеспечивающая запрос к базе, не требующий вывода результата

*/

bool dbController::database_simpleQuery(const std::string& SQL)
{
	MYSQL_MUTEX.lock();

	if(!databaseServiceAlive)
	{
		std::cout << "Error occured in database_simpleQuery" << std::endl << "Attempted to use dead connection!" << std::endl;
		logs->CommitGeneric(F, L, "Attempted to use dead connection!");
		MYSQL_MUTEX.unlock();
		return false;
	}

	statistics->IncrementInRAM(QUERYES_TOTAL, DATABASE);
	statistics->databaseTimings->StartCounting();

	std::string SQL_clipped = clipper(SQL);

	if(mysql_ping(mysql_desc) != 0)
	{
		std::cout << "Error occured in database_simpleQuery" << std::endl << "Connection to server lost..." << std::endl;
		logs->CommitGeneric(F, L, "Connection to server lost...");
		statistics->databaseTimings->EndCounting(); 
		statistics->IncrementInRAM(QUERY_TIME_AVG, DATABASE, statistics->databaseTimings->getPeriod()); 
		MYSQL_MUTEX.unlock();
		return false;
	}
	
	if(mysql_query(mysql_desc, SQL_clipped.c_str()) != 0)
	{
		std::string errtext = std::string(mysql_error(mysql_desc)); 

		std::cout << "Error occured in database_returnQuery:  "<< errtext << "\n\n" << "SQL was: " << SQL_clipped 
				<< "\n\nMySQL/MariaDB returned bad query result statuscode" << std::endl;
		logs->CommitGeneric(F, L, "MySQL/MariaDB returned bad query result statuscode");
		logs -> CommitConveer( F, L, " * MySQL/MariaDB returned bad query result statuscode" );
		statistics->databaseTimings->EndCounting(); 
		statistics->IncrementInRAM(QUERY_TIME_AVG, DATABASE, statistics->databaseTimings->getPeriod()); 
		MYSQL_MUTEX.unlock();
		return false;
	}
	else
	{
		statistics->databaseTimings->EndCounting(); 
		statistics->IncrementInRAM(QUERY_TIME_AVG, DATABASE, statistics->databaseTimings->getPeriod()); 
		MYSQL_MUTEX.unlock();
		return true;
	}
}


/*

Функция, обеспечивающая запрос к базе

*/

DatabaseResult dbController::database_returnQuery(const std::string& SQL)
{
	MYSQL_MUTEX.lock();

	DatabaseResult res;

	if(!databaseServiceAlive)
	{
		std::cout << "Error occured in database_returnQuery" << std::endl << "Attempted to use dead connection!" << std::endl;
		logs->CommitGeneric(F, L, "Attempted to use dead connection!");
		res.is_empty = true;
		MYSQL_MUTEX.unlock();
		return res;
	}

	statistics->IncrementInRAM(QUERYES_TOTAL, DATABASE);
	statistics->databaseTimings->StartCounting();

	std::string SQL_clipped = clipper(SQL);
	
	if(mysql_ping(mysql_desc) != 0)
	{
		std::cout << "Error occured in database_returnQuery" << std::endl << "Connection to server lost..." << std::endl;
		logs->CommitGeneric(F, L, "Connection to server lost...");
		res.is_empty = true;
		statistics->databaseTimings->EndCounting(); 
		statistics->IncrementInRAM(QUERY_TIME_AVG, DATABASE, statistics->databaseTimings->getPeriod()); 
		
		MYSQL_MUTEX.unlock();
		return res;
	}
	
	if(mysql_query(mysql_desc, SQL_clipped.c_str()) != 0)
	{
		std::string errtext = std::string(mysql_error(mysql_desc)); 

		std::cout << "Error occured in database_returnQuery:  "<< errtext << "\n\n" << "SQL was: " << SQL_clipped 
				<< "\n\nMySQL/MariaDB returned bad query result statuscode" << std::endl;
		logs->CommitGeneric(F, L, "MySQL/MariaDB returned bad query result statuscode");
		logs -> CommitConveer( F, L, "MySQL/MariaDB returned bad query result statuscode" );
		res.is_empty = true;
		statistics->databaseTimings->EndCounting(); 
		statistics->IncrementInRAM(QUERY_TIME_AVG, DATABASE, statistics->databaseTimings->getPeriod()); 
		MYSQL_MUTEX.unlock();
		return res;
	}
	else
	{
		MYSQL_RES* query_result = mysql_store_result(mysql_desc);
		
		if(query_result != NULL)
			{
				unsigned int stl = mysql_num_fields(query_result);
				unsigned int srl = mysql_num_rows(query_result);

				if(srl == 0)
				{
					//std::cout << "MySQL Query returned empty result (0 rows) OR/AND some errors occured" << std::endl;
					res.is_empty = true;
				}
				else
				{
					res.is_empty = false;
				}
				
				MYSQL_ROW row; 
				
				while(row = mysql_fetch_row(query_result))
				{
					std::string line;

					if(row != NULL)
					{
						for(int i = 0; i < stl; i++)
							{
								if(row[i] != NULL) { line.append(std::string(row[i])); }
								if(i != stl-1)
								{
									line.append(1, '/');
								}
							}
					}

					res.RowStrings.push_back(line);
				}

			res.size = res.RowStrings.size();
			res.size_bymysql = srl;

			}
			else
			{
				//std::cout << "MySQL Query returned empty result (0 rows) OR/AND some errors occured" << std::endl;
				res.is_empty = true;
			}

		mysql_free_result(query_result);
		statistics->databaseTimings->EndCounting(); 
		statistics->IncrementInRAM(QUERY_TIME_AVG, DATABASE, statistics->databaseTimings->getPeriod()); 
		MYSQL_MUTEX.unlock();
		return res;
	}

}


