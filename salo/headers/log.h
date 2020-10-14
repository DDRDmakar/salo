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

#ifndef LOGHEADER
#define LOGHEADER

#include <string>

class Log
{
private:
	std::string LOGDATE;
	std::string LOGFOLDER;

	std::string LOG_GENERIC;
	std::string LOG_CONVEER;
	std::string LOG_ERROR;
//	std::string LOG_ECMAIN;
//	std::string LOG_ECMAOUT;

	bool ECHO;
	bool ECMA_ECHO;

	//i/o buffers
	std::string conveer_buffer;
	std::string generic_buffer; 

public:
	void TriggerEcho();
	//void TriggerEcmaEcho();
	
	void CommitGeneric(const char* file, const int line, const std::string& message, const std::string& unique="generic");
	void CommitConveer(const char* file, const int line, const std::string& message, const std::string& unique="generic");
	//void Update(int out, const std::string& message);

	Log();
	~Log();
	
};


#define ECMA_IN 0
#define ECMA_OUT 1
#define F __FILE__
#define L __LINE__


extern Log* logs;

#endif
