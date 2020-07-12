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

#ifndef sequence_check_salostring
#define sequence_check_salostring null

#include <string>

char* WideToMultibyte(const wchar_t* wide); //classic
std::string WideToMultibyteString(const std::wstring& wide); //stl

wchar_t* MultibyteToWide(const char* multi); //classic
std::wstring MultibyteToWideString(const std::string& multi); //stl

std::string clipper(const std::string& multi);

std::string getUnicodeFromCodes(const std::string& str); //суррогаты переводит в мультибайт

#endif