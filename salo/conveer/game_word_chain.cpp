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

#include "headers/parse_tools.h"
#include "headers/database_manager.h"
#include "headers/dynamic_operation.h"
#include "headers/game_word_chain.h"
#include "headers/exception.h"

#define GAME_ID std::get<0>
#define GAME_CUR_WORD std::get<1>
#define GAME_USED_SET std::get<2>
#define GAME_SCORE std::get<3>

namespace Convspace
{


// Конструктор
Game_word_chain::Game_word_chain
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

std::wstring Game_word_chain::get_game_state_line( void )
{
	return
	config -> get_keyword( L"game_word_chain_myword" ) + L" " + GAME_CUR_WORD( user_info_storage[get_current_user_index()] ) + L"\n" +
	config -> get_keyword( L"game_word_chain_score") + L" " + std::to_wstring( GAME_SCORE( user_info_storage[get_current_user_index()] ) );
}

int Game_word_chain::get_current_user_index( std::string id )
{
	if( id.empty() ) id = source_info -> get_unique_id();
	for( int i = 0; i < user_info_storage.size(); ++ i ) if( id == GAME_ID( user_info_storage[i] ) ) return i;
	return -1;
}

void Game_word_chain::remove_from_list_2( std::string id )
{
	if( id.empty() ) id = source_info -> get_unique_id();
	for( int i = 0; i < user_info_storage.size(); ++ i )
	{
		if( id == GAME_ID( user_info_storage[i] ) )
		{
			user_info_storage.erase( user_info_storage.begin() + i );
			if (id == source_info->get_unique_id()) source_info->userconfig->editable_subtree()["game"]["gamemode"] = 0; // Выключаю игровой режим в структуре юзера
		}
	}
}

void Game_word_chain::remove_old( void )
{
	for( auto i : user_info_storage ) if( ! is_participant( GAME_ID( i ) ) ) remove_from_list_2( GAME_ID( i ) );
}

bool Game_word_chain::was_used( int current_game_id, std::wstring current_word )
{
	lowercase( current_word );
	for( auto e : GAME_USED_SET( user_info_storage[current_game_id] ) ) if( e == current_word ) return true;
	return false;
}

wchar_t Game_word_chain::get_last_letter( const std::wstring &current_line )
{
	if( current_line.empty() ) return L'\0';
	if( current_line.length() == 1 ) return current_line[0];
	if( std::regex_search( to_single_line(current_line), config -> get_regex( L"regex_game_word_chain_restricted_letters" ) ) )
	return current_line[ current_line.length() - 2 ];
	else return current_line.back();
}

std::wstring Game_word_chain::start( void )
{
	remove_old(); // Удаление старых записей
	
	// Достаём слово для игры в города
	std::wstring problem = dbm -> return_query_random( L"SELECT `answer` FROM `{@database_name@}`.`story` WHERE `type` = 'game_word_chain' LIMIT " + std::to_wstring( config -> get_coefficient( L"query_limit_big" ) ) + L" ;" );
	if( problem.empty() ) return config -> get_keyword( L"game_word_chain_empty_db" );
	
	std::vector <std::wstring> temp_problem_storage;
	temp_problem_storage.push_back( to_lowercase( problem ) );
	
	// Создаю запись под отдельного пользователя
	std::tuple <std::string, std::wstring, std::vector <std::wstring>, int> current_game = std::make_tuple
	(
		source_info -> get_unique_id(),
		problem,
		temp_problem_storage,
		0
	);
	
	add_to_list(); // Добавляю пользователя в список в оперативке (стар)
	source_info->userconfig->editable_subtree()["game"]["gamemode"] = 2; // Обозначаю принадлежность к игре в структуре юзера
	user_info_storage.push_back( current_game );
	
	return std::wstring( config -> get_keyword( L"game_word_chain_intro" ) + L"\n" + get_game_state_line() );
}

std::wstring Game_word_chain::play( const std::wstring &current_line )
{
	int current_index = get_current_user_index();
	if( current_index == -1 ) { stop(); return config -> get_keyword( L"game_word_chain_nousr" ); }
	if( current_line.empty() ) { stop(); return config -> get_keyword( L"game_word_chain_noword" ); }
	
	std::wstring current_line_low = to_lowercase( current_line );
	// TODO get_last_letter
	bool letters_match = get_last_letter( GAME_USED_SET( user_info_storage[ current_index ] ).back() ) == current_line_low.front();
	bool word_is_single = count_words( current_line ) == 1 || count_words( current_line ) == 2;
	bool word_was_used = was_used( current_index, current_line_low );
	
	if( !word_is_single || !letters_match ) return config -> get_keyword( L"game_word_chain_incorrect_input" );
	
	if( word_was_used ) return config -> get_keyword( L"game_word_chain_word_used" );
	else
	{
		// Если введённое пользователем слово подходит
		++ GAME_SCORE( user_info_storage[ current_index ] );
		write_down_high_score(GAME_SCORE(user_info_storage[current_index]));
		
		std::wstring next_word = get_next_word( current_index, get_last_letter( current_line_low ) );
		if( next_word.empty() )
		{
			stop();
			return config -> get_keyword( L"game_word_chain_win" ) + L"\n" + config -> get_keyword( L"game_word_chain_score" ) + L" " + std::to_wstring( GAME_SCORE( user_info_storage[ current_index ] ) );
		}
		else
		{
			GAME_CUR_WORD( user_info_storage[ current_index ] ) = next_word;
			GAME_USED_SET( user_info_storage[ current_index ] ).push_back( to_lowercase( next_word ) );
			return get_game_state_line();
		}
	}
}

std::wstring Game_word_chain::get_next_word( int current_index, wchar_t target_letter )
{
	std::wstring first_letter = L" ";
	first_letter[0] = target_letter;
	std::vector <std::wstring> available_word_storage = dbm -> DB_return_query( L"SELECT `answer` FROM `{@database_name@}`.`story` WHERE `type` = 'game_word_chain' AND `answer` LIKE '" + first_letter + L"%' LIMIT " + std::to_wstring( config -> get_coefficient( L"query_limit_big" ) ) + L" ;" );
	
	for( auto e : available_word_storage )
	{
		if( !was_used( current_index, to_lowercase( e ) ) ) return e;
	}
	return L"";
}

void Game_word_chain::write_down_high_score(const int score)
{
	try
	{
		// Manage userconfig
		
		if (
			!source_info->userconfig->there_is_element("game/game_word_chain/high_score") ||
			score > source_info->userconfig->get_coefficient("game/game_word_chain/high_score")
		) source_info->userconfig->editable_subtree()["game"]["game_word_chain"]["high_score"] = std::to_string(score);
		
		// Manage dynamic config
		
		if (
			!dyn->there_is_element("high_scores/game_word_chain/high_score") ||
			score > dyn->get_coefficient("high_scores/game_word_chain/high_score")
		)
		{
			dyn->editable_subtree()["high_scores"]["game_word_chain"]["high_score"] = std::to_string(score);
			dyn->editable_subtree()["high_scores"]["game_word_chain"]["id"] = source_info->get_unique_id();
			dyn->editable_subtree()["high_scores"]["game_word_chain"]["name"] = source_info->get_user_first() + " " + source_info->get_user_last();
		}
	}
	catch (Convspace::Exception &e) { error_out("Error writing down high score for Game_word_chain: " + e.what()); }
	catch (std::exception) { error_out("Error writing down high score for Game_word_chain. std::exception"); }
}


} // namespace Convspace
