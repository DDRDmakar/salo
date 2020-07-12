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
