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

#ifndef __SEQUENCE_CHECK_ARGPARSER__
#define __SEQUENCE_CHECK_ARGPARSER__

/*
* no minuses  ()   - unnamed argument (string)             var
* one minus   (-)  - named argument (string)               -name var
* two minuses (--) - array of arguments (vector<string>)   --name var1 var2 ...
* 
* Argument - one word or another text in double-quotes ("") or single-quotes ('').
* Single argument could be empty string, if it's defined inside double-quotes ("") or single quotes ('').
* Array of arguments could be empty.
* Parameter (name of array or variable) should be one word written without quotes, should not be empty string,
* if name is empty, this variable/array will be ignored.
* Parser supports slash (\\), line-break (\n), space (\ ) and quote mark (\") escaping (except text in single quotes).
* Other escapes will be ignored (Text will be saved without slashes. In single quotes all symbols will be saved).
* Arguments witin quotes could be separated with spaces or line-breaks.
* Warning! Tabulation symbol is not a separator!
* Line-breaks inside quotes would be saved as usual character.
* If value should begin with minus, you should put it in quotes, so it won't be treated as parameter-name.
* If argument line ends without closing quote ( Example: -name "str1" -size "str2 ), str2 will end
* on the end of argument string.
* If you want more functionality, you could put YAML, JSON, XML or other structure into quotes and parse it in your code.
*/

#include <string>
#include <vector>
#include <unordered_map>
#include <tuple>

namespace Convspace
{

class Arg_parser
{
	
private:
	
	// Split command-line arguments
	std::vector<std::pair<std::string, bool>> split_args(const std::string &source_line);
	
	// Storage
	std::vector<std::string>                                   all_args;
	std::string                                                full_text;
	std::vector<std::string>                                   arg_unnamed;
	std::unordered_map<std::string, std::string>               arg_named;
	std::unordered_map<std::string, std::vector<std::string>>  array_named;
	
public:
	
	// Constructor
	Arg_parser(const std::string &args);
	
	std::vector<std::string> get_all_args(void) const;
	std::string get_arg(const unsigned int index) const;
	std::string get_all_args_line(void) const;
	std::string get_first_args_line(const unsigned int to_index) const;
	std::string get_last_args_line(const unsigned int from_index) const;
	
	// Unnamed arguments access
	std::vector<std::string> get_array_unnamed(void) const;
	std::string get_arg_unnamed(int index) const;
	// Named arguments access
	std::string get_arg_named(const std::string &index) const;
	std::vector<std::string> get_keys_of_single(void) const;
	// Named argument array access
	std::vector<std::string> get_array_named(const std::string &index) const;
	std::string get_array_named_element(const std::string &index_1, int index_2) const;
	std::vector<std::string> get_keys_of_arrays(void) const;
	
	// Check
	bool empty(void) const;
	
	bool arg_exists(const std::string &value);
	
	int arg_unnamed_size(void) const;
	int arg_named_size(void) const;
	int arrays_named_count(void) const;
	int array_named_size(const std::string &index) const;
	
	bool arg_unnamed_exists(const std::string &value) const;
	bool arg_unnamed_not_empty(const int index) const;
	bool arg_named_exists(const std::string &index) const;
	bool arg_named_not_empty(const std::string &index) const;
	bool array_named_exists(const std::string &index) const;
	bool array_named_not_empty(const std::string &index) const;
};

} // namespace Convspace

#endif
