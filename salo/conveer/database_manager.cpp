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

#include "headers/database_manager.h"
#include "headers/parse_tools.h"
#include "headers/history.h"

namespace Convspace
{

Database_manager::Database_manager
(
	Config *transit_config,
	Community_organizer *transit_source_info,
	Message_storage *transit_message,
	History_organizer *transit_historian,
	Answer_generator *transit_generator
)
{
	logs -> CommitConveer( F, L, "Object initialization - Database_manager" );
	
	config = transit_config;
	source_info = transit_source_info;
	historian = transit_historian;
	generator = transit_generator;
	message = transit_message;
}

//==============[ QUERIES ]==============//


// Функция для обращения к базе данных
std::vector<std::string> Database_manager::DB_return_query(std::string query_line)
{
	replace_in_text(query_line, "{@database_name@}", config->get_keyword("database_name"));
	std::vector<std::string> query_result = (database->database_returnQuery(to_safestring_16(query_line))).RowStrings; // Запрос к базе
	for (auto &e : query_result) e = from_safestring(e);
	return query_result;
}
std::vector<std::wstring> Database_manager::DB_return_query(const std::wstring &query_line)
{
	return thin_to_wide_storage(DB_return_query(wide_to_thin_line(query_line)));
}
// Без возвращаемого результата
void Database_manager::DB_simple_query(std::string query_line)
{
	replace_in_text(query_line, "{@database_name@}", config->get_keyword("database_name"));
	database->database_simpleQuery(to_safestring_16(query_line)); // Запрос к базе
}
void Database_manager::DB_simple_query(const std::wstring &query_line)
{
	DB_simple_query(wide_to_thin_line(query_line));
}

// Достаёт из базы первую запись, подходящую по запросу
std::string Database_manager::return_query_one(const std::string &query_line)
{
	std::vector<std::string> stopka = DB_return_query(query_line);
	return stopka.empty() ? "" : stopka[0];
}
std::wstring Database_manager::return_query_one(const std::wstring &query_line)
{
	return thin_to_wide_line(return_query_one(wide_to_thin_line(query_line)));
}

// Достаёт из базы рандомную запись, подходящую по запросу
std::string Database_manager::return_query_random(const std::string &query_line)
{
	std::vector<std::string> stopka = DB_return_query(query_line);
	return stopka.empty() ? "" : stopka[ rand() % stopka.size() ];
}
std::wstring Database_manager::return_query_random(const std::wstring &query_line)
{
	return thin_to_wide_line(return_query_random(wide_to_thin_line(query_line)));
}

// Достаёт количество записей
int Database_manager::return_query_count(const std::string &query_line)
{
	std::vector<std::string> stopka = DB_return_query(query_line);
	return stopka.empty() ? -1 : atoi(stopka[0].c_str());
}
int Database_manager::return_query_count(const std::wstring &query_line)
{
	return return_query_count(wide_to_thin_line(query_line));
}

// Достаёт список записей
std::string Database_manager::return_query_list(const std::string &query_line)
{
	std::vector<std::string> stopka = DB_return_query(query_line);
	
	std::wstring current_line;
	for (auto i : stopka) current_line += std::wstring(thin_to_wide_line(i) + L"\n");
	
	replace_in_text(current_line, L"//", L"/[empty]/");
	replace_in_text(current_line, L"/", L" ——— ");
	
	return wide_to_thin_line(current_line);
}
std::wstring Database_manager::return_query_list(const std::wstring &query_line)
{
	return thin_to_wide_line(return_query_list(wide_to_thin_line(query_line)));
}

// ___________________________________________________________
// ___________________________________________________________
// ___________________________________________________________

// Отвечает, если других ответов не найдено
std::string Database_manager::check_zero_word ( const std::vector <std::wstring> &current_storage )
{
	wchar_t temp_symbol[2] = L" ";
	std::string query_line;
	std::string answer;
	
	if( current_storage.empty() ) return answer;
	
	logs -> CommitConveer( F, L, "Answer not found" ); // Debug
	
	if( current_storage.size() == 1 && current_storage[0].length() <= config -> get_coefficient( L"len_max_single_unknown_word" ) ) // Если сообщение - одно непонятное короткое слово
	{
		logs -> CommitConveer( F, L, "Generating symbol" );
		
		if( current_storage[0].length() == 1 ) // Если один символ
		{
			temp_symbol[0] = current_storage[0][0];
			++ temp_symbol[0];	// Сдвиг на символ вперёд
			
			answer = wide_to_thin_line( temp_symbol ); // Преобразование в тонкую строку
		}
		else answer = generator -> make_smile();
	}
	else
	{
		int option_sum = rand() %
		(
			config -> get_coefficient( L"poss_zero_smile" ) +
			config -> get_coefficient( L"poss_zero_answer" ) +
			config -> get_coefficient( L"poss_zero_theme" )
		);
		
		if ( option_sum < ( config -> get_coefficient( L"poss_zero_smile" ) ) )
		{
			// Ответ смайлом
			logs -> CommitConveer( F, L, "Answer - with smile" ); // Debug
			answer = generator -> make_smile();
		}
		else if ( option_sum < ( config -> get_coefficient( L"poss_zero_smile" ) + config -> get_coefficient( L"poss_zero_answer" ) ) )
		{
			// Ответ - отговорка
			logs -> CommitConveer( F, L, "SQL QUERY - answers_notfound table" ); // Debug
			
			answer = get_typical_answer( "not_found" );
		}
		else
		{
			// Предыдущая тема
			logs -> CommitConveer( F, L, "Answer - previous theme" ); // Debug
			
			answer = historian -> get_theme();
		}
	}
	
	return answer;
}

// Проверяет отсортированную строку по базе по одному слову. 
std::string Database_manager::check_single_word ( const std::vector <std::wstring> &current_storage )
{
	std::vector <std::wstring> current_storage_sorted;
	std::string query_line;
	std::string answer;
	
	if( current_storage.empty() ) return answer;
	
	logs -> CommitConveer( F, L, "Checking each word in message" );
	
	current_storage_sorted = sort_words( current_storage );
	
	for( auto i : current_storage_sorted )
	{
		// Проверка ответов в базе по каждому слову
		if( all_symbols_too_wide( i ) ) continue;
		
		query_line = wide_to_thin_line( i ); // Преобразование в тонкую строку
		
		std::vector <std::string> current_dbres = DB_return_query( "SELECT `answer` FROM `{@database_name@}`.`typical_questions` WHERE `keyphrase` LIKE '%" + screen_sql ( query_line ) + "%' AND (`bot` = '' OR `bot` = '" + source_info->get_bot_name() + "') LIMIT " + std::to_string ( config -> get_coefficient ( L"query_limit_norm" ) ) + " ;" );
		if( ! current_dbres.empty() ) // Если ответ найден
		{
			logs -> CommitConveer( F, L, std::string( "Answer found, " + std::to_string( current_dbres.size() ) + " results" ) );
			
			answer = current_dbres[ rand() % current_dbres.size() ];
			
			logs -> CommitConveer( F, L, "Saving theme" ); // Debug
			if( current_dbres.size() > 1 ) historian -> add_theme( query_line ); // Запись ключевого слова в историю
			// Записать ключевого слова в пользовательский конфиг
			source_info -> userconfig->unsafe_tree()["last_theme"] = query_line;
			
			break; // Завершение цикла
		}
	}
	
	return answer;
}

// Обрабатывает пары слов
std::string Database_manager::check_double_word ( const std::vector <std::wstring> &current_storage )
{
	std::vector <std::wstring> pair_storage;
	std::string answer;
	int i;
	
	if( current_storage.size() < 2 ) return answer;
	
	logs -> CommitConveer( F, L, "Checking each 2 words in message" );
	
	for( i = 0; i < ( current_storage.size() - 1 ); ++i )
	{
		// Проверка ответов в базе по каждым 2 словам
		
		if
		(
			current_storage[i].length() < config -> get_coefficient( L"len_min_word_in_pair" ) &&
			current_storage[ i + 1 ].length() < config -> get_coefficient( L"len_min_word_in_pair" )
		) continue;
		
		pair_storage.push_back( current_storage[i] + L" " + current_storage[ i + 1 ] );
	}
	
	pair_storage = sort_words( pair_storage );
	
	answer = check_single_word( pair_storage );
	
	return answer;
}

// Обрабатывает тройки слов
std::string Database_manager::check_triple_word ( const std::vector <std::wstring> &current_storage )
{
	std::vector <std::wstring> combination_storage;
	std::string answer;
	int i;
	
	if( current_storage.size() < 3 ) return answer;
	
	logs -> CommitConveer( F, L, "Checking each 3 words in message" );
	
	for( i = 0; i < ( current_storage.size() - 2 ); ++i )
	{
		// Проверка ответов в базе по каждым 3 словам
		
// 		if( current_storage[i].length() < config -> get_coefficient ( L"len_min_word_in_pair" ) && current_storage[ i + 1 ].length() < config -> get_coefficient ( L"len_min_word_in_pair" ) ) continue;
		
		combination_storage.push_back( current_storage[i] + L" " + current_storage[ i + 1 ] + L" " + current_storage[ i + 2 ] );
	}
	
	combination_storage = sort_words( combination_storage );
	
	answer = check_single_word( combination_storage );
	
	return answer;
}

//Обрабатывает несколько слов по отдельности
std::string Database_manager::check_word ( const std::vector <std::wstring> &current_storage, const unsigned int words_amount )
{
	std::string query_line; // Запрос к БД
	std::string query_part; // Текущее слово
	std::string answer; // Ответ от БД
	int i, j; // Счётчики
	
	if( current_storage.empty() || current_storage.size() < words_amount ) return answer;
	
	logs -> CommitConveer( F, L, "Checking word groups in message" );
	
	for( i = words_amount; i <= current_storage.size(); ++i )
	{
		// Проверка ответов в базе по группам слов
		
		query_line += "SELECT `answer` FROM `{@database_name@}`.`typical_questions` WHERE (`bot` = '' OR `bot` = '" + source_info->get_bot_name() + "') AND ";
		
		for( j = i - words_amount; j < i; ++j )
		{
			if( all_symbols_too_wide( current_storage[j] ) ) continue;
			query_part = wide_to_thin_line( current_storage[j] ); // Преобразование в тонкую строку
			query_line += std::string( "`keyphrase` LIKE '%" + screen_sql ( query_part ) + "%' AND " );
		}
		query_line += std::string( "1 LIMIT " + std::to_string ( config -> get_coefficient ( L"query_limit_norm" ) ) + " ;" );
		
		std::string current_dbres = return_query_random( query_line );
		if( ! current_dbres.empty() ) // Если ответ найден
		{
			logs -> CommitConveer( F, L, std::string( "Answer found" ) );
			answer = current_dbres;
			break; // Завершение цикла
		}
		
		query_line.clear();
	}
	
	return answer;
}

// ___________________________________________________________
// ___________________________________________________________
// ___________________________________________________________


// Достаёт из базы типовой ответ указанного типа
std::string Database_manager::get_typical_answer ( const char* target )
{
	logs -> CommitConveer( F, L, "SQL QUERY - getting typical answer" );
	std::string current_dbres = return_query_random( "SELECT `phrase` FROM `{@database_name@}`.`typical_answers` WHERE `type` = '" + std::string ( target ) + "' LIMIT " + std::to_string ( config -> get_coefficient ( L"query_limit_big" ) ) + " ;" );
	return ( current_dbres.empty() ) ? config -> get_keyword ( "empty_db_typical" ) : current_dbres;
}

std::string Database_manager::check_person ( const std::wstring & current_string ) // Отвечает на вопрос "кто"
{
	logs -> CommitConveer( F, L, "SQL QUERY - getting person" );
	
	std::string answer = get_typical_answer( "person_0" );
	
	if ( config -> get_binary( L"available_person_gender_checking" ) )
	{
		if ( appears_in_this_string ( current_string, config -> get_keyword ( L"like_male" ).c_str() ) ) answer = get_typical_answer( "person_1" );
		if ( appears_in_this_string ( current_string, config -> get_keyword ( L"like_female" ).c_str() ) ) answer = get_typical_answer( "person_2" );
		if ( appears_in_this_string ( current_string, config -> get_keyword ( L"like_group" ).c_str() ) ) answer = get_typical_answer( "person_3" );
	}
	
	return answer;
}
std::string Database_manager::check_person( void ) // Отвечает на вопрос "кто"
{
	logs -> CommitConveer( F, L, "SQL QUERY - getting person" );
	return get_typical_answer( "person" );
}

std::string Database_manager::check_time( const std::wstring & current_string ) // Отвечает на вопрос "когда"
{	
	int n, t, day, month, year;
	std::string answer;
	std::wstring date_name;
	
	date_name = config -> get_group_random ( L"date_names" );
	
	// Определение времени вопроса ( прошлое / будущее / неопределённое )
	
	logs -> CommitConveer( F, L, "Generating time" );
	
	if ( appears_in_this_string( current_string, config -> get_keyword ( L"time_future" ).c_str() ) ) // будущее
	{
		t = 1;
		n = generator -> generate_random_number();
		answer = wide_to_thin_line ( config -> get_keyword ( L"part_time_after" ) ) + " " + std::to_string( n ) + " " + wide_to_thin_line ( date_name );
	}
	
	else if ( appears_in_this_string( current_string, config -> get_keyword ( L"time_past" ).c_str() ) ) // прошедшее
	{
		t = -1;
		n = generator -> generate_random_number();
		answer = std::to_string ( n ) + " " + wide_to_thin_line ( date_name ) + " " + wide_to_thin_line ( config -> get_keyword ( L"part_time_before" ) );
	}
	
	else // неопределённое ( дата )
	{
		t = 0;
		day = ( rand() % 30 ) + 1;
		month = ( rand() % 12 ) + 1;
		year = config -> get_coefficient ( L"random_year_beg" ) + ( rand() % ( config -> get_coefficient ( L"random_year_end" ) - config -> get_coefficient ( L"random_year_beg" ) + 1 ) );
		
		answer = ( day < 10 ? "0" : "" ) + std::to_string( day ) + ':' + ( month < 10 ? "0" : "" ) + std::to_string( month ) + ':' + std::to_string( year ) + " " + wide_to_thin_line ( config -> get_keyword ( L"part_after_date" ) );
	}
	
	// Или выводит рандомную дату, или даёт ответ из базы времени
	if( config -> get_possibility( L"poss_time_from_db" ) )
	{
		logs -> CommitConveer( F, L, "SQL QUERY - getting time" );
		return get_typical_answer( ( "time_" + std::to_string( t ) ).c_str() );
	}
	
	return answer;
}

std::string Database_manager::check_place( const std::wstring & current_string ) // Отвечает на вопрос "где?"
{
	logs -> CommitConveer( F, L, "SQL QUERY - getting place" );
	
	// Определение времени вопроса ( прошлое / будущее / неопределённое )
	     if( appears_in_this_string( current_string, config -> get_keyword ( L"place_from" ).c_str() ) ) return get_typical_answer( "location_-1" );
	else if( appears_in_this_string( current_string, config -> get_keyword ( L"place_to" ).c_str() ) ) return get_typical_answer( "location_1" );
	else return get_typical_answer( "location_0" );
}

std::string Database_manager::check_place( void ) // Отвечает на вопрос "где?"
{
	logs -> CommitConveer( F, L, "SQL QUERY - getting place" );
	return  get_typical_answer( "location" );
}

std::string Database_manager::explain_reason ( void ) // Отвечает на вопрос "почему?"
{
	logs -> CommitConveer( F, L, "SQL QUERY - getting reason" );
	return get_typical_answer( "reason" );
}

std::string Database_manager::explain_solution ( void )
{
	logs -> CommitConveer ( F, L, "SQL QUERY - getting solution" );
	return get_typical_answer( "solution" );
}

std::string Database_manager::answer_empty_message( void ) // Отвечает на пустое сообщение ( отговорка )
{
	logs -> CommitConveer( F, L, "SQL QUERY - getting answer for empty message" );
	return get_typical_answer( "empty" );
}

std::string Database_manager::generate_confidence_level( void ) // ВОзвращает степень уверенности
{
	logs -> CommitConveer( F, L, "SQL QUERY - getting confidence level" );
	return get_typical_answer( "confidence" );
}

std::string Database_manager::evaluate( void ) // Оценка чего-либо
{
	logs -> CommitConveer( F, L, "SQL QUERY - getting evaluation" );
	return get_typical_answer ( "evaluate" );
}

std::string Database_manager::answer_not_found( void )
{
	logs -> CommitConveer( F, L, "SQL QUERY - getting universal phrase" );
	return get_typical_answer ( "not_found" );
}

// ___________________________________________________________
// ___________________________________________________________
// ___________________________________________________________

// Углублённый поиск по словам
std::string Database_manager::drill_message ( const int available )
{
	logs -> CommitConveer( F, L, "Drilling message ( type " + std::to_string( available ) + " )" );
	
	std::string qbzv;
	
	if ( qbzv.empty() && config -> get_coefficient ( L"available_search_sentence" ) == available ) qbzv = check_single_word ( message -> sentence_storage );
	if ( qbzv.empty() && config -> get_coefficient ( L"available_search_group_4" ) == available  ) qbzv = check_word ( message -> word_storage, 4 );
	if ( qbzv.empty() && config -> get_coefficient ( L"available_search_triple" ) == available )   qbzv = check_triple_word ( message -> word_storage );
	if ( qbzv.empty() && config -> get_coefficient ( L"available_search_group_3" ) == available )  qbzv = check_word ( message -> word_storage, 3 );
	if ( qbzv.empty() && config -> get_coefficient ( L"available_search_double" ) == available )   qbzv = check_double_word ( message -> word_storage );
	if ( qbzv.empty() && config -> get_coefficient ( L"available_search_group_2" ) == available )  qbzv = check_word ( message -> word_storage, 2 );
	if ( qbzv.empty() && config -> get_coefficient ( L"available_search_single" ) == available )   qbzv = check_single_word ( message -> word_storage );
	if ( qbzv.empty() && config -> get_coefficient ( L"available_search_group_1" ) == available )  qbzv = check_word ( message -> word_storage, 1 );
	if ( qbzv.empty() && config -> get_coefficient ( L"available_search_zero" ) == available )     qbzv = check_zero_word ( message -> word_storage );
	
	return qbzv;
}

void Database_manager::save_unsolved_phrase( const std::string &current_string ) // Сохраняет сообщение в список для обучения
{
	if( current_string.empty() ) return;
	
	logs -> CommitConveer( F, L, "SQL QUERY - saving unsolved message" );
	
	if( return_query_count( "SELECT count(*) FROM `{@database_name@}`.`unsolved` WHERE `answer` = '' AND `redactor` = '' ;" ) <= config -> get_coefficient( L"teachmode/max_count_unsolved" ) )
	{
		DB_simple_query( "INSERT INTO `{@database_name@}`.`unsolved` ( `phrase`, `answer`, `redactor` ) VALUES ( '" + screen_sql ( current_string ) + "', '', '' )" );
		DB_simple_query( "DELETE FROM `{@database_name@}`.`unsolved` WHERE `phrase` = '';" );
	}
}

// Находит вопросительные предложения, ответ на которые да/нет.
std::wstring Database_manager::detect_questionword( const Message_storage &current_storage )
{
	std::wstring answer;
	
	if( current_storage.word_storage.empty() || current_storage.punctuation_line.empty() ) return answer;
	
	logs -> CommitConveer( F, L, "SQL QUERY - detecting boolean question" );
	
	if( appears_in_this_string( current_storage.punctuation_line, L"?" ) )
	{
		for( auto i : current_storage.word_storage )
		{
			if( i.empty() ) continue;
			answer = return_query_one( L"SELECT `basic_form` FROM `{@database_name@}`.`syntax` WHERE `type` = 'question' AND `word` = '" + thin_to_wide_line( screen_sql( wide_to_thin_line( i ) ) ) + L"' LIMIT 1" );
			if( ! answer.empty() ) break;
		}
		
		if( answer.empty() ) return L"?";
	}
	
	return answer;
}

std::string Database_manager::get_user_name( void )
{
	logs -> CommitConveer( F, L, "SQL QUERY - getting user name" );
	
	std::string answer = return_query_one( "SELECT `name` FROM `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` WHERE `id` = '" + screen_sql( source_info -> get_unique_id() ) + "' LIMIT 1;" );
	if( answer.empty() && return_query_count( "SELECT count(*) FROM `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` WHERE `id` = '" + screen_sql( source_info -> get_unique_id() ) + "' LIMIT 1;" ) < 1 )
		return source_info->get_user_first();
		//return config -> get_keyword( "error_unknown_name" );
	else return answer;
}

std::string Database_manager::find_answer_in_history ( const std::wstring &current_line )
{
	if
	(
		current_line.empty() ||
		message -> dont_dig_history_with_attachment() ||
		message -> dont_dig_history()
	) return "";
	else return return_query_one( " SELECT `answer` FROM `{@database_name@}`.`history` WHERE `phrase` LIKE '" + screen_sql( current_line ) + "' AND `id` LIKE '" + source_info -> get_bot_name() + "%' AND `answer` <> '{@empty@}' AND `message_form` <> 'command' AND `answer` <> '' AND `time` > DATE_SUB( NOW(), INTERVAL " + config -> get_keyword( "answer_search_in_history_interval" ) + " ) LIMIT 1 ; " );
}
std::string Database_manager::find_answer_in_history ( const std::string &current_line )
{
	return find_answer_in_history( thin_to_wide_line( current_line ) );
}

Answerbox Database_manager::get_attachment( const std::string &type, const std::string &type_sub ) // Вытаскивает вложение из базы данных
{
	// Вложения для SaloAPI
	if( source_info -> get_interface() == "SaloAPI" ) return Answerbox( ( type_sub.empty() ? type : type_sub), type + "_here", type );
	
	//Достаём ссылки для нужного типа и выбираем случайную из них
	logs -> CommitConveer ( F, L, "SQL QUERY - getting attachment" );
	
	std::string current_row = return_query_random( "SELECT `link`, `answer` FROM `{@database_name@}`.`attachment` WHERE (`type` = '" + screen_sql(source_info -> get_bot_name_and_interface() + "_" +  type) + "'" + std::string(config->get_binary("available/use_global_attachments") ? " OR `type` = '" + screen_sql(source_info->get_interface() + "_" + type) + "'" : "") + ") AND `type_sub` = '" + screen_sql( type_sub ) + "' LIMIT " + std::to_string( config -> get_coefficient ( L"query_limit_big" ) ) + " ;" );
	
	int limiter_index = current_row.find( "/" );
	if( limiter_index == std::string::npos ) return Answerbox( config -> get_keyword( "empty_db_attachment" ) );
	else
	{
		std::string current_attachment = current_row.substr(0, limiter_index );
		std::string current_text = current_row.substr(limiter_index + 1, current_row.length() );
		if( current_text.empty() ) current_text = type_sub.empty() ? type : type_sub;
		return Answerbox( current_text, current_attachment, type );
	}
}

std::string Database_manager::fetch_text( const std::string &type, const std::string &type_sub ) // Вытаскивает текст из базы данных
{
	//Достаём тексты для нужного типа и выбираем случайный из них
	logs -> CommitConveer ( F, L, "SQL QUERY - getting text" );
	std::string current_text = return_query_random( "SELECT `answer` FROM `{@database_name@}`.`story` WHERE `type` = '" + screen_sql( type ) + "' AND `type_sub` = '" + screen_sql( type_sub ) + "' LIMIT " + std::to_string( config -> get_coefficient ( L"query_limit_big" ) ) + " ;" );
	return current_text.empty() ? config -> get_keyword ( "empty_db_story" ) : current_text;
}

/*
 * TO ADD USERS IN `user_reserve` DATABASE
 * 
 * INSERT INTO `DATABASE_NAME`.`user_reserve` (`id`, `first`, `last`, `name`, `messages_sent`, `cur_theme`, `attitude`, `status`, `config`, `active_last`) 
 * SELECT `id`, `first`, `last`, `name`, `messages_sent`, `cur_theme`, `attitude`, `status`, `config`, `active_last` 
 * FROM `user` 
 * WHERE `time` < DATE_SUB(NOW(), INTERVAL 1 MONTH);
 * 
 * DELETE FROM `DATABASE_NAME`.`user` 
 * WHERE `time` < DATE_SUB(NOW(), INTERVAL 1 MONTH);
 * 
 * DELETE FROM `DATABASE_NAME`.`web_user` 
 * WHERE `time` < DATE_SUB(NOW(), INTERVAL 1 MONTH);
 * 
 * */

void Database_manager::recover_user(const std::string &id)
{
	// If user is not stable, leave
	// Check, if ID is present in `user` database.
	if (
		!config->get_binary("available/recover_users") ||
		!source_info->user_is_stable() ||
		0 != return_query_count("SELECT COUNT(*) FROM `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` WHERE `id` = '" + screen_sql(id) + "';")
	) return;
	
	// If user is not found in `user` database, check it`s existance in `user_reserve` database.
	if (0 != return_query_count("SELECT COUNT(*) FROM `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user_reserve" : "web_user_reserve") + "` WHERE `id` = '" + screen_sql(id) + "';"))
	{
		// If user is there, move it to `user` database
		DB_simple_query("INSERT INTO `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` (`id`, `first`, `last`, `name`, `messages_sent`, `cur_theme`, `attitude`, `status`, `config`, `active_last`) SELECT `id`, `first`, `last`, `name`, `messages_sent`, `cur_theme`, `attitude`, `status`, `config`, NOW() FROM `user_reserve` WHERE `id` = '" + screen_sql(id) + "';");
		DB_simple_query("DELETE FROM `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user_reserve" : "web_user_reserve") + "` WHERE `id` = '" + screen_sql(id) + "';");
	}
}

} // namespace Convspace
