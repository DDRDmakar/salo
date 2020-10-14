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
