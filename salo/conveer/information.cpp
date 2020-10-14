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

#include "headers/information.h"
#include "headers/database_manager.h"
#include "headers/parse_tools.h"
#include "headers/config.h"
#include "headers/community.h"

namespace Convspace
{


Information::Information( Config *transit_config, Community_organizer *transit_source_info, Database_manager *transit_dbm )
{
	logs -> CommitConveer( F, L, "Object initialization - Information" );
	
	config = transit_config;
	source_info = transit_source_info;
	dbm = transit_dbm;
	
	total_count = 0;
	database_name_storage_version = -1;
}

int Information::typical_question_count( void ) // Размер главной базы
{
	return dbm -> return_query_count( "SELECT COUNT(*) FROM `{@database_name@}`.`typical_questions` WHERE (`bot` = '' OR `bot` = '" + source_info->get_bot_name() + "') ;" );
}

int Information::user_count( const std::string &interface ) // Количество стабильных пользователей
{
	return dbm -> return_query_count( "SELECT COUNT(*) FROM `{@database_name@}`.`user` WHERE `id` LIKE '" + source_info -> get_bot_name() + "_" + screen_sql( interface ) + "%' ;" );
}

int Information::unsolved_count( void ) // Количество нераспознанных фраз
{
	return dbm -> return_query_count( "SELECT COUNT(*) FROM `{@database_name@}`.`unsolved`;" );
}

int Information::unsolved_done_count( void ) // Количество нераспознанных фраз, готовых к одобрению
{
	return dbm -> return_query_count( "SELECT COUNT(*) FROM `{@database_name@}`.`unsolved` WHERE `answer` <> '';" );
}

int Information::message_count( const std::string &interface ) // Количество сообщений за всё время существования
{
	return stable_message_count( interface ) + unstable_message_count( interface );
}

int Information::stable_message_count( const std::string &interface ) // Количество сообщений за всё время существования ( стабильные интерфейсы )
{
	return dbm -> return_query_count( "SELECT SUM(`messages_sent`) FROM `{@database_name@}`.`user` WHERE `id` LIKE '" + source_info -> get_bot_name() + "_" + screen_sql( interface ) + "%' ;" );
}

int Information::unstable_message_count( const std::string &interface ) // Количество сообщений за всё время существования ( нестабильные интерфейсы )
{
	return dbm -> return_query_count( "SELECT SUM(`messages_sent`) FROM `{@database_name@}`.`web_user` WHERE `id` LIKE '" + source_info -> get_bot_name() + "_" + screen_sql( interface ) + "%' ;" );
}

int Information::history_size( const std::string &interface ) // Размер истории
{
	return dbm -> return_query_count( "SELECT COUNT(*) FROM `{@database_name@}`.`history` WHERE `id` LIKE '" + source_info -> get_bot_name() + "_" + screen_sql( interface ) + "%';" );
}

int Information::attachment_count( const std::string &type ) // Количество медиа - файлов в базе
{
	if( type.empty() ) return dbm -> return_query_count( "SELECT COUNT(*) FROM `{@database_name@}`.`attachment` WHERE (`type` LIKE '" + source_info->get_interface() + "_%' OR `type` LIKE '" + source_info->get_bot_name_and_interface() + "_%') ;" );
	else return dbm -> return_query_count( "SELECT COUNT(*) FROM `{@database_name@}`.`attachment` WHERE `type` = '" + screen_sql( type ) + "' ;" );
}

// Количество сообщений за определённый период времени
int Information::last_time_messages( const std::string &interval, const std::string &interface ) // Количество сообщений за последний промежуток времени
{
	// type = SECOND / MINUTE / HOUR / DAY / WEEK / MONTH / YEAR
	if( ! std::regex_match( interval, std::regex( "\\d+ ((SECOND)|(second)|(MINUTE)|(minute)|(HOUR)|(hour)|(DAY)|(day)|(WEEK)|(week)|(MONTH)|(month)|(YEAR)|(year))" ) ) ) return -1;
	
	return dbm -> return_query_count( "SELECT COUNT(*) FROM `{@database_name@}`.`history` WHERE `id` LIKE '" + source_info -> get_bot_name() + "_" + screen_sql( interface ) + "%' AND `time` > DATE_SUB( NOW(), INTERVAL " + interval + " ) ;" );
}

std::wstring Information::attachment_list( void ) // Список всех имеющихся типов вложений и их количество
{
	std::wstring current_line;
	
	current_line += L"ATTACHMENTS:\n";
	std::vector <std::wstring> stat_line_storage = dbm -> DB_return_query( L"SELECT `type`, `type_sub`, count(*) FROM `{@database_name@}`.`attachment` GROUP BY `type`, `type_sub` ;" );
	for( auto i : stat_line_storage ) current_line += i + L"\n";
	
	current_line += L"TEXTS:\n";
	stat_line_storage = dbm -> DB_return_query( L"SELECT `type`, `type_sub`, count(*) FROM `{@database_name@}`.`story` GROUP BY `type`, `type_sub` ;" );
	for( auto i : stat_line_storage ) current_line += i + L"\n";
	
	replace_in_text( current_line, L"//", L"/[default]/" );
	replace_in_text( current_line, L"/", L" ——— " );
	
	return current_line;
}

// Количество команд
int Information::last_time_commands( const std::string &interval, const std::string &command, const std::string &interface )
{
	if
	(
		! std::regex_match( interval, std::regex( "\\d+ +((SECOND)|(second)|(MINUTE)|(minute)|(HOUR)|(hour)|(DAY)|(day)|(WEEK)|(week)|(MONTH)|(month)|(YEAR)|(year))" ) )
	) return -1;
	
	return dbm -> return_query_count( "SELECT count(*) FROM `{@database_name@}`.`history` WHERE `phrase` LIKE 'cmd " + command + "%' AND `id` LIKE '" + source_info -> get_bot_name() + "_" + screen_sql( interface ) + "%' AND `time` > DATE_SUB( NOW(), INTERVAL " + interval + " ); " );
}

// Инфа про пользователя из базы
std::wstring Information::information_about_user( void )
{
	std::wstring current_line;
	
	current_line += L"ID : " + thin_to_wide_line( dbm -> return_query_one( "SELECT `id` FROM " + std::string( source_info -> user_is_stable() ? " `{@database_name@}`.`user` " : " `{@database_name@}`.`web_user` " ) + " WHERE `id` = '" + source_info -> get_unique_id() + "' LIMIT 1 ;" ) ) + L"\n";
	current_line += L"First : " + thin_to_wide_line( dbm -> return_query_one( "SELECT `first` FROM " + std::string( source_info -> user_is_stable() ? " `{@database_name@}`.`user` " : " `{@database_name@}`.`web_user` " ) + " WHERE `id` = '" + source_info -> get_unique_id() + "' LIMIT 1 ;" ) ) + L"\n";
	current_line += L"Last : " + thin_to_wide_line( dbm -> return_query_one( "SELECT `last` FROM " + std::string( source_info -> user_is_stable() ? " `{@database_name@}`.`user` " : " `{@database_name@}`.`web_user` " ) + " WHERE `id` = '" + source_info -> get_unique_id() + "' LIMIT 1 ;" ) ) + L"\n";
	current_line += L"Name : " + thin_to_wide_line( dbm -> return_query_one( "SELECT `name` FROM " + std::string( source_info -> user_is_stable() ? " `{@database_name@}`.`user` " : " `{@database_name@}`.`web_user` " ) + " WHERE `id` = '" + source_info -> get_unique_id() + "' LIMIT 1 ;" ) ) + L"\n";
	current_line += L"Messages sent : " + thin_to_wide_line( dbm -> return_query_one( "SELECT `messages_sent` FROM " + std::string( source_info -> user_is_stable() ? " `{@database_name@}`.`user` " : " `{@database_name@}`.`web_user` " ) + " WHERE `id` = '" + source_info -> get_unique_id() + "' LIMIT 1 ;" ) ) + L"\n";
	int current_status = dbm -> return_query_count( "SELECT `status` FROM " + std::string( source_info -> user_is_stable() ? " `{@database_name@}`.`user` " : " `{@database_name@}`.`web_user` " ) + " WHERE `id` = '" + source_info -> get_unique_id() + "' LIMIT 1 ;" );
	current_line += L"Status : " + std::to_wstring( current_status ) + L" ";
	
	switch( current_status )
	{
		case 0: { current_line += L" " + config -> get_keyword( L"status_creator" ); break; }
		case 1: { current_line += L" " + config -> get_keyword( L"status_admin" ); break; }
		case 2: { current_line += L" " + config -> get_keyword( L"status_user" ); break; }
		default: { current_line += L" " + config -> get_keyword( L"status_unknown" ); break; }
	}
	
	return current_line;
}

// Количество людей в бан-листе
int Information::banned_user_count( const std::string &interface )
{
	return dbm -> return_query_count( "SELECT COUNT(*) FROM `{@database_name@}`.`ban_list` WHERE `id` LIKE '" + source_info -> get_bot_name() + "_" + screen_sql( interface ) + "%';" );
}

// Считает типы сообщений
void Information::increment( const std::string &current_type )
{
	++ total_count;
	for( auto i : type_storage ) if( current_type == std::get <0> ( i ) ) ++ ( std::get <1> ( i ) );
	type_storage.push_back( std::make_pair( current_type, 1 ) );
}

// Количество сообщений определённого типа
int Information::message_type_count( const std::string &current_type )
{
	for( auto i : type_storage ) if( current_type == std::get <0> ( i ) ) return ( std::get <1> ( i ) );
	return 0;
}

// Количество всех сообщений за сессию
int Information::get_total( void )
{
	return total_count;
}

// G L O B A L

std::vector <std::string> Information::get_database_name_storage( void )
{
	if( database_name_storage_version != config -> get_version() )
	{
		database_name_storage_version = config -> get_version();
		database_name_storage = config -> get_group( "all_databases" );
	}
	return database_name_storage;
}

int Information::global_user_count( void ) // Количество стабильных пользователей
{
	int answer = 0;
	std::vector <std::string> dns = get_database_name_storage();
	for( auto e : dns ) answer += dbm -> return_query_count( "SELECT COUNT(*) FROM `" + e + "`.`user`;" );
	return answer;
}

int Information::global_message_count( void ) // Количество сообщений за всё время существования
{
	return global_stable_message_count() + global_unstable_message_count();
}
int Information::global_stable_message_count( void ) // Количество сообщений за всё время существования ( стабильные интерфейсы )
{
	int answer = 0;
	std::vector <std::string> dns = get_database_name_storage();
	for( auto e : dns ) answer += dbm -> return_query_count( "SELECT SUM(`messages_sent`) FROM `" + e + "`.`user`;" );
	return answer;
}
int Information::global_unstable_message_count( void ) // Количество сообщений за всё время существования ( нестабильные интерфейсы )
{
	int answer = 0;
	std::vector <std::string> dns = get_database_name_storage();
	for( auto e : dns ) answer +=  dbm -> return_query_count( "SELECT SUM(`messages_sent`) FROM `" + e + "`.`web_user`;" );
	return answer;
}

int Information::global_history_size( void ) // Размер истории
{
	int answer = 0;
	std::vector <std::string> dns = get_database_name_storage();
	for( auto e : dns ) answer += dbm -> return_query_count( "SELECT COUNT(*) FROM `" + e + "`.`history`;" );
	return answer;
}

// Количество сообщений за определённый период времени
int Information::global_last_time_messages( const std::string &interval ) // Количество сообщений за последний промежуток времени
{
	// type = SECOND / MINUTE / HOUR / DAY / WEEK / MONTH / YEAR
	if( ! std::regex_match( interval, std::regex( "\\d+ +((SECOND)|(second)|(MINUTE)|(minute)|(HOUR)|(hour)|(DAY)|(day)|(WEEK)|(week)|(MONTH)|(month)|(YEAR)|(year))" ) ) ) return -1;
	
	int answer = 0;
	std::vector <std::string> dns = get_database_name_storage();
	for( auto e : dns ) answer += dbm -> return_query_count( "SELECT COUNT(*) FROM `" + e + "`.`history` WHERE `time` > DATE_SUB( NOW(), INTERVAL " + interval + " ) ;" );
	
	return answer;
}

int Information::global_user_message_count( const std::string &interface, const std::string &id ) // Количество сообщений от пользователя по всем ботам
{
	int answer = 0;
	std::vector <std::string> dns = get_database_name_storage();
	for( auto e : dns ) answer += dbm -> return_query_count( "SELECT SUM(`messages_sent`) FROM `" + e + "`.`user` WHERE `id` = '" + source_info -> get_bot_name() + "_" + screen_sql( interface ) + "_" + screen_sql( id ) + "' ;" );
	return answer;
}

} // namespace Convspace
