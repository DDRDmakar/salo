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
#include <cmath>

#include "headers/parse_tools.h"

namespace Convspace
{

std::wstring thin_to_wide_line(const std::string &current_line)
{
	// 0xxx xxxx headbyte 1 -> 0 bytes after
	// 110x xxxx headbyte 2 -> 1 bytes after
	// 1110 xxxx headbyte 3 -> 2 bytes after
	// 1111 0xxx headbyte 4 -> 3 bytes after
	// 1111 10xx headbyte 5 -> 4 bytes after
	// 1111 110x headbyte 6 -> 5 bytes after (31 bit, wchar_t limit !!!)
	// 1111 1110 headbyte 7 -> 6 bytes after (reserved)
	// 1111 1111 headbyte 8 -> 7 bytes after (reserved)
	
	// 10xx xxxx addition
	
	std::wstring target; // Итоговая строка
	unsigned wchar_t current_wchar; // Текущий символ
	unsigned int i, j, bytes_count; // счётчики, кол-во байт всего
	unsigned char sample, mask; // первый байт в последовательности и маска для него
	
	i = 0; // Прохожу по всем символам строки
	while(i < current_line.length())
	{
		sample = current_line[i];
		bytes_count = 1;
		mask = 0b01111111;
		
		while(sample & 0b10000000) // Нахожу, сколько байт требуется считать
		{
			sample <<= 1;
			mask >>= 1;
			++ bytes_count;
		}
		if(bytes_count == 2) { ++ i; continue; } // Пропускаю дополнительный байт, если начал с него
		if(bytes_count > 2) -- bytes_count;      // Компенсирую то, что 0b10xxxxxx находится на 2ой позиции
		
		current_wchar = current_line[i] & mask; // Запись первого байта
		
		for(j = 1; j < bytes_count; ++ j) // Запись остальных байт
		{
			++ i;
			if
			( // Если строка не кончилась и байт начинается на 10
				i < current_line.length() &&
				(char)(current_line[i] & 0b11000000) == (char)0b10000000
			) current_wchar = (current_wchar << 6) + (current_line[i] & 0b00111111);
			else break;
		}
		
		target.push_back(current_wchar);
		++ i;
	}
	
	return target;
}

std::string wide_to_thin_line(const std::wstring &current_line)
{
	std::string target;
	
	// 0xxx xxxx                                                              7  bit
	// 110x xxxx / 10xx xxxx                                                  11 bit
	// 1110 xxxx / 10xx xxxx / 10xx xxxx                                      16 bit
	// 1111 0xxx / 10xx xxxx / 10xx xxxx / 10xx xxxx                          21 bit
	// 1111 10xx / 10xx xxxx / 10xx xxxx / 10xx xxxx / 10xx xxxx              26 bit
	//
	// 32 bit max
	
	for( unsigned wchar_t e : current_line )
	{
		// 0    : 2^7-1
		if( e <= 0x7F )
		{
			target.push_back( e & 0b01111111 ); // 111 1111
		}
		// 2^7  : 2^11-1
		else if( e <= 0x7FF )
		{
			target.push_back( ( e & 0x7C0 ) >> 6 | 0b11000000 ); // 0x7C0 = 111 1100 0000
			target.push_back( (e & 0x3F) | 0x80 ); // 0x3F =                      11 1111
		}
		// 2^11 : 2^16-1
		else if( e <= 0xFFFF )
		{
			target.push_back( ( e & 0xF000 ) >> 12 | 0b11100000 ); // 0xF000 = 1111 0000 0000 0000
			target.push_back( ( e & 0xFC0 ) >> 6 | 0x80 ); // 0xFC0 =               1111 1100 0000
			target.push_back( (e & 0x3F) | 0x80 ); // 0x3F =                               11 1111
		}
		// 2^16 : 2^21-1
		else if( e <= 0x1FFFFF )
		{
			target.push_back( ( e & 0x1C0000 ) >> 18 | 0b11110000 ); // 0x1C0000 = 1 1100 0000 0000 0000 0000
			target.push_back( ( e & 0x3F000 ) >> 12 | 0x80 ); // 0x3F000 =             11 1111 0000 0000 0000
			target.push_back( ( e & 0xFC0 ) >> 6 | 0x80 ); // 0xFC0 =                          1111 1100 0000
			target.push_back( (e & 0x3F) | 0x80 ); // 0x3F =                                          11 1111
		}
		// 2^21 : 2^26-1
		else if( e <= 0x3FFFFFF )
		{
			target.push_back( ( e & 0x3000000 ) >> 24 | 0b11111000 ); // 0x3000000 = 11 0000 0000 0000 0000 0000 0000
			target.push_back( ( e & 0xFC0000 ) >> 18 | 0x80 ); // 0xFC0000 =            1111 1100 0000 0000 0000 0000
			target.push_back( ( e & 0x3F000 ) >> 12 | 0x80 ); // 0x3F000 =                     11 1111 0000 0000 0000
			target.push_back( ( e & 0xFC0 ) >> 6 | 0x80 ); // 0xFC0 =                                  1111 1100 0000
			target.push_back( (e & 0x3F) | 0x80 ); // 0x3F =                                                  11 1111
		}
		// 2^26 : 2^31-1 - wchar_t limit !!!
		else if( e <= 0x7FFFFFFF )
		{
			target.push_back( ( e & 0x40000000 ) >> 30 | 0b11111100 ); // 0x40000000 = 100 0000 0000 0000 0000 0000 0000 0000
			target.push_back( ( e & 0x3F000000 ) >> 24 | 0x80 ); // 0x3F000000 =        11 1111 0000 0000 0000 0000 0000 0000
			target.push_back( ( e & 0xFC0000 ) >> 18 | 0x80 ); // 0xFC0000 =                    1111 1100 0000 0000 0000 0000
			target.push_back( ( e & 0x3F000 ) >> 12 | 0x80 ); // 0x3F000 =                             11 1111 0000 0000 0000
			target.push_back( ( e & 0xFC0 ) >> 6 | 0x80 ); // 0xFC0 =                                          1111 1100 0000
			target.push_back( (e & 0x3F) | 0x80 ); // 0x3F =                                                          11 1111
		}
		
		else continue;
	}
	
	return target;
}

// Перевод вектора тонких строк в вектор толстых строк
std::vector <std::wstring> thin_to_wide_storage( const std::vector <std::string> &current_storage )
{
	std::vector <std::wstring> answer;
	for( auto i : current_storage ) answer.push_back( thin_to_wide_line( i ) );
	return answer;
}

// Перевод вектора толстых строк в вектор тонких строк
std::vector <std::string> wide_to_thin_storage( const std::vector <std::wstring> &current_storage )
{
	std::vector <std::string> answer;
	for( auto i : current_storage ) answer.push_back( wide_to_thin_line( i ) );
	return answer;
}

// Разбивает текст на отдельные части, разделённые данным символом ( последовательностью символов );
std::vector <std::wstring> disassemble ( const std::wstring &current_line, const wchar_t *limiter )
{
	std::wstring current_part;
	std::vector <std::wstring> current_storage;
	int reserve, space;
	space = reserve = 0;
	
	if ( current_line.empty() || wcslen ( limiter ) == 0 ) return current_storage;
	
	while ( space != std::wstring::npos ) // Пока не прочитаны все части
	{
		space = current_line.find ( limiter, reserve );
		if ( space == std::wstring::npos ) current_part = current_line.substr ( reserve, current_line.length() ); // Вынимаем из текста отдельное слово
		else current_part = current_line.substr ( reserve, space - reserve );
		reserve = space + wcslen ( limiter ); // Сдвиг позиции начала поиска
		if ( current_part.empty() ) continue;
		if ( ! current_part.empty() ) current_storage.push_back ( current_part ); // Запись части в хранилище
		current_part.clear(); // Очистка временного хранилища
	}
	
	return current_storage;
}

// Разбивает текст на отдельные слова
std::vector <std::wstring> disassemble_words( std::wstring current_line )
{
	std::wstring current_word;
	std::vector <std::wstring> current_storage;
	int reserve, space;
	space = reserve = 0;
	
	clear_spaces( current_line );
	
	while( space != std::wstring::npos ) // Пока не прочитаны все слова
	{
		space = current_line.find( L" ", reserve );
		if( space == std::wstring::npos ) current_word = current_line.substr( reserve, current_line.length() - reserve ); // Вынимаем из текста отдельное слово
		else current_word = current_line.substr( reserve, space - reserve );
		reserve = space + 1; // Сдвиг позиции пробела
		if( current_word.empty() ) continue;
		clear_spaces( current_word );
		if( ! current_word.empty() ) current_storage.push_back( current_word ); // Запись слова в хранилище слов
		current_word.clear(); // Очистка временного хранилища
	}
	
	return current_storage;
}

// Рзабивает текст на отдельные предложения
std::vector <std::wstring> disassemble_sentences( std::wstring current_line )
{
	std::wstring current_part;
	std::vector <std::wstring> current_storage;
	int reserve, space;
	space = reserve = 0;
	
	if( current_line.empty() ) return current_storage;
	
	replace_from_text( current_line, L"?", L"." );
	replace_from_text( current_line, L"!", L"." );
	replace_from_text( current_line, L",", L"." );
	replace_from_text( current_line, L"..", L"." );
	
	if( current_line.front() == L'.' ) { current_line.erase( current_line.begin() ); if( current_line.empty() ) return current_storage; }
	if( current_line.back() == L'.' ) { current_line.pop_back(); if( current_line.empty() ) return current_storage; }
	clear_spaces( current_line );
	if( current_line.empty() ) return current_storage;
	
	while( space != std::wstring::npos ) // Пока не прочитаны все части
	{
		space = current_line.find( L".", reserve );
		if( space == std::wstring::npos ) current_part = current_line.substr( reserve, current_line.length() - reserve ); // Вынимаем из текста отдельную часть
		else current_part = current_line.substr( reserve, space - reserve );
		reserve = space + 1; // Сдвиг позиции разделителя
		clear_spaces( current_part );
		if( current_part.empty() ) continue;
		
		if( current_part.front() == L'.' ) current_part.erase( current_part.begin() );
		if( current_part.back() == L'.' ) current_part.pop_back();
		clear_spaces( current_part );
		
		if( ! current_part.empty() ) current_storage.push_back( current_part ); // Запись в хранилище
		current_part.clear(); // Очистка временного хранилища
	}
	
	return current_storage;
}

// Разделяет текст на сочетания слов от начала по возрастанию
std::vector <std::wstring> disassemble_pyramide( std::wstring current_line )
{
	std::wstring current_part;
	std::vector <std::wstring> current_storage;
	int reserve, space;
	space = reserve = 0;
	
	clear_spaces( current_line );
	
	while( space != std::wstring::npos ) // Пока не прочитаны все части
	{
		space = current_line.find( L" ", reserve );
		if( space == std::wstring::npos ) current_part = current_line; // Вынимаем из текста отдельную часть
		else current_part = current_line.substr( 0, space );
		reserve = space + 1; // Сдвиг позиции разделителя
		if( current_part.empty() ) continue;
		
		clear_spaces( current_part );
		
		if( ! current_part.empty() ) current_storage.push_back( current_part ); // Запись в хранилище
		current_part.clear(); // Очистка временного хранилища
	}
	
	return current_storage;
}

void lowercase( std::wstring &current_line ) // Удаляет заглавные буквы
{
	for( int i = 0; i < current_line.length(); ++i )
	{
		if( current_line [i] >= L'A' && current_line [i] <= L'Z') current_line [i] += L'z' - L'Z';
		if( current_line [i] >= L'А' && current_line [i] <= L'Я') current_line [i] += L'я' - L'Я';
		if( current_line [i] == L'Ё' ) current_line [i] = L'ё';
	}
}
std::wstring to_lowercase( std::wstring current_line )
{
	lowercase( current_line );
	return current_line;
}
std::string to_lowercase( std::string current_line )
{
	return wide_to_thin_line( to_lowercase( thin_to_wide_line( current_line ) ) );
}

void uppercase( std::wstring &current_line ) // Удаляет прописные буквы
{
	for( int i = 0; i < current_line.length(); ++i )
	{
		if( current_line [i] >= L'a' && current_line [i] <= L'z') current_line [i] -= L'z' - L'Z';
		if( current_line [i] >= L'а' && current_line [i] <= L'я') current_line [i] -= L'я' - L'Я';
		if( current_line [i] == L'ё' ) current_line [i] = L'Ё';
	}
}
std::wstring to_uppercase( std::wstring current_line )
{
	uppercase( current_line );
	return current_line;
}
std::string to_uppercase( std::string current_line )
{
	return wide_to_thin_line( to_uppercase( thin_to_wide_line( current_line ) ) );
}

// Сортирует слова в хранилище по длине ( по убыванию )
std::vector <std::wstring> sort_words( std::vector <std::wstring> current_storage )
{
	std::sort(current_storage.begin(), current_storage.end(), [](std::wstring a, std::wstring b) { return a.length() > b.length(); });
	
	return current_storage;
}

void unscreen( std::string &current_line ) // Убирает экранированные символы
{
	// ' " () [] <> *!@#$%^&№?_+- .,;:
	
//	replace_from_text( current_line, "\\n", "\n" );
//	replace_from_text( current_line, "\'", " " );
	replace_from_text( current_line, "`", " " );
//	replace_from_text( current_line, "\"", " " );
	
	// ---------------------------------------------
	
//	replace_from_text( current_line, "\\" );
	
	clear_spaces( current_line );
}

std::string screen_sql ( const std::wstring &current_line )
{
	std::string answer = wide_to_thin_line( current_line );
	
	answer = to_safestring_16(answer);
	
//	replace_from_text ( answer, "\\n", "\n" );
	replace_in_text ( answer, "\'", "\'\'" );
	replace_in_text ( answer, "\\", "\\\\" );
	
	return answer;
}
std::string screen_sql ( const std::string &current_line )
{
	return screen_sql ( thin_to_wide_line ( current_line ) );
}

std::wstring filter_SQL( std::wstring current_line )
{
	replace_in_text( current_line, L"`" );
	return current_line;
}
std::string filter_SQL( const std::string &current_line )
{
	return wide_to_thin_line( filter_SQL( thin_to_wide_line( current_line ) ) );
}

std::vector <std::wstring> split_args( const std::wstring &source_line )
{
	std::vector <std::wstring> target;
	std::wstring current_line;
	bool quote = false;
	bool escaped = false;
	
	for( const auto e : source_line )
	{
		if( e == L'\"' && !escaped )
		{
			if( !current_line.empty() ) target.push_back( current_line );
			current_line.clear();
			quote = !quote;
			continue;
		}
		
		if( e == L'n' && escaped )
		{
			current_line.push_back( L'\n' );
			escaped = false;
			continue;
		}
		
		if( e == L'\\' && !escaped ) { escaped = true; continue; }
		else escaped = false;
		
		if( !quote && !escaped && ( e == L' ' || e == L'\n' ) )
		{
			if( !current_line.empty() ) target.push_back( current_line );
			current_line.clear();
			continue;
		}
		
		current_line.push_back( e );
	}
	if( !current_line.empty() ) target.push_back( current_line );
	
	return target;
}

// Удалить из текста переносы строк
std::wstring to_single_line(std::wstring current_line)
{
	replace_from_text(current_line, L"\n", L" ");
	return current_line;
}
std::string to_single_line(std::string current_line)
{
	replace_from_text(current_line, "\n", " ");
	return current_line;
}

// Перевод текста в HEX суррогаты и обратно
std::wstring to_safestring(std::wstring current_line, uint8_t maxbit)
{
	uint32_t maxbit_value = pow(2, maxbit) - 1;
	
	replace_in_text(current_line, L"\\x", L"\\\\x");
	
	for (int i = 0; i < current_line.length(); ++i)
	{
		if (current_line[i] > maxbit_value)
		{
			std::wstringstream current_surrogate;
			current_surrogate << std::hex << (uint32_t)current_line[i];
			std::wstring current_surrogate_line = current_surrogate.str();
			for (int j = 0; j < current_surrogate_line.length(); ++j)
			{
				const wchar_t e = current_surrogate_line[j];
				if (
					!detect_symbol_range(e, L'0', L'9') &&
					!detect_symbol_range(e, L'a', L'f') &&
					!detect_symbol_range(e, L'A', L'F')
				) current_surrogate_line.erase(current_surrogate_line.begin() + j--);
			}
			current_line.replace(i, 1, L"\\x" + current_surrogate_line + L";");
			i += current_surrogate_line.length() + 2;
		}
	}
	
	return current_line;
}
std::string to_safestring(std::string current_line, uint8_t maxbit)
{
	return wide_to_thin_line(to_safestring(thin_to_wide_line(current_line), maxbit));
}
// Специальный вызов этой функции для двухбайтных символов (<= 0x7FF)
std::wstring to_safestring_16(std::wstring current_line) { return to_safestring(current_line, 11); }
std::string  to_safestring_16(std::string  current_line) { return to_safestring(current_line, 11); }

std::wstring from_safestring(std::wstring current_line)
{
	bool escaped = false;
	std::wstring current_surrogate_code;
	
	for(int i = 0; i < current_line.length(); ++ i)
	{
		if (current_line[i] == L'\\')
		{
			if (escaped) 
			{
				current_line.erase(current_line.begin() + i);
				escaped = false;
			}
			else escaped = true;
			
			continue;
		}
		
		if (escaped && current_line[i] == L'x')
		{
			unsigned int surrogate_position = i - 1;
			wchar_t ss = current_line[++i]; // Current surrogate symbol
			
			while (
				detect_symbol_range(ss, L'0', L'9') ||
				detect_symbol_range(ss, L'a', L'f') ||
				detect_symbol_range(ss, L'A', L'F')
			)
			{
				current_surrogate_code.push_back(ss);
				ss = current_line[++i];
			}
			
			if (ss == L';')
			{
				std::wstring insertation_line;
				if (!current_surrogate_code.empty()) insertation_line.push_back((wchar_t)std::wcstoul(current_surrogate_code.c_str(), nullptr, 16));
				current_line.replace(surrogate_position, current_surrogate_code.length() + 3, insertation_line);
				i -= (current_surrogate_code.length() + 3 - insertation_line.length());
			}
			current_surrogate_code.clear();
		}
		
		escaped = false;
	}
	
	return current_line;
}
std::string from_safestring(std::string current_line)
{
	return wide_to_thin_line(from_safestring(thin_to_wide_line(current_line)));
}

} // namespace Convspace
