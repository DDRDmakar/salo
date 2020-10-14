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