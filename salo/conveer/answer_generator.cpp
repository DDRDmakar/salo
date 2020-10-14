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

#include <math.h>

#include "../headers/external_api.h"
#include "headers/answer_generator.h"
#include "headers/parse_tools.h"

namespace Convspace
{

// Констркутор
Answer_generator::Answer_generator
(
	Config *transit_config,
	Community_organizer *transit_source_info,
	Message_storage *transit_message,
	Database_manager *transit_dbm,
	Config_dyn *transit_dyn,
	Ban_organizer *transit_security,
	Teach_organizer *transit_sensei,
	Information *transit_information
)
{
	logs -> CommitConveer( F, L, "Object initialization - Answer_generator" );
	
	config = transit_config;
	source_info = transit_source_info;
	message = transit_message;
	dbm = transit_dbm;
	dyn = transit_dyn;
	security = transit_security;
	sensei = transit_sensei;
	information = transit_information;
	game_magic_field = Game_guess_word(config, source_info, dbm, dyn);
	game_word_chain  = Game_word_chain(config, source_info, dbm, dyn);
	
	config_version = -1;
}

// Генерировать смайл
std::string Answer_generator::make_smile( void )
{
	wchar_t smile [2] = L" ";
	smile[0] = 0x1F600 + rand() % 0x50; // Рандомный смайл emoji
	return wide_to_thin_line( smile ); // Преобразование в тонкую строку
}

// Выбрать из нескольких вариантов
std::string Answer_generator::choose_between_options( const std::wstring& current_string )
{
	std::vector <std::wstring> options_storage;
	std::wstring current_part, beginning, option_limiter;
	int res, cur;
	
	options_storage.clear();
	current_part.clear();
	beginning.clear();
	option_limiter.clear();
	
	cur = res = 0; // Текущая позиция "или" и предыдущая.
	
	option_limiter = L" " + config -> get_keyword ( L"option_limiter" );
	beginning = config -> get_group_random ( L"option_prefix" );
	
	while( cur >= 0 ) // Пока не прочитаны все части
	{
		cur = current_string.find( option_limiter, res );
		if( cur < 0 ) current_part = current_string.substr( res, current_string.length() - res ); // Вынимаем из текста отдельную часть
		else current_part = current_string.substr( res, cur - res );
		res = cur + 4;
		if( current_part.length() ) options_storage.push_back( current_part ); // Запись слова в хранилище слов
		current_part.clear(); // Очистка временного хранилища
	}
	
	// Рандомное начало + рандомный вариант
	return wide_to_thin_line( beginning )
	+ std::string ( " " )
	+ wide_to_thin_line( options_storage[ rand() % options_storage.size() ] );
}

std::string Answer_generator::mpz_get_stl_string( const mpz_t &num, const unsigned int base )
{
	if( base < 2 ) return "";
	std::string answer;
	char* m = mpz_get_str( NULL, base, num );
	answer = m;
	delete[] m;
	return answer;
}

// Разбор свойств числа
std::string Answer_generator::analyse_num( const std::string &current_line )
{
	std::string target;
	
	unsigned int base; // Определяю входное основание СС
	if( std::regex_match( current_line, std::regex( "[0-9]+" ) ) ) base = 10;
	else if( std::regex_match( current_line, std::regex( "0b[01]+" ) ) ) base = 2;
	else if( std::regex_match( current_line, std::regex( "0x[0-9a-fA-F]+" ) ) ) base = 16;
	else return config -> get_keyword( "error_incorrect_arguments" );
	
	if // Проверка на переполнение
	(
		( base == 2 && current_line.length() > 996 ) || // +2 потому что есть префиксы 0x 0b
		( base == 10 && current_line.length() > 300 ) ||
		( base == 16 && current_line.length() > 252 )
	) return config -> get_keyword( "error_big_value" );
	
	mpz_t longnum;
	mpz_init( longnum );
	// Записываю число в класс, отрезая префикс системы счисления, если это требуется
	if( base == 2 || base == 16 ) mpz_set_str( longnum, current_line.substr( 2, current_line.length() ).c_str(), base );
	else mpz_set_str( longnum, current_line.c_str(), base );
	
	// Начало анализа числа
	
	unsigned long long int current_num = 0; // Если число в диапазоне int32, то записываю его в int
	if( mpz_cmp_ui( longnum, 0xFFFFFF ) <= 0 ) current_num = mpz_get_si( longnum );
	
	std::string divisors; // Находит делители
	if( mpz_cmp_ui( longnum, 0xFFFFFF ) > 0 ) divisors = config -> get_keyword( "error_big_value" );
	else for( unsigned long long int i = 1; i <= current_num; ++i ) if( current_num % i == 0 ) divisors += std::to_string( i ) + " ";
	
	wchar_t symbol[2] = L" "; // Перевод в символ unicode
	if( current_num ) symbol[0] = current_num;
	
	// Запись конечного ответа
	target += config -> get_keyword( "part_num_symbol" ) + "	[ " + wide_to_thin_line( symbol ) + " ]\n";
	target += "Dec : " + mpz_get_stl_string( longnum, 10 ) + "\n-------------------------\n";
	target += config -> get_keyword( "part_num_divisors" ) + " " + divisors + "\n-------------------------\n";
	
	for( base = 2; base <= 12; ++ base ) target += std::to_string( base ) + " : " + mpz_get_stl_string( longnum, base ) + "\n";
	target += "16 : " + mpz_get_stl_string( longnum, 16 ) + "\n";
	target += "24 : " + mpz_get_stl_string( longnum, 24 ) + "\n";
	target += "32 : " + mpz_get_stl_string( longnum, 32 ) + "\n";
	target += "36 : " + mpz_get_stl_string( longnum, 36 ) + "\n";
	target += "62 : " + mpz_get_stl_string( longnum, 62 ) + "\n";
	
	return target;
}

// НЕ ЧЛЕН КЛАССА !!!!
// Получить строку с датой
std::string get_datetime( const std::string &form )
{
	const unsigned int l = form.length() * 3 + 10;
	std::time_t current_time = std::time( nullptr );
	
	char *mbstr = new char[l];
	std::strftime( mbstr, l, form.c_str(), std::localtime(&current_time) );
	const std::string ans = mbstr;
	delete[] mbstr;
	
	return ans;
}

// Написать текст из символов
std::wstring Answer_generator::draw_text_with_symbols( std::wstring phrase, const wchar_t *letter_sym, const wchar_t *background_sym )
{
	logs -> CommitConveer( F, L, "Rendering big text" );
	
	lowercase( phrase );
	clear_spaces( phrase );
	
	std::wstring target;
	if( config -> get_version() != config_version )
	{
		big_alphabet = config -> get_group( L"big_letters" );
		config_version = config -> get_version();
	}
	
	if (big_alphabet.empty()) return config->get_keyword(L"error_no_big_letters");
	
	wchar_t temp_symbol_line[2] = L"";
	for( auto i : phrase )
	{
		temp_symbol_line[0] = i;
		int current_position = big_alphabet[0].find( temp_symbol_line );
		if( current_position == std::wstring::npos ) return config -> get_keyword( L"error_incorrect_symbol" );
		else target += big_alphabet[ current_position + 1 ];
	}
	
	replace_from_text( target, L"0", background_sym );
	replace_from_text( target, L"1", letter_sym );
	return target.empty() ? config -> get_keyword( L"error_incorrect_symbol" ) : target;
}

// Получить справку из интернета
std::wstring Answer_generator::get_description ( const std::wstring &target )
{
	if (!( config -> get_binary( L"available_description" ) ) ) return L"";
	
	std::wstring info_line = target;
	
	std::wsmatch wiki_keyword;
	
	try { std::regex_search(info_line, wiki_keyword, config->get_regex(L"detector/what_is")); }
	catch (std::regex_error) {}
	
	if (wiki_keyword.size() != 0 && (int)info_line.find(wiki_keyword.str(0)) == 0) remove_keyword(info_line, wiki_keyword.str(0));
	if (info_line.empty()) return L"";
	
	return thin_to_wide_line(wikipedia(wide_to_thin_line(info_line)));
}

// Рандомное число
int Answer_generator::generate_random_number ( void )
{
	return ( rand() % ( config -> get_coefficient ( L"random_num" ) + 1 ) ) * pow( 10, ( rand() % ( config -> get_coefficient( L"random_num_pow" ) + 1 ) ) );
}

// Текст необычными символами
std::wstring Answer_generator::generate_fancy_text( const std::wstring &current_line )
{
	int i, j, symbol_position;
	std::vector < std::wstring > fancy_letters_storage;
	std::wstring target, frame;
	
	symbol_position = 0;
	
	target = current_line;
	lowercase ( target );
	clear_spaces ( target );
	
	fancy_letters_storage = config -> get_group ( L"fancy_letters" );
	frame = config -> get_group_random ( L"fancy_letters_frame" );
	
	for ( i = 0; i < target.size(); ++ i )
	{
		symbol_position = fancy_letters_storage [0].find ( target.substr ( i, 1 ) );
		if ( symbol_position != std::wstring::npos )
		{
			target.replace ( i, 1 , fancy_letters_storage [ symbol_position + 1 ].substr ( rand() % fancy_letters_storage [ symbol_position + 1 ].length(), 1 ) );
		}
	}
	replace_in_text ( frame, L"{@fancy_text@}", target.c_str() );
	
	return frame;
}
std::string Answer_generator::generate_fancy_text( const std::string &current_line )
{
	return wide_to_thin_line( generate_fancy_text( thin_to_wide_line( current_line ) ) );
}

// Быдлотранслит
std::wstring Answer_generator::generate_translit( const std::wstring &type, const std::wstring &current_line )
{
	std::wstring target = current_line;
	clear_spaces( target );
	
	std::vector <std::wstring> alphabet_storage = config -> get_group( type );
	if( alphabet_storage.size() < 2 || alphabet_storage[0].length() >= alphabet_storage.size() ) return target;
	
	int symbol_position = 0;
	unsigned int i = 0;
	while( i < target.length() )
	{
		symbol_position = alphabet_storage[0].find( target.substr( i, 1 ) );
		if( symbol_position != std::wstring::npos )
		{
			target.replace( i, 1 , alphabet_storage[ symbol_position + 1 ] );
			i += alphabet_storage[ symbol_position + 1 ].length();
		}
		else ++ i;
	}
	
	return target;
}
std::string Answer_generator::generate_translit( const std::string &type, const std::string &current_line )
{
	return wide_to_thin_line( generate_translit( thin_to_wide_line( type ), thin_to_wide_line( current_line ) ) );
}

bool Answer_generator::user_in_game()
{
	return game_magic_field.is_participant() || game_word_chain.is_participant() ||
	source_info->userconfig->get_keyword("game/gamemode") != "0";
}
std::string Answer_generator::play_game()
{
	if( game_magic_field.is_participant() )
	{
		message -> add_message_form( "game_guess_word" );
		return wide_to_thin_line( game_magic_field.play( message -> pure_line ) );
	}
	if( game_word_chain.is_participant() )
	{
		message -> add_message_form( "game_word_chain" );
		return wide_to_thin_line( game_word_chain.play( message -> intact_line ) );
	}
	
	// Если дошло досюда, значит произошёл сбой
	
	// Исключаю из списков игр
	game_magic_field.stop(); 
	game_word_chain.stop();
	// Выключаю игровой режим в структуре юзера
	source_info->userconfig->editable_subtree()["game"]["gamemode"] = 0;
	
	return config -> get_keyword( "game_error_not_participant" );
}

Answerbox Answer_generator::change_text_style( const Answerbox &current_answer )
{
	Answerbox answer = current_answer;
	
	switch( source_info->userconfig->get_coefficient( L"leet_type" ) )
	{
		case 1:  { answer.text = wide_to_thin_line(generate_translit(L"translit_letters", to_lowercase(thin_to_wide_line(answer.text)))); break; }
		case 2:  { answer.text = wide_to_thin_line(to_uppercase(thin_to_wide_line(answer.text)));                                         break; }
		case 3:  { answer.text = wide_to_thin_line(to_lowercase(thin_to_wide_line(answer.text)));                                         break; }
		case 4:  { answer.text = wide_to_thin_line(generate_fancy_text(thin_to_wide_line(answer.text)));                                  break; }
		case 5:  { answer.text = wide_to_thin_line(generate_translit(L"translit_sound", thin_to_wide_line(answer.text)));                 break; }
		case 6:  { answer.text = wide_to_thin_line(generate_translit(L"translit_asian", to_lowercase(thin_to_wide_line(answer.text))));   break; }
		case 7:
		{
			if(
				config->get_binary("available_text_to_speech") &&
				config->one_matches("interface_with_speech_answer_available", source_info->get_interface()) &&
				config->one_matches("message_form_with_speech_answer_available", message->message_form)
			)
			{
				answer.is_active = false;
				onCommand(
					"speechfem" + (answer.text.empty() ? "" : ( " " + answer.text )),
					false,
					source_info->get_user(),
					source_info->get_configuration(),
					source_info->get_bot_pointer(),
					source_info->get_interface_unique_identificator()
				);
				break;
			}
		}
		case 8:
		{
			if(
				config->get_binary("available_text_to_speech") &&
				config->one_matches("interface_with_speech_answer_available", source_info->get_interface()) &&
				config->one_matches("message_form_with_speech_answer_available", message->message_form)
			)
			{
				answer.is_active = false;
				onCommand(
					"speechmal" + (answer.text.empty() ? "" : ( " " + answer.text )),
					false,
					source_info->get_user(),
					source_info->get_configuration(),
					source_info->get_bot_pointer(),
					source_info->get_interface_unique_identificator()
				);
				break;
			}
		}
		default: { break; }
	}
	
	if(
		!source_info->is_conf() &&
		source_info->get_interface() == "networking_vkgroup" &&
		config->get_binary("available_bot_signature_group")
	) answer.text.insert(0, config->get_keyword("bot_signature_for_group") + " ");
	
	return answer;
}

std::wstring Answer_generator::generate_spam( const std::wstring &source_line, const int max_length )
{
	std::wstring target;
	while( target.length() + source_line.length() < max_length ) target += L" " + source_line;
	return target;
}

} // namespace Convspace
