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

#include <algorithm>
#include <iterator>
#include "headers/message_storage.h"
#include "headers/parse_tools.h"
#include "headers/exception.h"

namespace Convspace
{


// Приведение текста к стандартному виду
void Message_storage::format_phrase_punctuation( std::wstring& current_line )
{
	logs -> CommitConveer( F, L, "Formatting phrase" );
	
	if( current_line.empty() ) return;
	
	lowercase( current_line ); // Удаление заглавных
	
	replace_from_text( current_line, L"\\n", L" " );
	replace_from_text( current_line, L"\\'", L" " );
	replace_from_text( current_line, L"\n", L" " );
	current_line = split_symbol_groups( current_line );
	punctuation_line = replace_and_get_punctuation( current_line ); // Удаление пунктуации
	clear_spaces( current_line );
}

// Приведение текста к стандартному виду ( без сохранения пунктуации и удаления слов - паразитов ) - для побочных строк
void Message_storage::format_phrase( std::wstring& current_line )
{
	logs -> CommitConveer( F, L, "Formatting phrase" );
	
	if( current_line.empty() ) return;
	
	lowercase( current_line ); // Удаление заглавных
	
	replace_from_text( current_line, L"\\n", L" " );
	replace_from_text( current_line, L"\\'", L" " );
	replace_from_text( current_line, L"\n", L" " );
	current_line = split_symbol_groups( current_line );
	replace_punctuation( current_line ); // Удаление пунктуации
	clear_spaces( current_line );
}

// Поверхностное форматирование
void Message_storage::basic_format_phrase( std::wstring& current_line )
{
	logs -> CommitConveer( F, L, "Basicaly formatting phrase" );
	
	if( current_line.empty() ) return;
	
	replace_from_text( current_line, L"\\n", L"\n" );
	replace_from_text( current_line, L"\\'", L"\'" );
	clear_spaces( current_line );
}

// Разбить символы из разных групп
std::wstring Message_storage::split_symbol_groups( std::wstring current_line )
{
	logs -> CommitConveer( F, L, "Splitting \"strange\" symbols" );
	
	std::vector <std::wstring> strange_symbol_storage = config -> get_group( L"strange_symbol" );
	
	for( int i = 1; i < current_line.size(); ++ i )
	{
		std::wstring sample_1 = current_line.substr( i - 1, 1 );
		std::wstring sample_2 = current_line.substr( i, 1 );
		for( auto e : strange_symbol_storage )
		{
			// Если рядом стоит "странный" символ и обычный ( но не пробел )
			if
			(
				( appears_in_this_string( e, sample_1 ) ^ appears_in_this_string( e, sample_2 ) ) &&
				( sample_1 != L" " && sample_2 != L" " )
			)
			{
				// Вставляю между ними пробел
				current_line.insert( i, L" " );
				break;
			}
		}
	}
	
	return current_line;
}

// Разбить группу строк на подгрупы
std::vector <std::vector <std::wstring>> Message_storage::split_group_parts( const std::vector <std::wstring> &current_group )
{
	std::vector <std::vector <std::wstring>> answer;
	std::vector <std::wstring> current_answer_part;
	
	for( int i = 0; i < current_group.size(); ++ i )
	{
		if( current_group[i] == L"{@sep@}" )
		{
			if( ! current_answer_part.empty() ) answer.push_back( current_answer_part );
			current_answer_part.clear();
		}
		else
		{
			current_answer_part.push_back( current_group[i] );
			if( i + 1 >= current_group.size() ) answer.push_back( current_answer_part );
		}
		
	}
	return answer;
}

} // namespace Convspace
