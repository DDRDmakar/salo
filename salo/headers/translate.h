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