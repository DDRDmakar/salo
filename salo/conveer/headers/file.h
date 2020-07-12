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
