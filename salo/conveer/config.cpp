
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

#include <fstream>
#include <sstream>

#include "../headers/log.h"
#include "headers/config.h"
#include "headers/parse_tools.h"
#include "headers/exception.h"

#include <yaml-cpp/yaml.h>

namespace Convspace
{

Config::Config( const std::string &transit_dir_name ) // Constructor
{
	logs -> CommitConveer( F, L, "Object initialization - Config" );
	
	version = 0;
	if ( transit_dir_name.empty() ) return;
	dir_name = (transit_dir_name.back() == '/') ? transit_dir_name : transit_dir_name + "/";
	refresh();
}

std::string Config::get_dir_name( void ) { return dir_name; }

int Config::get_version( void ) { return version; }

void Config::refresh( void )
{
	locale::global (locale ("")); // Установка локали для работы потока
	setlocale( LC_ALL, "" );
	
	logs -> CommitConveer( F, L, "Refreshing config" );
	
	try
	{
		// Assemble file
		// Max 24 include levels
		if (!dir_exists("resources/config/" + dir_name)) throw Convspace::Exception("Error - directory \"resources/config/" + dir_name + "\" does not exist.\nCan't create bot.");
		std::wstring config_text = recursive_read_file( "resources/config/" + dir_name + "main.yaml", 24 );
		// Parse file
		put_data(wide_to_thin_line(config_text));
		// Add extra nodes
		// addition_write( wide_to_thin_line( read_file( "resources/config/" + dir_name + "addition.txt" ) ) );
		addition_write(wide_to_thin_line(file_exists("resources/config/" + dir_name + "addition.yaml") ? read_file("resources/config/" + dir_name + "addition.yaml") : L""));
		
		++ version;
	}
	catch ( std::string e )
	{
		error_out(std::string("ERROR - config parsing terminated: " + e));
		throw Convspace::Exception_config(std::string("ERROR - config parsing terminated: " + e));
	}
}

std::wstring Config::recursive_read_file( const std::string &current_path, const int n_calls )
{
	if (n_calls <= 0) throw std::string( "Too_deep_include." );
	if (ends_with(current_path, "/")) throw std::string( "Incorrect path: " + current_path );
	
	std::wstring current_text = L"\n" + read_file( current_path ); // File text
	std::string current_dir = current_path.substr(0, current_path.find_last_of("/") + 1); // Directory address
	
	while(true)
	{
		std::wsmatch includies;
		std::regex_search( current_text, includies, std::wregex( L"\\n#include [^\\n#]+\\.[^\\n#]+#" ) );
		if (includies.size() == 0) break;
		
		std::wstring full_sequence = includies.str(0);
		std::wstring local_include_address = full_sequence.substr(9, full_sequence.length() - 9 - 1);
		clear_spaces(local_include_address);
		std::wstring include_address_data = recursive_read_file(current_dir + wide_to_thin_line(local_include_address), n_calls - 1);
		replace_in_text(current_text, full_sequence, L"\n" + include_address_data );
	}
	
	return current_text;
}

void Config::addition_write( const std::string &source_line )
{
	// vector of strings
	//auto current_storage = wide_to_thin_storage(split_args(thin_to_wide_line(source_line)));
	
	//unsigned int i = 0;
	//unsigned int pos = 0;
	
	try
	{
		// Create config object to store additions
		Config_yaml current_config(source_line);
		
		// Check, if there are any additions
		if (current_config.there_is_element("additions"))
		{
			// Check, if addition list has appropriate format (sequence)
			if (!current_config.there_is_element("additions") || (!current_config.subtree("additions").IsSequence() && !current_config.subtree("additions").IsMap()))
			{
				error_out("ERROR reading addition into config - additions node format is incorrect.");
				return;
			}
			
			// Additions set
			YAML::Node addition_storage = current_config.subtree("additions");
			
			// Iterate through additions
			if (addition_storage.IsSequence())
			{
				for (YAML::const_iterator it = addition_storage.begin(); it != addition_storage.end(); ++it)
				{
					// Addition mode, tells, what to do with arguments
					const std::string addition_mode = it->operator[]("mode").as<std::string>();
					
					// Insert value into set
					if (addition_mode == "insert") { insert_into_set(it->operator[]("path").as<std::string>(), it->operator[]("value").as<std::string>()); continue; }
					
					// Override existing value, or create new
					if (addition_mode == "set") { set_value(it->operator[]("path").as<std::string>(), it->operator[]("value").as<std::string>()); continue; }
					
					// Clear node contents
					if (addition_mode == "clear") { clear_node(it->operator[]("path").as<std::string>()); continue; }
					
					// Delete value from sequence by index
					if (addition_mode == "seq_delete_index") { seq_delete_index(it->operator[]("path").as<std::string>(), it->operator[]("index").as<int>()); continue; }
					
					// Delete value from sequence by value
					if (addition_mode == "seq_delete_value") { seq_delete_value(it->operator[]("path").as<std::string>(), it->operator[]("value").as<std::string>()); continue; }
					
					// Create empty node (empty list)
					if (addition_mode == "create") { create_node(it->operator[]("path").as<std::string>()); continue; }
				}
			}
			else if (addition_storage.IsMap())
			{
				for (YAML::const_iterator it = addition_storage.begin(); it != addition_storage.end(); ++it)
				{
					// Addition mode, tells, what to do with arguments
					const std::string addition_mode = it->second["mode"].as<std::string>();
					
					// Insert value into set
					if (addition_mode == "insert") { insert_into_set(it->second["path"].as<std::string>(), it->second["value"].as<std::string>()); continue; }
					
					// Override existing value, or create new
					if (addition_mode == "set") { set_value(it->second["path"].as<std::string>(), it->second["value"].as<std::string>()); continue; }
					
					// Clear node contents
					if (addition_mode == "clear") { clear_node(it->second["path"].as<std::string>()); continue; }
					
					// Delete value from sequence by index
					if (addition_mode == "seq_delete_index") { seq_delete_index(it->second["path"].as<std::string>(), it->second["index"].as<int>()); continue; }
					
					// Delete value from sequence by value
					if (addition_mode == "seq_delete_value") { seq_delete_value(it->second["path"].as<std::string>(), it->second["value"].as<std::string>()); continue; }
					
					// Create empty node (empty list)
					if (addition_mode == "create") { create_node(it->second["path"].as<std::string>()); continue; }
				}
			}
			else throw Convspace::Exception("Error - unknown addition list format");
		}
	}
	catch (YAML::Exception &e)
	{
		error_out("ERROR parsing addinion to config: " + std::string(e.what()));
		throw Convspace::Exception_config("ERROR parsing addinion to config: " + std::string(e.what()));
	}
	catch (Convspace::Exception_config &e)
	{
		error_out("ERROR parsing addinion to config: " + std::string(e.what()));
		throw e;
	}
}

} // namespace Convspace
