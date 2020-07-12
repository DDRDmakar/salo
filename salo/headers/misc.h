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

#ifndef MISC 
#define MISC

void miscFileToConsole(const char* fname);
void miscCheckExecName(const char* fname);
char* miscFindAndCopy(const char* where, const char* what, const char* end, int offset = 0);
std::string miscFindAndCopyString(const char* where, const char* what, const char* end, int offset = 0);
std::string miscInputPassword();
int miscSecondsToHours(int seconds);
void miscPrintAccessToken(const std::string& token); 
std::string miscGetStringFromFile(const char* fname);
long miscUnixTimeNow();
std::string miscSecondsToDHMS(int seconds);
std::string miscSecondsToRusDHM(int seconds);
std::string miscGetFile(const char* fname, int ignoreCRLF = 0);
void miscSetFile(const char* fname, const std::string& str);
bool miscFileExists(const char* fname);
std::string miscGetCurrentDateTime();
std::string miscExecSystem(const char*);
std::string miscFormatted(const std::string& what, int mode);
std::string miscUpscale(const std::string& what, int scale);
std::string miscStringDouble(double val, int pers);
std::string miscFilterLetters(const std::string& src);

#endif