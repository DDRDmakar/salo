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

#include <yaml-cpp/yaml.h>

#include "headers/config_user.h"
#include "headers/parse_tools.h"
#include "headers/answer_generator.h"

namespace Convspace
{

Config_user::Config_user(const std::string &plain_node_text = "") : Config_yaml(plain_node_text) {}

void Config_user::add_basic_data_user(
	const std::string &id,
	const std::string &first_name,
	const std::string &last_name
)
{
	config_storage["id"] = id;
	config_storage["first"] = first_name;
	config_storage["last"] = last_name;
	config_storage["name"] = first_name;
	
	config_storage["active_first"] = get_datetime( posix_time );
	config_storage["active_last"]  = get_datetime( posix_time );
}


void Config_user::write_last_activity( void ) { unsafe_tree()["active_last"]  = get_datetime( posix_time ); }

bool Config_user::is_in_game( void ) { return get_coefficient( "gamemode" ); }

bool Config_user::is_teacher( void ) { return get_binary( "teachmode" ); }

void Config_user::increment_mes( void )
{
	try
	{
		unsafe_tree()["n_messages"] = std::to_string( std::stoi( get_keyword("n_messages") ) + 1 );
	}
	catch ( std::invalid_argument e ) { error_out( "Error while parsing n_messages from user config: " + std::string( e.what() ) ); }
}

void Config_user::increment_teachmes( void )
{
	try
	{
		unsafe_tree()["n_teachmessages"] = std::to_string( std::stoi( get_keyword("n_teachmessages") ) + 1 );
	}
	catch ( std::invalid_argument e ) { error_out( "Error while parsing n_teachmessages from user config: " + std::string( e.what() ) ); }
}

void Config_user::user_add_to_string_num(const std::string &path, const int v) { add_to_string_num(path, v); }

} // namespace Convspace
