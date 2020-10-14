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

#include <iostream>
#include <stdio.h>
#include <limits>
#include <time.h>
#include <exception>
#include <list>
#include <bitset>
#include <string.h>

// YAML
#include <yaml-cpp/yaml.h>

// ImageMagick
#include <Magick++.h>

// OpenCV
#include <opencv4/opencv2/objdetect/objdetect.hpp>
#include <opencv4/opencv2/highgui/highgui.hpp>
#include <opencv4/opencv2/imgproc/imgproc.hpp>

#include "text_tools.h"
#include "exception.h"

#ifndef sequence_check_core
#define sequence_check_core

using namespace std;

// Check, if string begins with this
bool begins_with(const std::string &current_line, const std::string &target);

// Function, which rules editing processes, applying filters.
void editor_controller(const std::string &path_to_binary, const std::string &path_to_task);

// Separate too long lines with line-breaks instead of spaces to fit some rectangle area
std::string split_too_long_lines(std::string plain_text, const int max_line_length, const int max_lines_count = 0);

// Count symbols in multibyte string
unsigned count_symbols(const std::string &plain_text);

// Copy file
void copy_file(const std::string file_name_input, const std::string file_name_output);

#endif
