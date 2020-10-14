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

#ifndef __SEQUENCE_CHECK_INFORMATION__
#define __SEQUENCE_CHECK_INFORMATION__

#include "config.h"
#include "community.h"
#include "database_manager.h"

using namespace std;

namespace Convspace
{

class Information
{
	
private:
	
	Config *config;
	Community_organizer *source_info;
	Database_manager *dbm;
	
	int total_count;
	std::vector <std::pair <std::string, int>> type_storage;
	
	int database_name_storage_version;
	std::vector <std::string> database_name_storage;
	std::vector <std::string> get_database_name_storage( void ); // Получить список имён всех БД
	
public:
	
	Information( Config *transit_config = NULL, Community_organizer *transit_source_info = NULL, Database_manager *transit_dbm = NULL );
	
	void increment( const std::string &current_type ); // Считает типы сообщений
	
	int typical_question_count( void ); // Размер главной базы
	int user_count( const std::string &interface = "" ); // Количество стабильных пользователей
	int unsolved_count( void ); // Количество нераспознанных фраз
	int unsolved_done_count( void ); // Количество нераспознанных фраз, готовых к одобрению
	int message_count( const std::string &interface = "" ); // Количество сообщений за всё время существования
	int stable_message_count( const std::string &interface = "" ); // Количество сообщений за всё время существования ( стабильные интерфейсы )
	int unstable_message_count( const std::string &interface = "" ); // Количество сообщений за всё время существования ( нестабильные интерфейсы )
	int history_size( const std::string &interface = "" ); // Размер истории
	int attachment_count( const std::string &type = "" ); // Количество медиа - файлов в базе
	int last_time_messages( const std::string &interval, const std::string &interface = "" ); // Количество сообщений за последний промежуток времени
	std::wstring attachment_list( void ); // Список всех имеющихся типов вложений и их количество
	int last_time_commands( const std::string &interval, const std::string &command = "", const std::string &interface = "" ); // Количество команд за последний промежуток времени
	std::wstring information_about_user( void ); // Информация о пользователе
	int banned_user_count( const std::string &interface = "" ); // Количество людей в бан-листе
	int message_type_count( const std::string &current_type ); // Количество сообщений определённого типа
	int get_total( void ); // Количество всех сообщений за сессию
	
	// Global functions
	int global_user_count( void ); // Количество стабильных пользователей
	int global_message_count( void ); // Количество сообщений за всё время существования
	int global_stable_message_count( void ); // Количество сообщений за всё время существования ( стабильные интерфейсы )
	int global_unstable_message_count( void ); // Количество сообщений за всё время существования ( нестабильные интерфейсы )
	int global_history_size( void ); // Размер истории
	int global_last_time_messages( const std::string &interval ); // Количество сообщений за последний промежуток времени
	int global_user_message_count( const std::string &interface, const std::string &id ); // Количество сообщений от пользователя по всем ботам
};

} // namespace Convspace

#endif
