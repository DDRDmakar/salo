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


#ifndef __SEQUENCE_CHECK_GCHAIN__
#define __SEQUENCE_CHECK_GCHAIN__

#include "dynamic_operation.h"
#include "database_manager.h"


namespace Convspace
{

class Game_word_chain : public Dynamic_organizer // Игра в города
{
	
private:
	
	Database_manager *dbm;
	
	// userID, current word, used words, score
	std::vector <std::tuple <std::string, std::wstring, std::vector <std::wstring>, int>> user_info_storage;
	
	std::wstring get_game_state_line( void );
	int get_current_user_index( std::string id = "" );
	void remove_from_list_2( std::string id = "" );
	bool was_used( int current_game_id, std::wstring current_word );
	std::wstring get_next_word( int current_index, wchar_t target_letter );
	wchar_t get_last_letter( const std::wstring &current_line );
	void write_down_high_score(const int score);
	
public:
	
	// Конструктор
	Game_word_chain
	(
		Config *transit_config = NULL,
		Community_organizer *transit_source_info = NULL,
		Database_manager *transit_dbm = NULL,
		Config_dyn *transit_dyn = NULL
	);
	
	std::wstring start( void );
	std::wstring play( const std::wstring &current_line );
	void remove_old( void );
	
};

} // namespace Convspace

#endif
