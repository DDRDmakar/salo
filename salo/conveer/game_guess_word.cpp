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
#include "headers/database_manager.h"
#include "headers/dynamic_operation.h"
#include "headers/game_guess_word.h"
#include "headers/exception.h"

namespace Convspace
{


// Конструктор
Game_guess_word::Game_guess_word
(
	Config *transit_config,
	Community_organizer *transit_source_info,
	Database_manager *transit_dbm,
	Config_dyn *transit_dyn
)
: Dynamic_organizer(transit_config, transit_source_info, transit_dyn)
{
	dbm = transit_dbm;
}


// TUPLE:
// 0 - id, 1 - target_word, 2 - current_cells, 3 - n_mistakes, 4 - used_letters

std::wstring Game_guess_word::start( void )
{
	remove_old(); // Удаление старых записей
	
	// Достаём слово для отгадывания
	std::wstring problem = dbm -> return_query_random( L"SELECT `answer` FROM `{@database_name@}`.`story` WHERE `type` = 'game_guess_word' LIMIT " + std::to_wstring( config -> get_coefficient( L"query_limit_big" ) ) + L" ;" );
	if( problem.empty() ) return config -> get_keyword( L"game_guess_word_empty_db" );
	lowercase( problem );
	std::wstring cells = problem;
	for( int i = 0; i < cells.length(); ++ i ) cells[i] = L'#';
	
	// Создаю запись под отдельного пользователя
	std::tuple <std::string, std::wstring, std::wstring, int, std::wstring> current_game = std::make_tuple
	(
		source_info -> get_unique_id(),
		problem,
		cells,
		0,
		L""
	);
	
	add_to_list(); // Добавляю пользователя в список в оперативке (стар)
	source_info->userconfig->editable_subtree()["game"]["gamemode"] = 1; // Обозначаю принадлежность к игре в структуре юзера
	target_word.push_back( current_game );
	
	return std::wstring( config -> get_keyword( L"game_guess_word_intro" ) + get_game_state_line() );
}

std::wstring Game_guess_word::play( const std::wstring &current_line )
{
	remove_old(); // Удаление старых записей
	
	int current_index = get_current_user_index();
	if( current_index == -1 ) return config -> get_keyword( L"game_guess_word_nousr" );
	if( current_line.empty() ) return config -> get_keyword( L"game_guess_word_noword" );
	
	std::wstring current_target_line = std::get <1> ( target_word[current_index] );
	
	// Если пользователь назвал одну букву
	if( current_line.length() == 1 )
	{
		// Добавляю букву в список использованных
		if( ( int )( std::get <4> ( target_word[current_index] ) ).find( current_line ) == std::wstring::npos )
			( std::get <4> ( target_word[current_index] ) ) += current_line;
		else return config -> get_keyword( L"game_guess_word_repeating_letter" );
		
		// Заполняю буквами места, которые игрок угадал
		bool no_matches = true;
		for( int i = 0; i < current_target_line.length(); ++ i )
		{
			if( current_target_line[i] == current_line[0] )
			{
				( std::get <2> ( target_word[current_index] ) )[i] = current_line[0];
				no_matches = false;
			}
		}
		
		// Если буква не подошла
		if( no_matches )
		{
			++ ( std::get <3> ( target_word[current_index] ) );
			// Если попытки кончились
			if( ( std::get <3> ( target_word[current_index] ) ) == config -> get_coefficient( L"max_game_guess_word_mistakes" ) )
			{
				( std::get <2> ( target_word[current_index] ) ) = ( std::get <1> ( target_word[current_index] ) );
				write_down_high_score(false);
				stop();
				return config -> get_keyword( L"game_guess_word_lose" ) + L"\n" + get_cell_line();
			}
		}
		// Если всё слово угадано
		else if( ( std::get <1> ( target_word[current_index] ) ) == ( std::get <2> ( target_word[current_index] ) ) )
		{
			write_down_high_score(true);
			stop();
			
			// Return text answer
			return config -> get_keyword( L"game_guess_word_win" ) + L"\n" + get_cell_line();
		}
		
		return get_game_state_line();
	}
	
	// Если человек решил отгадать слово целиком
	else if( current_line.length() == current_target_line.length() && ( int )current_line.find( L" " ) == std::wstring::npos )
	{
		// Если угадал
		if( current_line == current_target_line ) 
		{
			( std::get <2> ( target_word[current_index] ) ) = ( std::get <1> ( target_word[current_index] ) );
			write_down_high_score(true);
			stop();
			return config -> get_keyword( L"game_guess_word_win" ) + L"\n" + get_cell_line();
		}
		// Усли не угадал
		else
		{
			++ ( std::get <3> ( target_word[current_index] ) );
			// Если попытки кончились
			if( ( std::get <3> ( target_word[current_index] ) ) == config -> get_coefficient( L"max_game_guess_word_mistakes" ) )
			{
				( std::get <2> ( target_word[current_index] ) ) = ( std::get <1> ( target_word[current_index] ) );
				write_down_high_score(false);
				stop();
				return config -> get_keyword( L"game_guess_word_lose" ) + L"\n" + get_cell_line();
			}
			
			return get_game_state_line();
		}
	}
	// Если на входе что-то другое, значит ошибка ввода
	else return config -> get_keyword( L"game_guess_word_incorrect_input" );
}

std::wstring Game_guess_word::get_game_state_line( void )
{
	int current_index = get_current_user_index();
	return std::wstring
	(
		L"\n" +
		config -> get_keyword( L"game_guess_word_current_word" ) + L" " + get_cell_line() + L"\n" +
		config -> get_keyword( L"game_guess_word_used_letters" ) + L" " + std::get <4> ( target_word[current_index] ) + L"\n" +
		config -> get_keyword( L"game_guess_word_letters" ) + L" " + std::to_wstring( ( std::get <2> ( target_word[current_index] ).length() ) ) + L"\n" +
		config -> get_keyword( L"game_guess_word_mistakes" ) + L" " + std::to_wstring( std::get <3> ( target_word[current_index] ) ) + L"/" + std::to_wstring( config -> get_coefficient( L"max_game_guess_word_mistakes" ) ) + L"\n" +
		config -> get_keyword( L"game_guess_word_additional" )
	);
}

std::wstring Game_guess_word::get_cell_line( void )
{
	int current_index = get_current_user_index();
	std::wstring output_cell_line = std::get <2> ( target_word[current_index] );
	for( int i = 0; i < output_cell_line.length(); i += 2 ) output_cell_line.insert( i, L" " );
	replace_in_text(output_cell_line, L"#", L"▒");
	
	return output_cell_line;
}

void Game_guess_word::remove_old( void )
{
	for( auto i : target_word ) if( ! is_participant( std::get <0> ( i ) ) ) remove_from_list_2( std::get <0> ( i ) );
}

int Game_guess_word::get_current_user_index( const std::string &id )
{
	for( int i = 0; i < target_word.size(); ++ i ) if( id == std::get <0> ( target_word[i] ) ) return i;
	return -1;
}
int Game_guess_word::get_current_user_index( void )
{
	return get_current_user_index( source_info -> get_unique_id() );
}

void Game_guess_word::remove_from_list_2( const std::string &id )
{
	for( int i = 0; i < target_word.size(); ++ i ) if( id == std::get <0> ( target_word[i] ) ) target_word.erase( target_word.begin() + i );
}
void Game_guess_word::remove_from_list_2( void )
{
	remove_from_list_2( source_info -> get_unique_id() );
}

void Game_guess_word::write_down_high_score(bool good_job)
{
	// [^\\](\\\\)*\{@[^\{\}@]+@\}
	try
	{
		// Update user config
		
		// If user wins
		if (good_job)
		{
			// If node exists
			if (source_info->userconfig->there_is_element("game/game_guess_word/wins"))
			{
				const std::string current_node_value = source_info->userconfig->get_keyword("game/game_guess_word/wins");
				source_info->userconfig->editable_subtree("game/game_guess_word/wins") = std::to_string(std::stoi(current_node_value) + 1);
			}
			// Create new node
			else source_info->userconfig->editable_subtree()["game"]["game_guess_word"]["wins"] = "1";
		}
		// If user loses
		else
		{
			// If node exists
			if (source_info->userconfig->there_is_element("game/game_guess_word/loses"))
			{
				const std::string current_node_value = source_info->userconfig->get_keyword("game/game_guess_word/loses");
				source_info->userconfig->editable_subtree("game/game_guess_word/loses") = std::to_string(std::stoi(current_node_value) + 1);
			}
			// Create new node
			else source_info->userconfig->editable_subtree()["game"]["game_guess_word"]["loses"] = "1";
		}
		
		// Update dynamic conveer config
		const std::string wins_value = source_info->userconfig->subtree()["game"]["game_guess_word"]["wins"] ?
			source_info->userconfig->get_keyword("game/game_guess_word/wins") : "0";
		
		if (
			good_job && (
				!dyn->there_is_element("high_scores/game_guess_word/wins") ||
				std::stoi(dyn->get_keyword("high_scores/game_guess_word/wins")) < std::stoi(wins_value)
			)
		)
		{
			dyn->editable_subtree()["high_scores"]["game_guess_word"]["wins"] = wins_value;
			dyn->editable_subtree()["high_scores"]["game_guess_word"]["loses"] =
				source_info->userconfig->there_is_element("game/game_guess_word/loses") ?
				source_info->userconfig->get_keyword("game/game_guess_word/loses") : "0";
			dyn->editable_subtree()["high_scores"]["game_guess_word"]["id"] = source_info->get_unique_id();
			dyn->editable_subtree()["high_scores"]["game_guess_word"]["name"] = source_info->get_user_first() + " " + source_info->get_user_last();
		}
	}
	catch (YAML::Exception e) { error_out("ERROR reading user or dynamic config: " + std::string(e.what())); }
	catch (std::exception e) { error_out("ERROR reading user config - std::exception"); }
}

} // namespace Convspace
