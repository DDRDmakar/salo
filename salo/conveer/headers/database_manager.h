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


#ifndef __SEQUENCE_CHECK_DBMANAGER__
#define __SEQUENCE_CHECK_DBMANAGER__

#include "config.h"
#include "answerbox.h"
#include "message_storage.h"
#include "answer_generator.h"
#include "community.h"

namespace Convspace
{

class Database_manager
{
	
private:
	
	Config *config;
	Community_organizer *source_info;
	Answer_generator *generator;
	History_organizer *historian;
	Message_storage *message;
	
	std::string check_word ( const std::vector <std::wstring> &current_storage, const unsigned int words_amount );
	std::string check_zero_word ( const std::vector <std::wstring> &current_storage );
	std::string check_single_word ( const std::vector <std::wstring> &current_storage );
	std::string check_double_word ( const std::vector <std::wstring> &current_storage );
	std::string check_triple_word ( const std::vector <std::wstring> &current_storage );
	std::string get_typical_answer ( const char* target );
	
public:
	
	Database_manager
	(
		Config *transit_config = NULL,
		Community_organizer *transit_source_info = NULL,
		Message_storage *transit_message = NULL,
		History_organizer *transit_historian = NULL,
		Answer_generator *transit_generator = NULL
	);
	
	// Database functions
	std::vector <std::string> DB_return_query(std::string query_line);
	std::vector <std::wstring> DB_return_query(const std::wstring &query_line);
	void DB_simple_query(std::string query_line);
	void DB_simple_query(const std::wstring &query_line);
	std::string return_query_one( const std::string &query_line );
	std::wstring return_query_one( const std::wstring &query_line );
	std::string return_query_random( const std::string &target );
	std::wstring return_query_random( const std::wstring &target );
	int return_query_count( const std::string &query_line );
	int return_query_count( const std::wstring &query_line );
	std::string return_query_list( const std::string &query_line );
	std::wstring return_query_list( const std::wstring &query_line );
	
	// Typical answers
	std::string check_person ( const std::wstring &current_string );
	std::string check_person ( void );
	std::string check_time ( const std::wstring &current_string );
	std::string check_place ( const std::wstring &current_string );
	std::string check_place ( void );
	std::string explain_reason ( void );
	std::string explain_solution ( void );
	std::string answer_empty_message ( void );
	std::string generate_confidence_level ( void );
	std::string evaluate ( void );
	std::string answer_not_found( void );
	
	// Others
	std::string drill_message ( const int available );
	void save_unsolved_phrase ( const std::string &current_string );
	std::wstring detect_questionword ( const Message_storage &current_storage );
	std::string get_user_name ( void );
	std::string find_answer_in_history ( const std::wstring &current_line );
	std::string find_answer_in_history ( const std::string &current_line );
	Answerbox get_attachment( const std::string &type, const std::string &type_sub = "" ); // Вытаскивает вложение из базы данных
	std::string fetch_text( const std::string &type, const std::string &type_sub = "" ); // Вытаскивает текст из базы данных
	void recover_user(const std::string &id); // Восстановление пользователя из резервной базы, если он там находится
	
};

} // namespace Convspace

#endif
