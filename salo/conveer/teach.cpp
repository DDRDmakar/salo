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

// Обучение возможно только СТАБИЛЬНЫМ пользователем

#include <string>
#include <vector>

#include "headers/teach.h"
#include "headers/parse_tools.h"
#include "../headers/log.h"

using namespace std;

namespace Convspace
{


// Конструктор
Teach_organizer::Teach_organizer
(
	Config *transit_config,
	Community_organizer *transit_source_info,
	Database_manager *transit_dbm,
	Config_dyn *transit_dyn
)
: Dynamic_organizer( transit_config, transit_source_info, transit_dyn )
{
	logs -> CommitConveer( F, L, "Object initialization - Teach_organizer" );
	dbm = transit_dbm;
}


void Teach_organizer::clr_unsolved( void ) // Удаляет записи о текущих сессиях обучения из БД
{
	logs -> CommitConveer( F, L, "SQL QUERY - clear unsolved messages" );
	dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`unsolved` SET `redactor` = '' WHERE `redactor` <> '' AND `answer` = '' ;" );
}

void Teach_organizer::stop_teachmode( void ) // Удаляет записи о текущих сессиях обучения из БД
{
	logs -> CommitConveer( F, L, "SQL QUERY - stop teaching mode" );
	dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`unsolved` SET `redactor` = '' WHERE `answer` = '' AND `redactor` = '" + screen_sql( source_info -> get_unique_id() ) + "' ;" );
}

void Teach_organizer::approve_all( void ) // Одобрить все записи с обучения
{
	logs -> CommitConveer( F, L, "SQL QUERY - approve all new phrases" );
	dbm -> DB_simple_query( "INSERT INTO `{@database_name@}`.`typical_questions` ( `keyphrase`, `answer`, `bot` ) SELECT `phrase`, `answer`, '' FROM `{@database_name@}`.`unsolved` WHERE `redactor` <> '' AND `answer` <> '' ;" );
	dbm -> DB_simple_query( "DELETE FROM `{@database_name@}`.`unsolved` WHERE `redactor` <> '' OR `phrase` = '' ; " );
	dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`typical_questions` SET `answer` = '{@empty@}' WHERE `answer` = '' ; " );
}

void Teach_organizer::approve_serial( void ) // Одобрять записи с обучения по одной
{
	logs -> CommitConveer( F, L, "SQL QUERY - approve new learned phrases" );
	
	std::string answer;
	
	// Пока сообщение не fin
	while( answer != config -> get_keyword( "approve_fin" ) )
	{
		// Достаём фразу
		std::string ph = dbm -> return_query_one( "SELECT `phrase` FROM `{@database_name@}`.`unsolved` WHERE `redactor` <> '' LIMIT 1 ;" );
		if( ph.empty() )
		{
			std::cout << wide_to_thin_line( config -> get_keyword( L"teachmode/empty_teachmode" ) ) << std::endl;
			break;
		}
		else
		{
			// Ответ на фразу
			std::string an = dbm -> return_query_one(  "SELECT `answer` FROM `{@database_name@}`.`unsolved` WHERE `redactor` <> '' LIMIT 1 ;" );
			
			std::cout << "\"" << ph << "\" - \"" << an << "\"	";
			cin >> answer;
			
			// Проверка на некорректный ввод
			if( answer != "0" && answer != "1" && answer != wide_to_thin_line( config -> get_keyword( L"approve_fin" ) ) )
			{
				std::cout << wide_to_thin_line( config -> get_keyword( L"error_incorrect_input" ) ) << std::endl;
				continue;
			}
			else
			{
				// запись ответа в базу
				if( answer == "1" ) dbm -> DB_simple_query( "INSERT INTO `{@database_name@}`.`typical_questions` ( `keyphrase`, `answer`, `bot` ) SELECT `phrase`, `answer`, '' FROM `{@database_name@}`.`unsolved` WHERE `phrase` = '" + ph + "' AND `answer` = '" + an + "' LIMIT 1 ;"  );
				dbm -> DB_simple_query( "DELETE FROM `{@database_name@}`.`unsolved` WHERE `phrase` = '" + screen_sql( ph ) + "' AND `answer` = '" + screen_sql( an ) + "' LIMIT 1 ;" );
			}
		}
	}
	// Исправление пустых ответов
	dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`typical_questions` SET `answer` = '{@empty@}' WHERE `answer` = '' ; " );
}

void Teach_organizer::delete_unsolved( void ) // Очистить всю таблицу с обучения
{
	logs -> CommitConveer( F, L, "SQL QUERY - remove all unsolved phrases" );
	dbm -> DB_simple_query( "DELETE FROM `{@database_name@}`.`unsolved` WHERE 1 ; " );
}

void Teach_organizer::remove_from_list_2( std::string id )
{
	if( id.empty() ) id = source_info -> get_unique_id();
	for( int i = 0; i < user_info_storage.size(); ++ i ) if( id == std::get <0> ( user_info_storage[i] ) ) user_info_storage.erase( user_info_storage.begin() + i );
}

void Teach_organizer::remove_old( void )
{
	for( auto i : user_info_storage ) if( ! is_participant( std::get <0> ( i ) ) ) remove_from_list_2( std::get <0> ( i ) );
}

// Получить номер пользователя в контейнере
int Teach_organizer::get_current_user_index( std::string id )
{
	if( id.empty() ) id = source_info -> get_unique_id();
	for( int i = 0; i < user_info_storage.size(); ++ i ) if( id == std::get <0> ( user_info_storage[i] ) ) return i;
	return -1;
}

// Функция, обучающая бота
std::string Teach_organizer::teach( const std::wstring &transit_phrase )
{
	logs -> CommitConveer( F, L, "Teachmode main function" );
	
	add_basic_data( transit_phrase );
	replace_in_text( phrase, L"\\n", L"\n" );
	
	try
	{
		
		// Проверка на пустую фразу
		if ( transit_phrase.empty() ) return config->get_keyword( "teachmode/error/nophrase" );
		// Проверка на содержание ссылки
		if (std::regex_search(to_single_line(to_lowercase(phrase)), config -> get_regex(L"regex/part/forbidden_answer"))) return config->get_keyword("error/forbidden_answer") + "\n";
		// Проверка на всякие слова нехорошие
		if (
			std::regex_match(to_single_line(to_lowercase(phrase)), config->get_regex(L"regex/full/forbidden_teachmode_phrases")) &&
			!std::regex_match(to_single_line(to_lowercase(phrase)), config->get_regex(L"detector/teach_del"))
		) return config->get_keyword("teachmode/error/forbidden_phrase") + "\n";
		
		// Увеличиваю на 1 счётчик сообщений обучения
		source_info->userconfig->increment_teachmes();
		// Записываю рекорд обучения, если он есть
		const int current_n_teachmessages = source_info->userconfig->there_is_element("n_teachmessages") ?
			source_info->userconfig->get_coefficient("n_teachmessages") : 0;
			
		if (
			!dyn->there_is_element("high_scores/teachmode/max_teachmessages/n_messages") ||
			current_n_teachmessages > dyn->get_coefficient("high_scores/teachmode/max_teachmessages/n_messages")
		)
		{
			dyn->editable_subtree()["high_scores"]["teachmode"]["max_teachmessages"]["n_messages"] = std::to_string(current_n_teachmessages);
			dyn->editable_subtree()["high_scores"]["teachmode"]["max_teachmessages"]["id"] = source_info->get_unique_id();
			dyn->editable_subtree()["high_scores"]["teachmode"]["max_teachmessages"]["name"] = source_info->get_user_first() + " " + source_info->get_user_last();
		}
		
		// Достаю ID текущей записи в таблице обучения, которая соответствует этому юзеру.
		const int current_user_index = get_current_user_index();
		if( current_user_index == -1 ) return config -> get_keyword( "teachmode/error/not_participant" );
		std::string current_problem_id = std::get <1> ( user_info_storage[ current_user_index ] );
		
		std::string answer; // Ответ пользователю
		
		if (std::regex_match(to_single_line(to_lowercase(phrase)), config->get_regex(L"detector/teach_del"))) // Если del, то пропускает сообщение
		{
			logs -> CommitConveer( F, L, "SQL QUERY - deleting unsolved message" );
			dbm -> DB_simple_query( "DELETE FROM `{@database_name@}`.`unsolved` WHERE `id` = '" + screen_sql( current_problem_id ) + "' LIMIT 1 ;" );
			answer = config -> get_keyword( "teachmode/mes_deleted" ) + "\n";
		}
		else // Сохраняет сообщение в БД
		{
			logs -> CommitConveer( F, L, "SQL QUERY - saving answer for unsolved" );
			
			dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`unsolved` SET `answer` = '" + screen_sql( phrase ) + "' WHERE `id` = '" + screen_sql( current_problem_id ) + "' LIMIT 1 ;" );
			if( config -> get_binary( L"available_teachmode_auto_approve" ) )
			{
				dbm -> DB_simple_query( "INSERT INTO `{@database_name@}`.`typical_questions` ( `keyphrase`, `answer`, `bot` ) SELECT `phrase`, `answer`, '" + std::string(config->get_binary("teachmode/add_answers_as_default") ? "" : source_info->get_bot_name()) + "' FROM `{@database_name@}`.`unsolved` WHERE `id` = '" + screen_sql( current_problem_id ) + "' LIMIT 1 ;"  );
				dbm -> DB_simple_query( "DELETE FROM `{@database_name@}`.`unsolved` WHERE `id` = '" + screen_sql( current_problem_id ) + "' LIMIT 1 ;" );
			}
			answer = config -> get_keyword( "teachmode/mes_saved" ) + "\n";
		}
		
		// Получаю id новой нераспознанной фразы
		current_problem_id = dbm -> return_query_one( "SELECT `id` FROM `{@database_name@}`.`unsolved` WHERE `phrase` <> '' AND `answer` = '' AND `redactor` = '' LIMIT 1 ;" );
		if( current_problem_id.empty() )
		{
			clr_list();
			answer += config -> get_keyword( "teachmode/empty_unsolved" );
		}
		else
		{
			// Запись нового ID нераспознанной фразы в контейнер
			std::get <1> ( user_info_storage[ current_user_index ] ) = current_problem_id;
			// Запись ID учителя в базу и вывод ответа пользователю
			dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`unsolved` SET `redactor` = '" + screen_sql( source_info -> get_unique_id() ) + "' WHERE `id` = '" + screen_sql( current_problem_id ) + "' LIMIT 1 ;" );
			answer += dbm -> return_query_one( "SELECT `phrase` FROM `{@database_name@}`.`unsolved` WHERE `id` = '" + screen_sql( current_problem_id ) + "' LIMIT 1 ;" );
		}
		return answer;
	}
	catch (std::regex_error)
	{
		error_out("Teachmode regex error!");
		return config->get_keyword("error/regex");
	}
}

std::string Teach_organizer::start( void ) // выдаёт первый запрос к учителю при старте
{
	logs -> CommitConveer( F, L, "SQL QUERY - start teachmode" );
	
	add_to_list();
	remove_old(); // Удаление старых записей
	// Получаю id нераспознанной фразы
	std::string current_problem_id = dbm -> return_query_one( "SELECT `id` FROM `{@database_name@}`.`unsolved` WHERE `phrase` <> '' AND `answer` = '' AND `redactor` = '' LIMIT 1 ;" );
	if( current_problem_id.empty() )
	{
		clr_list();
		return config -> get_keyword( "teachmode/empty_unsolved" );
	}
	else
	{
		// Добавляю пользователя в список учителей
		add_to_list();
		user_info_storage.push_back( std::make_tuple( source_info -> get_unique_id(), current_problem_id ) );
		
		// Запись ID учителя в базу и вывод ответа пользователю
		dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`unsolved` SET `redactor` = '" + screen_sql( source_info -> get_unique_id() ) + "' WHERE `id` = '" + screen_sql( current_problem_id ) + "' LIMIT 1 ;" );
		return config->get_keyword("media_text/help/" + std::string(config->get_binary("available_teachmode_auto_approve") ? "teacher_teachable" : "teacher")) + "\n" + dbm -> return_query_one( "SELECT `phrase` FROM `{@database_name@}`.`unsolved` WHERE `id` = '" + screen_sql( current_problem_id ) + "' LIMIT 1 ;" );
	}
}


} // namespace Convspace
