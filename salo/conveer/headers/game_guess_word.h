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


#ifndef __SEQUENCE_CHECK_GGUESS__
#define __SEQUENCE_CHECK_GGUESS__

#include "dynamic_operation.h"
#include "community.h"
#include "database_manager.h"
#include "config.h"
#include "config_dyn.h"


namespace Convspace
{

class Game_guess_word : public Dynamic_organizer // Игра "отгадай слово"
{
	
private:
	
	Database_manager *dbm;
	
	// id, target_word, current_cells, n_mistakes, used_letters
	std::vector <std::tuple <std::string, std::wstring, std::wstring, int, std::wstring>> target_word;
	
	std::wstring get_game_state_line( void );
	std::wstring get_cell_line( void );
	int get_current_user_index( const std::string &id );
	int get_current_user_index( void );
	void remove_from_list_2( const std::string &id );
	void remove_from_list_2( void );
	
public:
	
	// Конструктор
	Game_guess_word
	(
		Config *transit_config = NULL,
		Community_organizer *transit_source_info = NULL,
		Database_manager *transit_dbm = NULL,
		Config_dyn *transit_dyn = NULL
	);

	
//	void add_basic_data_2( std::wstring &transit_phrase );
	std::wstring start( void );
	std::wstring play( const std::wstring &current_line );
	void remove_old( void );
	void write_down_high_score(bool good_job);
};

} // namespace Convspace

#endif
