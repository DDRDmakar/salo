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


#ifndef __SEQUENCE_CHECK_PARSE_TOOLS__
#define __SEQUENCE_CHECK_PARSE_TOOLS__

#include <iostream>
#include <vector>
#include <string>
#include <wchar.h>
#include "file.h"

using namespace std;

namespace Convspace
{

// Format:
std::string wide_to_thin_line ( const std::wstring &current_line );
std::wstring thin_to_wide_line ( const std::string &current_line );
std::vector <std::wstring> thin_to_wide_storage( const std::vector <std::string> &current_storage );
std::vector <std::string> wide_to_thin_storage( const std::vector <std::wstring> &current_storage );
std::vector <std::wstring> disassemble ( const std::wstring &current_line, const wchar_t *limiter );
std::vector <std::wstring> disassemble_words( std::wstring current_line );
std::vector <std::wstring> disassemble_sentences( std::wstring current_line );
std::vector <std::wstring> disassemble_pyramide( std::wstring current_line );
void lowercase( std::wstring &current_line );
void uppercase( std::wstring &current_line );
std::wstring to_lowercase( std::wstring current_line );
std::string to_lowercase( std::string current_line );
std::wstring to_uppercase( std::wstring current_line );
std::string to_uppercase( std::string current_line );
std::vector <std::wstring> sort_words( std::vector <std::wstring> current_storage );
void unscreen( std::string &current_line );
std::string screen_sql ( const std::wstring &current_line );
std::string screen_sql ( const std::string &current_line );
std::wstring filter_SQL( std::wstring current_line );
std::string filter_SQL( const std::string &current_line );
std::vector <std::wstring> split_args( const std::wstring &source_line );
std::wstring to_single_line(std::wstring current_line); // Удалить из текста переносы строк
std::string to_single_line(std::string current_line);
std::wstring to_safestring(std::wstring current_line, uint8_t maxbit);
std::string to_safestring(std::string current_line, uint8_t maxbit);
std::wstring to_safestring_16(std::wstring current_line);
std::string  to_safestring_16(std::string  current_line);
std::wstring from_safestring(std::wstring current_line);
std::string from_safestring(std::string current_line);

// Remove:
void replace_from_text( std::wstring &current_line, const std::wstring &part, const std::wstring &n_part = L"", std::wstring *current_punctuation_string = NULL );
void replace_from_text( std::string &current_line, const std::string &part, const std::string &n_part = "", std::string *current_punctuation_string = NULL );
void replace_in_text( std::wstring &current_line, const std::wstring &part, const std::wstring &n_part = L"", std::wstring *current_punctuation_string = NULL );
void replace_in_text( std::string &current_line, const std::string &part, const std::string &n_part = "", std::string *current_punctuation_string = NULL );

void remove_keyword( std::wstring &current_line, const std::wstring &target = L"" );
void remove_keyword( std::string &current_line, const std::string &target = "" );

void clear_spaces( std::wstring& current_string );
void clear_spaces( std::string& current_string );

// Detect:
bool detect_punctuation( wchar_t symbol ); // Проверяет, принадлежит ли символ к символам пунктуации
bool appears_in_this_string( const std::wstring& current_line, const std::wstring &target );
bool appears_in_this_string( const std::string& current_line, const std::string &target );
int amount_in_this_string( const std::wstring& current_string, const std::wstring &target );
int amount_in_this_string( const std::string& current_string, const std::string &target );
int word_exists( const std::vector <std::wstring> &current_storage, const std::wstring &target );
bool word_exists( const std::vector <std::wstring> &current_storage, const std::wstring &target, int position );
bool detect_symbol_range( wchar_t symbol, wchar_t range_front, wchar_t range_back );
bool detect_range_one( const std::wstring& current_string , unsigned int beginning, unsigned int ending );
bool detect_range_all( const std::wstring& current_string , unsigned int beginning, unsigned int ending );
unsigned int count_words(std::wstring current_string);
bool begins_with( const std::wstring &current_line, const std::wstring &target );
bool begins_with( const std::string &current_line, const std::string &target );
bool ends_with(const std::wstring &current_line, const std::wstring &target);
bool ends_with(const std::string &current_line, const std::string &target);
bool all_symbols_too_wide( const std::wstring current_line ); // Проверяет, все ли символы больше 2 байт мультибайта
bool all_symbols_too_wide( const std::string current_line ); // Зависит от базы данных, которая используется

void error_out( const std::string &current_line ); // Sends error to logs and console

} // namespace Convspace

#endif
