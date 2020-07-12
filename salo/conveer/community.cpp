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

#include <vector>
#include <string>
#include <time.h>
#include <functional>

#include "headers/database_manager.h"
#include "headers/community.h"
#include "headers/pattern_replacer.h"
#include "headers/parse_tools.h"

namespace Convspace
{

Community_organizer::Community_organizer( Config *transit_config, Database_manager *transit_dbm )
{
	logs -> CommitConveer( F, L, "Object initialization - Community_organizer" );
	
	user_status = 2;
	stable = false;
	
	config = transit_config;
	dbm = transit_dbm;
	
	if (config) bot_name = config->get_keyword("bot_name"); // Имя бота для БД и идентификации
	
	userconfig = nullptr;
}

void Community_organizer::add_basic_data( const Person &transit_user, const CONFIGURATION &transit_config )
{
	begin_add_basic_data:;
	
	logs -> CommitConveer( F, L, "Filling object Community_organizer" );
	
	user = transit_user;
	bot_config = transit_config;
	
	stable = 
	(
		bot_config.INTERFACE == "networking_vkcom" ||
		bot_config.INTERFACE == "networking_telegram" ||
		bot_config.INTERFACE == "SaloAPI" ||
		bot_config.INTERFACE == "networking_vkgroup"
	);
	
	unique_id = bot_name + "_" + bot_config.INTERFACE + "_" + user.IDENTIFICATOR; // Абсолютно уникальный идентификатор
	
	// Reading individual config from DB
	try
	{
		if ( true )
		{
			const std::string plain_individual_config_node = dbm -> return_query_one("SELECT `config` FROM `{@database_name@}`.`" + ( stable ? std::string( "user" ) : std::string( "web_user" ) ) + "` WHERE `id` = '" + screen_sql( unique_id ) + "' LIMIT 1");
			if ( userconfig ) delete userconfig;
			
			// If there is no config
			if ( plain_individual_config_node.empty() )
			{
				there_is_no_config:;
				
				userconfig = new Config_user(
					wide_to_thin_line(
						read_file(
							config -> get_keyword("individual_config_path")
						)
					)
				);
				userconfig->add_basic_data_user( unique_id, user.Name, user.Last );
				
				// Коррекция для записей в базе, добавленных до создания конфига
				if ( !user_is_new() )
				{
					userconfig->unsafe_tree()["name"] = dbm -> return_query_one( "SELECT `name` FROM " + std::string( user_is_stable() ? " `{@database_name@}`.`user` " : " `{@database_name@}`.`web_user` " ) + " WHERE `id` = '" + get_unique_id() + "' LIMIT 1 ;" );
					userconfig->unsafe_tree()["n_messages"] = dbm -> return_query_one( "SELECT `messages_sent` FROM " + std::string( user_is_stable() ? " `{@database_name@}`.`user` " : " `{@database_name@}`.`web_user` " ) + " WHERE `id` = '" + get_unique_id() + "' LIMIT 1 ;" );
					userconfig->unsafe_tree()["user_status"] = std::stoi( dbm -> return_query_one( "SELECT `status` FROM " + std::string( user_is_stable() ? " `{@database_name@}`.`user` " : " `{@database_name@}`.`web_user` " ) + " WHERE `id` = '" + get_unique_id() + "' LIMIT 1 ;" ) );
				}
			}
			// If there is config
			else
			{
				userconfig = new Config_user( plain_individual_config_node );
				
				// Коррекция "битых" записей в базе
				if (
					!userconfig->subtree()["id"] ||
					!userconfig->subtree()["first"] ||
					!userconfig->subtree()["last"] ||
					!userconfig->subtree()["name"] ||
					!userconfig->subtree()["n_messages"] ||
					!userconfig->subtree()["n_teachmessages"] ||
					!userconfig->subtree()["active_first"] ||
					!userconfig->subtree()["active_last"] ||
					!userconfig->subtree()["last_theme"] ||
					!userconfig->subtree()["user_status"] ||
					!userconfig->subtree()["leet_type"]
				)
				{
					error_out("Error found in individual user config from database, correcting.");
					goto there_is_no_config;
				}
			}
		}
	}
	catch (YAML::Exception e)
	{
		dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`" + ( user_is_stable() ? std::string( "user" ) : std::string( "web_user" ) ) + "` SET `config` = '' WHERE `id` = '"  + screen_sql( get_unique_id() ) + "' ;" );
		goto begin_add_basic_data;
	}
}

Person Community_organizer::get_user( void ) { return user; }
CONFIGURATION Community_organizer::get_configuration( void ) { return bot_config; }

int Community_organizer::get_user_status( const std::string &id )
{
	
//	0 = creator
//	1 = admin
//	2 = user
//	3 = superadmin
	
	logs -> CommitConveer( F, L, "Getting user status" );
	
	bool search_in_reserve = false;
	// If user is not present in users database
	// 
	if (!id.empty() && 0 == dbm->return_query_count("SELECT COUNT(*) FROM `{@database_name@}`.`" + std::string(user_is_stable() ? "user" : "web_user") + "` WHERE `id` = '" + screen_sql(id) + "' LIMIT 1"))
	{
		// If user is not present in reserve database
		if (0 == dbm->return_query_count("SELECT COUNT(*) FROM `{@database_name@}`.`" + std::string(user_is_stable() ? "user_reserve" : "web_user_reserve") + "` WHERE `id` = '" + screen_sql(id) + "' LIMIT 1")) return 2;
		else search_in_reserve = true;
	}
	int current_status = dbm -> return_query_count("SELECT `status` FROM `{@database_name@}`.`" + std::string(user_is_stable() ? "user" : "web_user") + std::string(search_in_reserve ? "_reserve" : "") + "` WHERE `id` = '" + screen_sql( id.empty() ? unique_id : id ) + "' LIMIT 1");
	return current_status != -1 ? current_status : 2;
}

// Первое ли это сообщение
bool Community_organizer::user_is_new( void )
{
	return dbm -> return_query_count( "SELECT count(*) FROM `{@database_name@}`.`" + ( user_is_stable() ? std::string( "user" ) : std::string( "web_user" ) ) + "` WHERE `id` = '" + get_unique_id() + "' ;" ) == 0;
}

bool Community_organizer::user_is_stable ( void ) { return stable; }
std::string Community_organizer::get_user_id ( void ) { return user.IDENTIFICATOR; }
std::string Community_organizer::get_unique_id( void ) { return unique_id; }
std::string Community_organizer::get_user_ip ( void ) { return bot_config.web_ip; }
std::string Community_organizer::get_user_first( void ) { return user.Name; }
std::string Community_organizer::get_user_last( void ) { return user.Last; }
std::string Community_organizer::get_interface( void ) { return bot_config.INTERFACE; }
std::string Community_organizer::get_userhash( void ) { return bot_config.web_userhash; }
std::string Community_organizer::get_attachment ( void ) { return bot_config.attachment == "null" ? "" : bot_config.attachment; }
std::string Community_organizer::get_bot_name( void ) { return bot_name; }
std::string Community_organizer::get_bot_name_and_interface( void ) { return bot_name + "_" + bot_config.INTERFACE; }
std::string Community_organizer::get_bot_interface_and_id( void ) { return bot_config.INTERFACE + "_" + user.IDENTIFICATOR; }
std::string Community_organizer::get_saloapi_login( void ) { return user.saloapi_login; }
void* Community_organizer::get_bot_pointer( void ) { return bot_config.BOT; }
std::string Community_organizer::get_interface_unique_identificator( void ) { return bot_config.INTERFACE_UNIQUE_IDENTIFICATOR; };
bool Community_organizer::is_conf(void) { return bot_config.isConf; }

size_t Community_organizer::gethash(std::string pat)
{
	Pattern_replacer storage(pat);
	
	std::time_t current_time = std::time(nullptr);
	struct tm *timestruct = localtime(&current_time);
	
	storage.replace("id", get_user_id());
	storage.replace("uid", get_unique_id());
	storage.replace("name", get_user_first() + " " + get_user_last());
	
	storage.replace("year", std::to_string(timestruct->tm_year));
	storage.replace("mon", std::to_string(timestruct->tm_mon));
	storage.replace("yday", std::to_string(timestruct->tm_yday));
	storage.replace("mday", std::to_string(timestruct->tm_mday));
	storage.replace("wday", std::to_string(timestruct->tm_wday));
	storage.replace("hour", std::to_string(timestruct->tm_hour));
	storage.replace("min", std::to_string(timestruct->tm_min));
	storage.replace("sec", std::to_string(timestruct->tm_sec));
	
	std::hash<std::string> hash_fn;
	
	return hash_fn(storage.extract());
}

} // namespace Convspace
