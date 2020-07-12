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


#include "../headers/log.h"
#include "headers/history.h"
#include "headers/parse_tools.h"

namespace Convspace
{


History_organizer::History_organizer
(
	Config *transit_config,
	Community_organizer *transit_source_info,
	Message_storage *transit_message,
	Database_manager *transit_dbm,
	Config_dyn *transit_dyn
)
{
	logs -> CommitConveer( F, L, "Object initialization - History_organizer" );
	
	config = transit_config;
	source_info = transit_source_info;
	message = transit_message;
	dbm = transit_dbm;
	dyn = transit_dyn;
	open_session();
}

void History_organizer::add_phrase( const std::string& transit_input ) { input = transit_input; }
void History_organizer::add_answer( const std::string &transit_output ) { output = transit_output; }
void History_organizer::add_theme( const std::string &transit_theme ) { theme = transit_theme; }
void History_organizer::add_unsolved( const std::string &transit_unsolved ) { unsolved = transit_unsolved; }
void History_organizer::add_message_form( const std::string &transit_message_form ) { message_form = transit_message_form; }

void History_organizer::open_session( void )
{
	input.clear();
	output.clear();
	theme.clear();
	unsolved.clear();
	message_form.clear();
}

void History_organizer::close_session( void )
{
	logs -> CommitConveer( F, L, "SQL QUERY - updating history" );
	
	// Добавление нового пользователя происходит в начале конвеера.
	// Подразумевается, что функция add_new_user() уже была вызвана.
	
	if (config->get_binary("available/history"))
	{
		// Сохраняем сообщение в истории
		dbm -> DB_simple_query( "INSERT INTO `{@database_name@}`.`history` ( `id`, `phrase`, `message_form`, `answer`, `time` ) VALUES ( '" + screen_sql( source_info -> get_unique_id() ) + "', '" + screen_sql( input ) + "', '" + screen_sql( message -> message_form ) + "', '" + screen_sql( output ) + "', NOW() ) ;" );
		// Очистка лишней истории
		if( config -> get_binary( L"available_clear_history" ) ) dbm -> DB_simple_query( "DELETE FROM `{@database_name@}`.`history` WHERE `time` < DATE_SUB( NOW(), INTERVAL " + wide_to_thin_line( config -> get_keyword( L"clear_history_interval" ) ) + " ) ;" );
	}
	// Текущая тема разговора
	if( ! theme.empty() ) dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`" + ( source_info -> user_is_stable() ? std::string( "user" ) : std::string( "web_user" ) ) + "` SET `cur_theme` = '" + screen_sql( theme ) + "' WHERE `id` = '"  + screen_sql( source_info -> get_unique_id() ) + "' ;" );
	// Увеличить счётчик сообщений
	dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`" + ( source_info -> user_is_stable() ? std::string( "user" ) : std::string( "web_user" ) ) + "` SET `messages_sent` = `messages_sent` + 1 WHERE `id` = '"  + screen_sql( source_info -> get_unique_id() ) + "' ;" );
	if( source_info -> get_interface() == "SaloAPI" ) dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`saloapi_user` SET `messages_sent` = `messages_sent` + 1 WHERE `id` = '"  + screen_sql( source_info -> get_unique_id() ) + "' ;" );
	source_info->userconfig->increment_mes();
	write_down_max_messages();
	// Обновить время последней активности в конфиге
	source_info->userconfig->write_last_activity();
	// Обновить время последней активности в базе
	dbm->DB_simple_query("UPDATE `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` SET `active_last` = NOW() WHERE `id` = '" + screen_sql(source_info->get_unique_id()) + "';");
	// Записать конфиг в базу
	dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`" + ( source_info -> user_is_stable() ? std::string( "user" ) : std::string( "web_user" ) ) + "` SET `config` = '" + screen_sql( source_info -> userconfig->show() ) + "' WHERE `id` = '"  + screen_sql( source_info -> get_unique_id() ) + "' ;" );
	if( source_info -> get_interface() == "SaloAPI" ) dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`saloapi_user` SET `config` = '" + screen_sql( source_info -> userconfig->show() ) + "' WHERE `id` = '"  + screen_sql( source_info -> get_unique_id() ) + "' ;" );
	
	// Correction
	dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`history` SET `answer` = '{@empty@}' WHERE `answer` = '' LIMIT 1 ;" );
	
	
	open_session();
}

// Добавить нового пользователя, если это необходимо
void History_organizer::add_new_user( void )
{
	// Поиск пользователя с таким id
	// Если такового нет, то добавляем нового
	
	bool user_exists = dbm->return_query_count("SELECT count(*) FROM `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` WHERE `id` = '" + screen_sql(source_info->get_unique_id()) + "';") > 0;
	
	// Если бот сработал на пользователя без обращения и это первое сообщение, то не записывать его в список пользователей
	// Нельзя начать с сообщения типа "доброе утро"
	if (message->message_fetched_without_bot_name && !user_exists) return;
	
	if(!user_exists)
	{
		logs -> CommitConveer( F, L, "SQL QUERY - adding user" );
		dbm -> DB_simple_query( "INSERT INTO `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` (`id`, `first`, `last`, `name`, `messages_sent`, `cur_theme`, `attitude`, `status`) VALUES ( '" + screen_sql( source_info -> get_unique_id() ) + "', '" + screen_sql( source_info -> get_user_first() ) + "', '" + screen_sql( source_info -> get_user_last() ) + "', '" + screen_sql( source_info -> get_user_first() ) + "', 0, 'NOT SAVED', 0, 2 ) ;" );
		
		if ( source_info -> get_interface() == "SaloAPI" )
		{
			dbm -> DB_simple_query( "INSERT INTO `{@database_name@}`.`saloapi_user` (`id`, `first`, `last`, `name`, `messages_sent`, `cur_theme`, `attitude`, `status`) VALUES ( '" + screen_sql( source_info -> get_user_id() ) + "', '" + screen_sql( source_info -> get_user_first() ) + "', '" + screen_sql( source_info -> get_user_last() ) + "', '" + ( ( source_info -> get_user_first() == "" ) ? screen_sql( source_info -> get_saloapi_login() ) : screen_sql( source_info -> get_user_first() ) ) + "', 0, 'NOT SAVED', 0, 2 ) ;" );
		}
	}
}

std::string History_organizer::get_theme( void )
{
	std::string answer;
	
	logs -> CommitConveer( F, L, "SQL QUERY - getting previous theme" );
	
	std::string query_line = dbm -> return_query_one( "SELECT `cur_theme` FROM `{@database_name@}`.`" + ( source_info -> user_is_stable() ? std::string( "user" ) : std::string( "web_user" ) ) + "` WHERE `cur_theme` != 'NOT SAVED' AND `id` = '" + screen_sql ( source_info -> get_unique_id() ) + "' LIMIT 1 ;" );
	if( ! query_line.empty() )
	{
		logs -> CommitConveer( F, L, "Using theme \"" + query_line + "\"" );
		answer = dbm -> return_query_random( "SELECT `answer` FROM `{@database_name@}`.`typical_questions` WHERE `keyphrase` LIKE '%" + screen_sql( query_line ) + "%' AND (`bot` = '' OR `bot` = '" + source_info->get_bot_name() + "') LIMIT " + std::to_string( config -> get_coefficient ( L"query_limit_small" ) ) + " ;" );
		if( answer.empty() )
		{
			logs -> CommitConveer( F, L, "Error - saved theme is incorrect!( no answers found )" );
			answer = config -> get_keyword( "error_incorrect_theme" );
		}
	}
	else
	{
		// Если не нашлось, то ответ - отговорка
		logs -> CommitConveer( F, L, "Current theme is not saved yet :c " );
		answer = dbm -> answer_not_found();
		if( answer.empty() ) answer = config -> get_keyword( "empty_db_empty" );
	}
	
	return answer;
}

bool History_organizer::repeat_control( const std::string &current_line, const std::string &column )
{
	// Поиск подобных сообщений у данного пользователя
	int repeat_control_result = dbm -> return_query_count( "SELECT count(*) FROM `{@database_name@}`.`history` WHERE `" + screen_sql( column ) + "` LIKE '" + screen_sql( current_line ) + "' AND `id` = '" + screen_sql( source_info -> get_unique_id() ) + "' AND `" + screen_sql( column ) + "` <> '' AND `time` > DATE_SUB( NOW(), INTERVAL " + wide_to_thin_line( config -> get_keyword( L"repeat_control_interval" ) ) + " );" );
	logs -> CommitConveer( F, L, "Repeat control [" + column + "] returned " + std::to_string( repeat_control_result ) );
	return ( repeat_control_result > 0 );
}

void History_organizer::write_down_max_messages(void)
{
	// Записываю рекорд количества сообщений, если он есть
	const int current_n_messages = source_info->userconfig->there_is_element("n_messages") ?
		source_info->userconfig->get_coefficient("n_messages") : 0;
		
	if (
		!dyn->there_is_element("high_scores/max_n_messages/n_messages") ||
		current_n_messages > dyn->get_coefficient("high_scores/max_n_messages/n_messages")
	)
	{
		dyn->editable_subtree()["high_scores"]["max_n_messages"]["n_messages"] = std::to_string(current_n_messages);
		dyn->editable_subtree()["high_scores"]["max_n_messages"]["id"] = source_info->get_unique_id();
		dyn->editable_subtree()["high_scores"]["max_n_messages"]["name"] = source_info->get_user_first() + " " + source_info->get_user_last();
	}
}

} // namespace Convspace
