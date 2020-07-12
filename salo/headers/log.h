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
