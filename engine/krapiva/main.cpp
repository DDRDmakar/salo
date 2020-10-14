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

// ./krapiva "набор аргументов для моего парсера"
// В аргументах:
//     1. Заменить \ на \\
//     2. Заменить " на \"
//     3. Заменить переносы строк на \n

#include "headers/core.h"

int main(int argc,char **argv) 
{
	Magick::InitializeMagick(*argv);
	
	// Where program locates
	const std::string path_to_binary = std::string(argv[0]).substr(0, std::string(argv[0]).find_last_of("/") + 1);
	// Directory with the task, slash always in the end
	const std::string path_to_task = argc < 2 ? "" : std::string(std::string(argv[1]) + ((strlen(argv[1]) == 0 || argv[1][strlen(argv[1]) - 1] == '/') ? "" : "/"));
	
	// Randomizer
	srand(time(NULL));
	
	try { editor_controller(path_to_binary, path_to_task); }
	catch (Magick::ErrorBlob &e)
	{
		copy_file(path_to_binary + "resources/error_noimage.png", path_to_task + "output.png");
		std::cout << "Error - source image for editor not found\n";
	}
	catch (std::invalid_argument &e)
	{
		copy_file(path_to_binary + "resources/error_wrongargs.png", path_to_task + "output.png");
		std::cout << "Exception: " << e.what() << std::endl;
	}
	catch (YAML::Exception &e)
	{
		copy_file(path_to_binary + "resources/error_wrongargs.png", path_to_task + "output.png");
		std::cout << "Exception: " << e.what() << std::endl;
	}
	catch (krapiva::Exception_face &e)
	{
		copy_file(path_to_binary + "resources/error_noface.png", path_to_task + "output.png");
		std::cout << "Exception: " << e.what() << std::endl;
	}
	catch (std::exception) { std::cout << "Error while image processing\n"; }
	
	return 0; 
}

void copy_file(const std::string file_name_input, const std::string file_name_output)
{
	std::ifstream in;
	std::ofstream out;
	//Open the file
	in.open(file_name_input);
	//Failed to open file
	if(in.fail())
	{
		std::cout << "Failed to open file " << file_name_input << std::endl;
		in.close();
		out.close();
		return;
	}
	out.open(file_name_output);
	if(out.fail())
	{
		std::cout << "Failed to create file " << file_name_input << std::endl;
		in.close();
		out.close();
		return;
	}
	
	out << in.rdbuf();
	out.close();
	in.close();
}
