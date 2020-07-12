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
