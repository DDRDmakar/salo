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


#ifndef __SEQUENCE_CHECK_CONVEER__
#define __SEQUENCE_CHECK_CONVEER__

#include <mutex>
#include <string>
#include <vector>

#include "config.h"
#include "config_dyn.h"
#include "message_storage.h"
#include "community.h"
#include "database_manager.h"
#include "answer_generator.h"
#include "history.h"
#include "ban.h"
#include "teach.h"
#include "information.h"

namespace Convspace
{

class Conveer
{
	
private:
	
	static std::mutex conveer_mutex;
	static unsigned int transit_id; // Генератор уникального id
	static unsigned int count; // Количество конвееров
	int conveer_id; // Уникальный id
	std::string config_name; // Имя конфига
	
	bool first_run; // Первый прогон конвеера
	int repeats; // Количество повторов
	int total_count; // Общее число сообщений
	
	Config config;
	Message_storage message;
	Community_organizer source_info;
	Database_manager dbm;
	Answer_generator generator;
	History_organizer historian;
	Ban_organizer security;
	Teach_organizer sensei;
	Config_dyn dyn_config;
	
	std::vector <std::string> fetcher_keyword_storage; // Список слов, на которые реагирует бот
	int fetcher_keyword_storage_version;
	
	// Answer generating function
	Answerbox internal_answer( void );
	std::wstring replace_typical_sequence( std::wstring &target );
	std::wstring replace_typical_sequence( std::string &target );
	
public:
	
	Config *workdat;
	
	Conveer( const std::string &transit_config_name );
	~Conveer( void );
	
	// Gateway to external functions
	Answerbox generate_answer( const Person &user, const CONFIGURATION &bot_config, const std::string &mes );
	void put_data( const Person &user, const CONFIGURATION &bot_config/*, const std::string &mes = ""*/ );
	unsigned int get_count( void ); // Получить количество конвееров
	unsigned int get_id( void ); // Получить id конвеера
	std::string get_config_name( void ); // Получить имя конфига
	std::string get_database_name( void ); // Получить имя БД, используесой ботом
	std::vector <std::string> get_fetcher_keywords( void ); // Получить список слов, на которые реагирует бот
	void refresh_config( void ); // Обновить конфиг из файла

	Information information;
};

} // namespace Convspace

#endif
