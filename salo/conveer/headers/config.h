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


#ifndef __SEQUENCE_CHECK_CONFIG__
#define __SEQUENCE_CHECK_CONFIG__

#include <string>
#include <wchar.h>
#include <vector>
#include <regex>

#include "config_yaml.h"

using namespace std;

/*
 * 
 * Config, working with files in existing directory.
 * Files are CONST
 * 
 * Constructor - gets directory name, where config is placed.
 *               Root directory is in folder "resources/config"
 * To include another file to config, use #include <filename>#
 * This sequence will be replaced with text from another file.
 * 
 * If config has incorrect syntax, user gets popup error message and program stops reading files.
 * If file includes invalid link to another file, config stops reading files.
 * Directory name should end with '/'.
 * File name should not start with '/'.
 * File path and name should not contain more than 1 space.
 * and empty container or zero value.
 * Node name shouldn't start or end with /
 * 
 * File "addition.yaml" contains extra nodes, which should be inserted into config tree.
 * It could be useful for configs, which are included from external template.
 * Syntax:
 * 
 ** additions:
 **     - { mode: insert, path: <path>, value: <value> }
 **     - { mode: set, path: <path>, value: <value> }
 **     - { mode: clear, path: <path> } [BUG]
 **     - { mode: seq_delete_index, path: <path>, value: <index> } [BUG]
 **     - { mode: seq_delete_value, path: <path>, value: <value> } [BUG]
 **     - { mode: create, path: <path>}
 * 
 */

namespace Convspace
{

class Config : public Config_yaml
{
	
private:
	
	int version; // Counter to control config versions
	std::string dir_name; // Directory name for config
	
	// Assembles all text config files in one, linking files to text
	std::wstring recursive_read_file( const std::string &current_path, const int n_calls ); // Read config files
	void addition_write( const std::string &source_line ); // Inserts extra nodes into config tree
	
public:
	
	Config( const std::string &tansit_dir_name = "" );
	void refresh( void ); // Reload information from files
	std::string get_dir_name( void ); // Get directory name of this config
	int get_version( void ); // Get config version (0, 1, 2, 3 ... )
	
};

} // namespace Convspace

#endif
