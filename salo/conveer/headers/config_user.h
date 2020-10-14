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


#ifndef __SEQUENCE_CHECK_CONFIG_USER__
#define __SEQUENCE_CHECK_CONFIG_USER__

#include <string>
#include <wchar.h>
#include <vector>
#include <regex>
#include "config_yaml.h"

using namespace std;

/*
 * 
 * Class, managing with individual user config information.
 * Safely queries through the YAML tree and catches all exceptions,
 * controls arguments 
 * 
 */

namespace Convspace
{

class Config_user : public Config_yaml
{
private:
	
	std::string posix_time = "%Y-%m-%dT%H:%M:%S.0Z";
	
public:
	
	// Constructor
	Config_user(const std::string &plain_node_text);
	
	void add_basic_data_user(
		const std::string &id,
		const std::string &first_name,
		const std::string &last_name
	);
	
	
	void write_last_activity( void ); // Write down time of last activity
	bool is_in_game( void ); // Check, if user is in game
	bool is_teacher( void ); // Check, if user is in teachmode
	void increment_mes( void ); // Increment n_messages value
	void increment_teachmes( void ); // Increment n_teachmessages value
	
	void user_add_to_string_num(const std::string &path, const int v); // Increment number value, which is stored as string
};

} // namespace Convspace

#endif
