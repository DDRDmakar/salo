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
#ifndef sequence_check_translate
#define sequence_check_translate null

#include "network.h"

class Translate
{
private:
	std::string API_KEY;
	network* pYaNetwork;

public:
	Translate();
	~Translate();

	//Определение языка
	//аргументы: текст, язык которого необходимо определить
	//возвращает язык, который можно использовать для функции перевода или TRANSLATOR_FAILED при ошибке
	std::string DetectLanguage(const std::string& Text);

	//Перевод
	//аргументы: текст для перевода, язык источника (использовать детект для него), язык назначения
	//возвращает строку с переведнным текстом или TRANSLATOR_FAILED при ошибках
	std::string TranslateText(const std::string& Text, const std::string SourceLang, const std::string DestLang);

	//...
	bool TranslatorEnabled;
};


extern Translate* translate;

#endif