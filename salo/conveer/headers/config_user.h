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
