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

#include "headers/message_storage.h"
#include "headers/parse_tools.h"

namespace Convspace
{


// Определяет тип сообщения
std::string Message_storage::detect_message_form( void )
{
	int i, j;
	int possibility_t, possibility_f;
	std::vector<std::wstring> keyword_storage;
	
	keyword_storage.clear();
	logs->CommitConveer(F, L, "Detecting message form");
	
	if ( pure_line.empty() ) return "";
	
	try 
	{
	// Repeat
		if (
			config->get_possibility(L"poss_repeat") &&
			std::regex_match(to_single_line(pure_line), config->get_regex(L"detector/repeat"))
		) return "repeat";
		
	// Possibility
		if (
			config->get_possibility(L"poss_poss") &&
			std::regex_match(to_single_line(pure_line), config->get_regex(L"detector/possibility"))
		) return "likelihood";
		
	// Evaluate
		if (
			config->get_possibility(L"poss_evaluate") &&
			std::regex_match(to_single_line(pure_line), config->get_regex(L"detector/evaluate"))
		) return "evaluate";
		
	// Choice
		if (
			config->get_possibility( L"poss_choice" ) &&
			word_exists(word_storage, config->get_keyword(L"option_limiter").c_str())
		) return "choice";
		
	// Number
		if (
			config->get_possibility(L"poss_number") &&
			std::regex_match(intact_line, config->get_regex(L"regex_full_single_integer"))
		) return "number";
		
	// Mathe
		if (config->get_possibility(L"poss_mathe"))
		{
			if ( detect_mathe ( intact_line ) ) return "mathe";
		}
		
	// Reason
		if (
			config->get_possibility(L"poss_reason") &&
			std::regex_match(to_single_line(pure_line), config->get_regex(L"detector/reason"))
		) return "?_reason";
		
	// Person
		if (
			config->get_possibility(L"poss_person") &&
			std::regex_match(to_single_line(pure_line), config->get_regex(L"detector/person"))
		) return "person";
		
	// Count
		if (
			config->get_possibility(L"poss_count") &&
			std::regex_match(to_single_line(pure_line), config->get_regex(L"detector/count"))
		) return "x_count";
		
	// Time
		if (
			config->get_possibility(L"poss_time") &&
			std::regex_match(to_single_line(pure_line), config->get_regex(L"detector/time"))
		) return "time";
		
	// Location
		if (
			config->get_possibility(L"poss_location") &&
			std::regex_match(to_single_line(pure_line), config->get_regex(L"detector/time"))
		) return "where";
		
	// Solution
		if (
			config->get_possibility(L"poss_solution") &&
			std::regex_match(to_single_line(pure_line), config->get_regex(L"detector/solution")) &&
			appears_in_this_string(punctuation_line, L"?")
		) return "opinion";
		
	// Boolean
		if (config->get_possibility(L"poss_boolean"))
		{
			if(  dbm -> detect_questionword( *this ) == L"?" ) return "boolean";
		}
		
	// 1F600 - 1F64F Эмоджи (смайлы)
		if (
			config->get_possibility(L"poss_smile") &&
			detect_range_one(intact_line, 0x1F600, 0x1F64F) &&
			pure_line.length() <= config->get_coefficient("max_len_detect_smile")
		) return "smile";
		
	// Wiki
		if (
			(
				config -> get_possibility( L"poss_wiki" ) &&
				std::regex_match(to_single_line(pure_line), std::wregex(config->get_keyword(L"detector/what_is") + L"[^ ][\\w\\W]*"))
			) ||
			(
				(word_storage.size() == 1) &&
				config->get_possibility(L"poss_single_word_wiki")
			)
		) return "info";
		
	}
	catch (std::regex_error) { error_out("Error while detecting message form - incorrect regex"); }
	
	
	return "";
}

// Определение в сообщении математического примера
bool Message_storage::detect_mathe( std::wstring current_string )
{
	int i, j;
	int numbers, operators, o_brackets, c_brackets;
	std::vector < std::wstring > keyword_storage;
	
	numbers = operators = o_brackets = c_brackets = 0;
	keyword_storage.clear();
	
	keyword_storage = config -> get_group( L"mathe_prefix" );
	for ( i = 0; i < keyword_storage.size(); ++ i ) remove_keyword ( current_string, keyword_storage [i].c_str() );
	
	if( current_string.length() == 0 ) return false;
	
	o_brackets += amount_in_this_string( current_string, L"(" );
	c_brackets += amount_in_this_string( current_string, L")" );
	
	operators += amount_in_this_string( current_string, L"(" );
	operators += amount_in_this_string( current_string, L")" );
	operators += amount_in_this_string( current_string, L"+" );
	operators += amount_in_this_string( current_string, L"-" );
	operators += amount_in_this_string( current_string, L"*" );
	operators += amount_in_this_string( current_string, L"/" );
	operators += amount_in_this_string( current_string, L"^" );
	operators += amount_in_this_string( current_string, L"%" );
	operators += amount_in_this_string( current_string, L"log" );
	operators += amount_in_this_string( current_string, L"ln" );
	operators += amount_in_this_string( current_string, L"lg" );
	operators += amount_in_this_string( current_string, L"sqrt" );

	numbers += amount_in_this_string( current_string, L"0" );
	numbers += amount_in_this_string( current_string, L"1" );
	numbers += amount_in_this_string( current_string, L"2" );
	numbers += amount_in_this_string( current_string, L"3" );
	numbers += amount_in_this_string( current_string, L"4" );
	numbers += amount_in_this_string( current_string, L"5" );
	numbers += amount_in_this_string( current_string, L"6" );
	numbers += amount_in_this_string( current_string, L"7" );
	numbers += amount_in_this_string( current_string, L"8" );
	numbers += amount_in_this_string( current_string, L"9" );
	
	return
	(
		( ! current_string.empty() ) &&
		( operators + numbers > current_string.length() / 2 ) &&
		( operators > 0 && numbers > 0 ) &&
		( o_brackets == c_brackets )
	);
	
	// Если в сообщении чисел и операторов больше, чем половина текста, значит пример существует
}

// Определить словосочетание в строке
bool Message_storage::detect_word_set ( const std::wstring &target )
{
	if( target.empty() ) return false;
	if( count_words( target ) == 1 ) return word_exists( word_storage, target );
	else return appears_in_this_string( pure_line, target );
}

// Определить словосочетание в начале фразы
bool Message_storage::detect_word_set_begin( const std::wstring &target )
{
	if( target.empty() ) return false;
	if( count_words( target ) == 1 ) return word_exists( word_storage, target, 0 );
	else return( ( int ) pure_line.find( target ) == 0 );
}

// Проверка возможность/невозможность анализа сообщения
bool Message_storage::dont_dig_history( void )
{
	logs -> CommitConveer( F, L, "Checking, dont_dig_history" );
	return config -> one_matches( "type_dont_dig_history", message_form );
}
bool Message_storage::dont_dig_history_with_attachment( void )
{
	logs -> CommitConveer( F, L, "Checking, dont_dig_history_with_attachment" );
	return ( ! source_info -> get_attachment().empty() ) && config -> one_matches( "type_dont_dig_history_with_attachment", message_form );
}
bool Message_storage::dont_parse_before_typecheck( void )
{
	logs -> CommitConveer( F, L, "Checking, dont_parse_before_typecheck" );
	return config -> one_matches( "type_dont_parse_before_typecheck", message_form );
}
bool Message_storage::dont_parse_before_typecheck_with_attachment( void )
{
	logs -> CommitConveer( F, L, "Checking, dont_parse_before_typecheck_with_attachment" );
	return ( ! source_info -> get_attachment().empty() ) && config -> one_matches( "type_dont_parse_before_typecheck_with_attachment", message_form );
}


} // namespace Convspace
