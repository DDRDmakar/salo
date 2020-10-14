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


#ifndef __SEQUENCE_CHECK_ANSWERGEN__
#define __SEQUENCE_CHECK_ANSWERGEN__

#include <gmpxx.h>
#include <string>
#include "game_guess_word.h"
#include "game_word_chain.h"
#include "config.h"
#include "config_dyn.h"
#include "database_manager.h"
#include "message_storage.h"
#include "ban.h"
#include "teach.h"
#include "information.h"
#include "community.h"

namespace Convspace
{

class Answer_generator
{
	
private:
	
	Config *config;
	Community_organizer *source_info;
	Message_storage *message;
	Database_manager *dbm;
	Config_dyn *dyn;
	Ban_organizer *security;
	Teach_organizer *sensei;
	Information *information;
	Game_guess_word game_magic_field;
	Game_word_chain game_word_chain;
	
	int config_version;
	std::list <std::pair <std::wstring, std::wstring>> capsule;
	std::vector <std::wstring> big_alphabet;
	
	std::string mpz_get_stl_string( const mpz_t &num, const unsigned int base );
	Answerbox analyse_cmd_internal( const std::wstring &cmd, const std::wstring &current_line, const std::vector <std::wstring> &current_storage );
	
public:
	
	Answer_generator // Констркутор
	(
		Config *transit_config = NULL,
		Community_organizer *transit_source_info = NULL,
		Message_storage *transit_message = NULL,
		Database_manager *transit_dbm = NULL,
		Config_dyn *transit_dyn = NULL,
		Ban_organizer *transit_security = NULL,
		Teach_organizer *transit_sensei = NULL,
		Information *transit_information = NULL
	);
	
	std::string make_smile( void );
	std::string choose_between_options( const std::wstring& current_string );
	std::string analyse_num( const std::string &current_line );
	std::wstring draw_text_with_symbols( const std::wstring phrase, const wchar_t *letter_sym, const wchar_t *background_sym );
	std::wstring get_description ( const std::wstring &target );
	int generate_random_number ( void );
	std::wstring generate_fancy_text ( const std::wstring &current_line );
	std::string generate_fancy_text ( const std::string &current_line );
	std::wstring generate_translit ( const std::wstring &type, const std::wstring &current_line );
	std::string generate_translit( const std::string &type, const std::string &current_line );
	Answerbox change_text_style( const Answerbox &current_answer );
	std::wstring generate_spam( const std::wstring &source_line, const int max_length ); // Набивает строку данным текстом до достижения максимального объёма
	
	// Ответ на вложения
	Answerbox answer_attachment( const std::string &current_attachment );
	
	// Ответ на команды
	bool detect_cmd( const std::vector <std::wstring> &current_storage );
	Answerbox analyse_cmd( void );
	Answerbox get_help( const std::wstring &key );
	std::string call_console(const std::wstring &cmd);
	void changeuserstatus(const std::string &id, const unsigned int status);
	std::string normalize_id(std::string id);
	Answerbox get_media(const std::string &current_path, const size_t keynum); // Get attachment from media node in config
	std::string get_text(const std::string &current_path, const size_t keynum); // Get text from config
	
	// Проверка состояния игр
	bool user_in_game();
	std::string play_game();
	
};


std::string get_datetime( const std::string &form );


} // namespace

#endif
