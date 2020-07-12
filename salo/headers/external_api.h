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


#ifndef EXAPI
#define EXAPI

using namespace std;

#include <string>

//пустая строка в случае, если не нашлось результата по запросу
std::string wikipedia(const std::string& query);

//строка с ошибкой в случае, если не нашлось результатов по запросу
std::string weather(const std::string& query);

//строка с ошибкой в случае если количество фраз не в диапазоне.
std::string generateSpeech(int n);

#endif
