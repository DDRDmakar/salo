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

#include <sys/types.h>
#include <sys/stat.h>

#include "headers/parse_tools.h"
#include "headers/exception.h"

namespace Convspace
{

// Check, if directory exists
bool dir_exists(const std::string &target)
{
	struct stat info;
	if(stat(((begins_with(target, "/") ? "" : PathToBinary) + target).c_str(), &info) != 0)
		return false;
	return (info.st_mode & S_IFDIR);
}

// Create directory
void make_dir(const std::string &target)
{
	if (mkdir(((begins_with(target, "/") ? "" : PathToBinary) + target).c_str(), ACCESSPERMS) != 0) throw Convspace::Exception("Error - dir \"" + target + "\" cannot be created.");
}

// Check, if file exists
bool file_exists(const std::string &target)
{
	std::locale::global(std::locale(""));
	std::wifstream input_file_stream((begins_with(target, "/") ? "" : PathToBinary) + target);
	return input_file_stream.is_open();
}

// Read text file
std::wstring read_file(const std::string &target)
{
	std::locale::global(std::locale("")); // Установка локали для работы толстого потока
	
	std::wifstream input_file_stream((begins_with(target, "/") ? "" : PathToBinary) + target, std::ios::in | std::ios::binary);
	
	if(!input_file_stream.is_open())
	{
		error_out("ERROR - while reading, file \"" + target + "\" does not exist");
		return L"";
	}
	
	std::wstringstream wss;
	wss << input_file_stream.rdbuf();
	input_file_stream.close();
	return wss.str();
}

// Write text into file
void write_file(const std::string &target, const std::wstring &text)
{
	std::locale::global(std::locale("")); // Установка локали для работы толстого потока
	std::wofstream output_file_stream((begins_with(target, "/") ? "" : PathToBinary) + target, std::ios::out | std::ios::binary);
	if(!output_file_stream.is_open()) error_out("ERROR - while opening file for writing: \"" + target + "\"");
	output_file_stream << text;
	if(!output_file_stream.good()) error_out("ERROR - while writing text into file: \"" + target + "\". Maybe locale is not correct?");
	output_file_stream.close();
}

void copy_file(const std::string &filename_in, const std::string &filename_out)
{
	std::ifstream file_in;
	std::ofstream file_out;
	//Open the file
	file_in.open((begins_with(filename_in, "/") ? "" : PathToBinary) + filename_in);
	//Failed to open file
	if(file_in.fail())
	{
		file_in.close();
		file_out.close();
		throw Convspace::Exception("Failed to open file " + filename_in);
	}
	file_out.open((begins_with(filename_out, "/") ? "" : PathToBinary) + filename_out);
	if(file_out.fail())
	{
		file_in.close();
		file_out.close();
		throw Convspace::Exception("Failed to copy file " + filename_in);
	}
	
	file_out << file_in.rdbuf();
	file_out.close();
	file_in.close();
}

// read binary file
// WARINIG !!! Allocates memory!
char* read_file_bin(const std::string &target)
{
	std::ifstream input_file_stream((begins_with(target, "/") ? "" : PathToBinary) + target, std::ios::in | std::ios::binary);
	
	if (!input_file_stream.is_open())
	{
		error_out("ERROR - while reading, file \"" + target + "\" does not exist");
		return nullptr;
	}
	
	// get length of file:
	input_file_stream.seekg(0, input_file_stream.end);
	int stream_len = input_file_stream.tellg();
	input_file_stream.seekg(0, input_file_stream.beg);
	
	char *buffer = new char[stream_len];
	
	input_file_stream.read(buffer, stream_len);
	
	if (!input_file_stream) error_out("ERROR - while reading, only " + std::to_string(input_file_stream.gcount()) + " could be read");
	
	input_file_stream.close();
	
	// ...buffer contains the entire file...
	// delete[] buffer;
	
	return buffer;
}

// Write binary file
// WARINIG !!! uses external pointer!
void write_file_bin(const std::string &target, const char *binfile)
{
	std::ofstream output_file_stream((begins_with(target, "/") ? "" : PathToBinary) + target, std::ios::out | std::ios::binary);
	if(!output_file_stream.is_open()) error_out("ERROR - while opening file for writing: \"" + target + "\"");
	
	output_file_stream.write(binfile, sizeof(binfile));
	if(!output_file_stream.good()) error_out("ERROR - while writing text into file: \"" + target + "\". Maybe locale is not correct?");
	output_file_stream.close();
}

} // namespace Convspace
