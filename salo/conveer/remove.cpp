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

#include "headers/parse_tools.h"

namespace Convspace
{



// Заменяет в тексте данную последовательность символов на другую данную последовательность
void replace_from_text( std::wstring &current_line, const std::wstring &part, const std::wstring &n_part, std::wstring *current_punctuation_string )
{
	if( part.empty() || appears_in_this_string(n_part, part) ) return;
	int position = current_line.find( part );
	while ( position != std::wstring::npos )
	{
		if( current_punctuation_string ) *current_punctuation_string += part; // Добавляем её в хранилище знаков пунктуации
		current_line.replace( position, part.length(), n_part ); // Заменяем её на n_part
		position = current_line.find( part ); // Находим следующее место, где встречается эта последовательность
	}
}
void replace_from_text( std::string &current_line, const std::string &part, const std::string &n_part, std::string *current_punctuation_string )
{
	if( part.empty() || appears_in_this_string(n_part, part) ) return;
	int position = current_line.find( part );
	while( position != std::string::npos )
	{
		if( current_punctuation_string ) *current_punctuation_string += part; // Добавляем её в хранилище знаков пунктуации
		current_line.replace( position, part.length(), n_part ); // Заменяем её на n_part
		position = current_line.find( part ); // Находим следующее место, где встречается эта последовательность
	}
}

// Один раз заменяет в тексте данную последовательность символов на другую данную последовательность
void replace_in_text( std::wstring &current_line, const std::wstring &part, const std::wstring &n_part, std::wstring *current_punctuation_string )
{
	if( part.empty() || part == n_part ) return;
	int position = current_line.find( part );
	while ( position != std::wstring::npos )
	{
		if( current_punctuation_string ) *current_punctuation_string += part; // Добавляем её в хранилище знаков пунктуации
		current_line.replace( position, part.length(), n_part ); // Заменяем её на n_part
		position = current_line.find( part, position + n_part.length() ); // Находим следующее место, где встречается эта последовательность
	}
}
void replace_in_text( std::string &current_line, const std::string &part, const std::string &n_part, std::string *current_punctuation_string )
{
	if( part.empty() || part == n_part ) return;
	int position = current_line.find( part );
	while( position != std::string::npos )
	{
		if( current_punctuation_string ) *current_punctuation_string += part; // Добавляем её в хранилище знаков пунктуации
		current_line.replace( position, part.length(), n_part ); // Заменяем её на n_part
		position = current_line.find( part, position + n_part.length() ); // Находим следующее место, где встречается эта последовательность
	}
}

// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________

void remove_keyword( std::wstring &current_line, const std::wstring &target ) // Удаление ключевого слова ( в начале сообщения )
{
	clear_spaces( current_line );
	if( current_line.length() < target.length() ) return;
	if( target.empty() )
	{
		size_t space = current_line.find(L" ");
		if (current_line.find(L"\n") < space) space = current_line.find(L"\n");
		if( space == std::wstring::npos ) space = current_line.length();
		current_line.erase( 0, space );
	}
	else
	{
		if( current_line.compare( 0, target.length(), target ) == 0 ) current_line.erase( 0, target.length() );
	}
	
	clear_spaces( current_line ); // Удаление оставшихся пробелов
}
void remove_keyword( std::string &current_line, const std::string &target ) // Удаление ключевого слова ( в начале сообщения )
{
	clear_spaces( current_line );
	if( current_line.length() < target.length() ) return;
	if( target.empty() )
	{
		int space = current_line.find(" ");
		if (current_line.find("\n") < space) space = current_line.find("\n");
		if( space == std::string::npos ) space = current_line.length();
		current_line.erase( 0, space );
	}
	else
	{
		if( current_line.compare( 0, target.length(), target ) == 0 ) current_line.erase( 0, target.length() );
	}
	
	clear_spaces( current_line ); // Удаление оставшихся пробелов
}

// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________

void clear_spaces(std::wstring& current_line) // Удаление лишних пробелов для толстой строки
{
	replace_from_text(current_line, L"	", L" ");
	while(!current_line.empty() && (current_line.front() == L' ' || current_line.front() == L'\n')) current_line.erase(current_line.begin());
	replace_from_text(current_line, L"  ", L" "); // Удаление двойных пробелов
	while(!current_line.empty() && (current_line.back() == L' ' || current_line.back() == L'\n')) current_line.pop_back();
}

void clear_spaces(std::string& current_line) // Удаление лишних пробелов для тонкой строки
{
	replace_from_text(current_line, "	", " ");
	while(!current_line.empty() && (current_line.front() == ' ' || current_line.front() == '\n')) current_line.erase(current_line.begin());
	replace_from_text(current_line, "  ", " "); // Удаление двойных пробелов
	while(!current_line.empty() && (current_line.back() == ' ' || current_line.back() == '\n')) current_line.pop_back();
}


} // namespace Convspace
