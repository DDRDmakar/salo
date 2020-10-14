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

#include "headers/arg_parser.h"
#include "headers/parse_tools.h"

namespace Convspace
{

Arg_parser::Arg_parser(const std::string &args)
{
	const auto args_storage = split_args(args); // Storage of arguments with it's status
	
	full_text = args; // Save full command-line text
	
	bool named_single = false; // If single named argument reading is active
	bool named_array = false;  // if named array reading is active
	
	std::vector<std::string> current_array; // Collector
	std::string current_name; // Name of current single variable or array of variables
	
	for (const auto &e : args_storage)
	{
		// Save each argument in storage of all arguments as usual string
		all_args.push_back(std::get<0>(e));
		
		if (std::get<1>(e)) // If function meets parameter
		{
			named_single = false; // Prevent reading empty single variables
			
			if (named_array) // If we are reading array, finish reading array
			{
				// Save array
				if (!current_name.empty()) array_named.insert({current_name, current_array});
				// Clear current array
				current_array.clear();
				// Disable array reading
				named_array = false;
			}
			
			// Saving current name to temporary variable
			current_name = std::get<0>(e);
			// Enabling named array/single variable reading
			(begins_with(current_name, "--") ? named_array : named_single) = true;
			// Delete first minus
			current_name.erase(current_name.begin());
			// Delete second minus, if it exists
			if (!current_name.empty() && current_name.front() == '-') current_name.erase(current_name.begin());
			
			continue;
		}
		
		// If function is reading single variable, finish reading single variable
		if (named_single)
		{
			// Save current argument and name
			if (!current_name.empty()) arg_named.insert({current_name, std::get<0>(e)});
			// Disable single parameter reading
			named_single = false;
			
			continue;
		}
		
		// If function is reading array
		if (named_array) { current_array.push_back(std::get<0>(e)); continue; }
		
		// Save unnamed parameters
		arg_unnamed.push_back(std::get<0>(e));
	}
	
	// If we are reading array, save array
	if (named_array && !current_name.empty()) array_named.insert({current_name, current_array});
}

std::vector<std::pair<std::string, bool>> Arg_parser::split_args(const std::string &source_line)
{
	std::vector<std::pair<std::string, bool>> target; // Answer
	std::string current_line;   // Collector line
	bool quote_double = false;  // Quotation status
	bool quote_single = false;
	bool escaped = false; // Escape status
	
	for (const auto e : source_line)
	{
		// If function meets double quote
		if (e == '\"' && !escaped && !quote_single)
		{
			// If we have unsaved string, save it
			// If current line is beyond quotes and begins with minus, mark it as parameter
			if (!current_line.empty() || quote_double)
				target.push_back(std::make_pair(current_line, !quote_double && begins_with(current_line, "-")));
			current_line.clear(); // Clear collector
			quote_double = !quote_double; // Change quotation status
			continue;
		}
		// If function meets single quote
		if (e == '\'' && !escaped && !quote_double)
		{
			// If we have unsaved string, save it
			// If current line is beyond quotes and begins with minus, mark it as parameter
			if (!current_line.empty() || quote_single)
				target.push_back(std::make_pair(current_line, !quote_single && begins_with(current_line, "-")));
			current_line.clear(); // Clear collector
			quote_single = !quote_single; // Change quotation status
			continue;
		}
		
		// If function meets escaped line-break character
		if (e == 'n' && escaped && !quote_single)
		{
			current_line.push_back('\n'); // Save line-break into collector
			escaped = false; // End escape
			continue;
		}
		
		// If function meets line-break or space beyond quotes
		if (!escaped && !quote_double && !quote_single && (e == ' ' || e == '\n'))
		{
			// If we have unsaved string, save it
			// If current line begins with minus, mark it as parameter
			if (!current_line.empty())
				target.push_back(std::make_pair(current_line, begins_with(current_line, "-")));
			current_line.clear(); // Clear collector
			continue;
		}
		
		// If function meets escaping character
		// If it's not escaped, then start escape
		if (e == '\\' && !escaped && !quote_single) { escaped = true; continue; }
		// Else we have to end escape (tell, that it's normal symbol, need it to work with "\" symbol)
		// And other symbols, which shouldn`t be escaped
		else escaped = false;
		
		// ==== End of escaped characters processing, under it 'ESCAPED' always FALSE
		
		// Push current character into collector
		current_line.push_back(e);
	}
	// If we have unsaved string, save it
	// If current line is beyond quotes and begins with minus, mark it as parameter
	if (!current_line.empty() || quote_double)
		target.push_back(std::make_pair(current_line, !quote_double && begins_with(current_line, "-")));
	
	return target;
}

// All arguments access
std::vector<std::string> Arg_parser::get_all_args(void) const { return all_args; }
std::string Arg_parser::get_arg(const unsigned int index) const
{
	try { return all_args.at(index); }
	catch (std::out_of_range) { return ""; }
}
std::string Arg_parser::get_all_args_line(void) const { return full_text; }
std::string Arg_parser::get_first_args_line(const unsigned int to_index) const
{
	std::stringstream ss;
	for (int i = 0; i < all_args.size() && i < to_index; ++i)
		ss << all_args[i] << ((i + 1 < all_args.size() && i + 1 < to_index) ? " " : "");
	return ss.str();
}
std::string Arg_parser::get_last_args_line(const unsigned int from_index) const
{
	std::stringstream ss;
	for (int i = from_index; i < all_args.size(); ++i)
		ss << all_args[i] << (i + 1 < all_args.size() ? " " : "");
	return ss.str();
}

// Unnamed arguments access
std::vector<std::string> Arg_parser::get_array_unnamed(void) const { return arg_unnamed; }
std::string Arg_parser::get_arg_unnamed(int index) const { return index < arg_unnamed.size() ? arg_unnamed.at(index) : ""; }
// Named arguments access
std::string Arg_parser::get_arg_named(const std::string &index) const
{
	try { return arg_named.at(index); }
	catch (std::out_of_range) { return ""; }
}
std::vector<std::string> Arg_parser::get_keys_of_single(void) const
{
	std::vector<std::string> keys_array;
	for (const auto &e : arg_named) keys_array.push_back(e.first);
	return keys_array;
}
// Named argument array access
std::vector<std::string> Arg_parser::get_array_named(const std::string &index) const
{
	try { return array_named.at(index); }
	catch (std::out_of_range) { return std::vector<std::string>(); }
}
std::string Arg_parser::get_array_named_element(const std::string &index_1, int index_2) const
{
	try { return index_2 < array_named.at(index_1).size() ? array_named.at(index_1).at(index_2) : ""; }
	catch (std::out_of_range) { return ""; }
}
std::vector<std::string> Arg_parser::get_keys_of_arrays(void) const
{
	std::vector<std::string> keys_array;
	for (const auto &e : array_named) keys_array.push_back(e.first);
	return keys_array;
}


// Check
bool Arg_parser::empty(void) const { return all_args.empty(); }
bool Arg_parser::arg_exists(const std::string &value)
{
	for (const auto &e : all_args) if (e == value) return true;
	return false;
}
int Arg_parser::arg_unnamed_size(void) const { return arg_unnamed.size(); }
int Arg_parser::arg_named_size(void) const { return arg_named.size(); }
int Arg_parser::arrays_named_count(void) const { return array_named.size(); }
int Arg_parser::array_named_size(const std::string &index) const
{
	try { return array_named.at(index).size(); }
	catch (std::out_of_range) { return 0; }
}

bool Arg_parser::arg_unnamed_exists(const std::string &value) const
{
	for (const auto &e : arg_unnamed) if (e == value) return true;
	return false;
}
bool Arg_parser::arg_unnamed_not_empty(const int index) const { return arg_unnamed_size() > index && !get_arg_unnamed(index).empty(); }

bool Arg_parser::arg_named_exists(const std::string &index) const { return arg_named.find(index) != arg_named.end(); }
bool Arg_parser::arg_named_not_empty(const std::string &index) const { return arg_named_exists(index) && !get_arg_named(index).empty(); }
bool Arg_parser::array_named_exists(const std::string &index) const { return array_named.find(index) != array_named.end(); }
bool Arg_parser::array_named_not_empty(const std::string &index) const { return array_named_exists(index) && !get_array_named(index).empty(); }

} // namespace Convspace
