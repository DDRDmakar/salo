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
#include "../headers/log.h"

using namespace std;

namespace Convspace
{

// Проверяет, принадлежит ли символ к символам пунктуации
bool detect_punctuation( wchar_t symbol )
{
	// 400 - 4FF кириллица
	
	return // Если символ - не буква
	(
		( symbol >= 0x9  && symbol <= 0xF  )	||
		( symbol >= 0x1C && symbol <= 0x1F )	||
		( symbol >= 0x21 && symbol <= 0x2F )	||
		( symbol >= 0x3A && symbol <= 0x40 )	||
		( symbol >= 0x5B && symbol <= 0x60 )	||
		( symbol >= 0x7B && symbol <= 0x7F )	||
		( symbol >= 0x80 && symbol <= 0x89 )	||
		symbol == 0x8B ||
		symbol == 0x8D ||
		( symbol >= 0x8F && symbol <= 0x99 )	||
		symbol == 0x9B ||
		symbol == 0x9D ||
		symbol == 0xA0 ||
		( symbol >= 0xA2 && symbol <= 0xBF )	||
		symbol == 0xD7 ||
		symbol == 0xF7 ||
		symbol == L'—' ||
		symbol == L'«' ||
		symbol == L'»' ||
		symbol == L'…'
	);
}

// Проверяет, есть ли в тексте такое слово ( возвращает количество совпадений )
int word_exists( const std::vector <std::wstring> &current_storage, const std::wstring &target )
{
	int c = 0;
	for( int i = 0; i < current_storage.size(); ++i ) if( current_storage[i] == target ) ++c;
	return c;
}

// Проверяет, существует ли такое слово в этом хранилище
bool word_exists( const std::vector <std::wstring> &current_storage, const std::wstring &target, int position )
{
	if( current_storage.size() <= position ) return false;
	return( current_storage[position] == target );
}

// Проверяет, существует ли в тексте такая последовательность символов
bool appears_in_this_string( const std::wstring& current_line, const std::wstring &target )
{
	if( target.empty() ) return false;
	return( ( int ) current_line.find( target ) != std::wstring::npos );
}
bool appears_in_this_string( const std::string& current_line, const std::string &target )
{
	if( target.empty() ) return false;
	return( ( int ) current_line.find( target ) != std::string::npos );
}

// Возвращает количество встреч последовательностей в строке
int amount_in_this_string( const std::wstring& current_string, const std::wstring &target )
{
	int sum = 0;
	int cur = current_string.find( target );
	
	while( cur != std::wstring::npos )
	{
		++sum;
		cur = current_string.find( target, cur + 1 );
	}
	
	return sum;
}
int amount_in_this_string( const std::string& current_string, const std::string &target )
{
	int sum = 0;
	int cur = current_string.find( target );
	
	while( cur != std::wstring::npos )
	{
		++sum;
		cur = current_string.find( target, cur + 1 );
	}
	
	return sum;
}

// Определяет, входит ли символ в заданный промежуток
bool detect_symbol_range( wchar_t symbol, wchar_t range_front, wchar_t range_back )
{
	return ( range_front <= symbol && symbol <= range_back );
}

// Определяет, есть ли в строке символы из данного промежутка
bool detect_range_one( const std::wstring& current_string , unsigned int beginning, unsigned int ending )
{
	if( beginning > ending ) return false;
	int i, j;
	for( i = 0; i < current_string.length(); ++i ) if( current_string[i] >= beginning && current_string[i] <= ending ) return true;
	return false;
}

// Определяет, все ли символы в строке попадают в промежуток
bool detect_range_all( const std::wstring& current_string , unsigned int beginning, unsigned int ending )
{
	if( beginning > ending ) return false;
	int i, j;
	for( i = 0; i < current_string.length(); ++i ) if( current_string[i] < beginning || current_string[i] > ending ) return false;
	return true;
}

unsigned int count_words(std::wstring current_string)
{
	clear_spaces(current_string);
	return amount_in_this_string(current_string, L" ") + amount_in_this_string(current_string, L"\n") + 1;
}

bool begins_with( const std::wstring &current_line, const std::wstring &target )
{
	return target.empty() ? false : ( ( int )current_line.find( target ) == 0 );
}
bool begins_with( const std::string &current_line, const std::string &target )
{
	return target.empty() ? false : ( ( int )current_line.find( target ) == 0 );
}
bool ends_with(const std::wstring &current_line, const std::wstring &target)
{
	return target.empty() ? false : ((int)current_line.find(target) == current_line.length() - target.length());
}
bool ends_with(const std::string &current_line, const std::string &target)
{
	return target.empty() ? false : ((int)current_line.find(target) == current_line.length() - target.length());
}

bool all_symbols_too_wide( const std::wstring current_line )
{
	return detect_range_all( current_line, 0x800, 0x3FFFFFF );
}
bool all_symbols_too_wide( const std::string current_line )
{
	return all_symbols_too_wide( thin_to_wide_line( current_line ) );
}

void error_out( const std::string &current_line )
{
	const std::string yellow = "\033[1;33m";
	const std::string white  = "\033[0m";
	std::cout << yellow + current_line + white << std::endl;
	logs -> CommitConveer( F, L, current_line );
}

} // namespace Convspace
