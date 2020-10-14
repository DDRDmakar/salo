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

#include "headers/text_tools.h"

// Read text file
std::string read_file( const std::string &target )
{
	if (!std::ifstream(target).is_open()) throw std::invalid_argument("Error - file \"" + target + "\" not found");
	
	std::ifstream input_file_stream( target, std::ios::binary );
	input_file_stream.clear();
	std::stringstream ss;
	ss << input_file_stream.rdbuf();
	input_file_stream.close();
	return ss.str();
}
