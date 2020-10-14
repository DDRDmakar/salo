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

#ifndef __SEQUENCE_CHECK_FILE__
#define __SEQUENCE_CHECK_FILE__

#include <fstream>
#include <sstream>

extern std::string PathToBinary;

namespace Convspace
{
	bool dir_exists(const std::string &target); // Check, if directory exists
	void make_dir(const std::string &target); // Create directory
	bool file_exists(const std::string &target); // Check, if file exists
	std::wstring read_file(const std::string &target); // Reads text file
	void write_file(const std::string &target, const std::wstring &text); // Writes text into file
	void copy_file(const std::string &filename_in, const std::string &filename_out); // Cope file
}

#endif
