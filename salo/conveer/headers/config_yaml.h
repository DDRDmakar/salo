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

#ifndef __SEQUENCE_CHECK_CONFIG_YAML__
#define __SEQUENCE_CHECK_CONFIG_YAML__

#include <string>
#include <wchar.h>
#include <vector>
#include <regex>

#include <yaml-cpp/yaml.h>

using namespace std;

/*
 * Manages with YAML tree
 * 
 * Class for technical information storage.
 * All coefficients and text constants
 * could be stored here. Class provides access
 * to internal information, if programm needs it
 * through special safe functions, or directly from DOM object.
 * (Not required to use this class in fast-looping parts of code).
 * 
 * If config has incorrect syntax, user gets popup error message and program stops reading files.
 * 
 * If user is trying to access non-existing key, he will get console error message 
 * and empty container or zero value.
 * Node name shouldn't start or end with /
 * If nodes have the same keys, programm returns only first of them
 * 
 * YAML::Node::Type
 * {
 *     Null
 *     Scalar
 *     Sequence
 *     Map
 *     Undefined
 * }
 * 
 * Compile arguments for g++ :
 * -I /usr/local/include/ /usr/local/lib/libyaml-cpp.a
 * 
 * Better use this class by pointer and recreate object to prevent memory leaking.
 */

namespace Convspace
{

class Config_yaml
{
	
private:
	
	const YAML::Node unsafe_subtree(const std::string &target); // Find node in structure by address, throws exceptions (BE CAREFUL!!!)
	
protected:
	
	YAML::Node config_storage; // YAML-CPP object, containing all information
	
	// They throw Convspace::Exception(), be careful!
	void insert_into_set(const std::string &path, const std::string &value); // Insert element into sequence
	void set_value(const std::string &path, const std::string &value); // Override existing value ( map support only! )
	void clear_node(const std::string &path = "");                         // Clear node
	void seq_delete_index(const std::string &path, const int index); // Delete element from sequence by index
	void seq_delete_value(const std::string &path, const std::string &value); // Delete element from sequence by value
	void create_node(const std::string &path);  // Create empty node (default node = '')
	void add_to_string_num(const std::string &path, const int v); // Increment number value, which is stored as string
	
public:
	
	Config_yaml(const std::string &plain_text = ""); // Constructor from string
	Config_yaml(const YAML::Node &parent_node); // Constructor from node
	~Config_yaml(void); // Destructor
	
	void put_data( const std::string &plain_text = "" );
	const YAML::Node subtree(const std::string &target = ""); // Get pointer to DOM object
	YAML::Node unsafe_tree(void); // Get node to edit it ( BE CAREFUL! )
	YAML::Node editable_subtree(const std::string &target = ""); // Get subnode to edit it ( BE CAREFUL! )
	std::string show(const std::string &path = ""); // Stringify node
	// Config_yaml copy(const std::string &path = ""); // Safely copy node
	
	// Functions to access config from 
	    // Primary functons
	int get_coefficient( const std::string &target ); // Coefficient ( integer )
	std::string get_keyword( const std::string &target ); // String
	std::vector <std::string> get_group( const std::string &target ); // List of strings
	std::string get_group( const std::string &target, const unsigned int position ); // Get element from list of strings
	int get_group_size( const std::string &target ); // Get size of string list
	bool get_binary( const std::string &target ); // Boolean value
	bool get_possibility( const std::string &target ); // Availability
	    // Secondary functions
	std::string get_group_random( const std::string &target ); // Random string from group
	bool one_matches( const std::string &target, const std::string &elem ); // If any element matches given element
	std::regex get_regex( const std::string &target ); // Get regex object from config
	    // Logical functions
	bool there_is_element(const std::string &target); // Check, if element of such type exists
	
	    // And the same functions for wide lines
	int get_coefficient( const std::wstring &target );
	std::wstring get_keyword( const std::wstring &target );
	std::vector <std::wstring> get_group( const std::wstring &target );
	std::wstring get_group( const std::wstring &target, const unsigned int position );
	int get_group_size( const std::wstring &target );
	bool get_binary( const std::wstring &target );
	bool get_possibility( const std::wstring &target );
	
	std::wstring get_group_random( const std::wstring &target );
	bool one_matches( const std::wstring &target, const std::wstring &elem );
	std::wregex get_regex( const std::wstring &target );
	
	bool there_is_element(const std::wstring &target);
	
};

} // namespace Convspace

#endif
