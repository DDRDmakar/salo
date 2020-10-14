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

#include "headers/message_storage.h"
#include "headers/community.h"
#include "headers/database_manager.h"
#include "headers/parse_tools.h"

namespace Convspace
{


Message_storage::Message_storage
(
	Config *transit_config,
	Community_organizer *transit_source_info,
	Database_manager *transit_dbm
)
{
	logs -> CommitConveer( F, L, "Object initialization - Message_storage" );
	
	is_clear = true;
	n_words = 0;       // Нет слов
	message_form = ""; // Неизвестный тип
	language = "nolang";
	
	config = transit_config;
	source_info = transit_source_info;
	dbm = transit_dbm;
	
	if( config )
	{
		config_version = config -> get_version();
		config_storage = config -> subtree("cmd");
	}
	else config_version = 0;
	
	non_bot_name_storage_version = 0;
	
	if (transit_config && transit_source_info && transit_dbm)
	{
		detector = Command_detector(transit_config, transit_source_info);
	}
}

Message_storage::~Message_storage(void) {}

void Message_storage::add_message_form( const std::string &transit_message_form ) { message_form = transit_message_form; }

void Message_storage::put( std::string mes )
{
	logs -> CommitConveer( F, L, "Putting word into Message_storage" );

	is_clear = false; // В объекте есть содержимое
	
//	Ф И Л Ь Т Р А Ц И Я __________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
	
	logs -> CommitConveer( F, L, "Filtering string" );
	basic_line_without_translation = filter_SQL( mes ); // Защита от SQL инъекций
	unscreen( basic_line_without_translation ); // Убирает экранированные символы
	
	// Здесь был переводчик
	basic_line = basic_line_without_translation; // Это осталось со времени, когда был автопереводчик
	
	remove_primary_keyword( basic_line ); // Удаление ключевого слова ( обращения к боту )
	
//	О Б Р А Б О Т К А ____________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
	
	pure_line = intact_line = thin_to_wide_line( basic_line ); // Преобразование в толстую строку
	
	sentence_storage = disassemble_sentences( intact_line ); // Разделение сообщения на предложения
	for( int i = 0; i < sentence_storage.size(); ++i ) format_phrase( sentence_storage[i] );
	sentence_storage_sorted = sort_words( sentence_storage ); // Сортировка по убыванию длины
	
	format_phrase_punctuation( pure_line ); // Удаление пунктуации, заглавных букв и лишних пробелов
	basic_format_phrase( intact_line ); // Удаление базовых ошибок передачи
	basic_without_parasite = intact_line;
	remove_parasite_words( basic_without_parasite ); // Удаление слов - паразитов для повторения
	// TODO сделать строку для выбора из вариантов
	
	pyramide_storage = disassemble_pyramide( pure_line ); // Разделение слов для замены на команды
	
	remove_parasite_words( pure_line ); // Удаление ненужных слов
	
	word_storage = disassemble_words( pure_line ); // Разбиение сообщения на слова
	word_storage_intact = disassemble_words( intact_line ); // Разбиение сообщения на слова ( без форматирования )
	word_storage_sorted = sort_words( word_storage ); // Сортируем слова по длине
	remove_short_words( word_storage_sorted ); // Удаляем слишком короткие слова
	
	n_words = word_storage.size(); // Не знаю, нахрена это нужно, но доставляет моральное удовлетворение
	message_form = detect_message_form(); // Определение типа сообщения
	
	pure_line_thin = wide_to_thin_line( pure_line );
	
	// Преобразование определённых фраз в команды
	// Передаются сообщение без форматирования и группы слов с форматированием
	//convert_cmd();
	//dbm -> convert_cmd( "command" );
	if (!detector.is_ready()) detector.refresh();
	if  (detector.is_ready()) command_line = detector.convert(intact_line);
	else command_line = intact_line;
	word_storage_cmd = disassemble_words(command_line);
	
	logs -> CommitConveer( F, L, "Preprocessing finished suxessfully" );
	logs -> CommitConveer( F, L, std::string( "\"" + pure_line_thin + "\" [" + std::to_string( pure_line_thin.length() ) + " bytes]" ) );
}


//	О Ч И С Т К А ________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________

void Message_storage::clear( void )
{
	is_clear = true;
	
	n_words = 0;
	message_form = '\0';
	
	// Тонкие строки
	language.clear();
	basic_line.clear();
	basic_line_without_translation.clear();
	pure_line_thin.clear();
	
	// Толстые строки
	pure_line.clear();
	intact_line.clear();
	punctuation_line.clear();
	command_line.clear();
	basic_without_parasite.clear();
	
	// Стопки
	word_storage.clear();
	word_storage_intact.clear();
	word_storage_sorted.clear();
	word_storage_cmd.clear();
	sentence_storage.clear();
	sentence_storage_sorted.clear();
	pyramide_storage.clear();
	
}

std::string Message_storage::show_storage( const std::vector <std::string> &current_storage, const std::string name )
{
	std::string result = "+==== STRING STORAGE " + name + " ====+\n";
	result += "+==== " + std::to_string( current_storage.size() ) + " elements\n";
	for( auto i : current_storage ) result += "# [" + i + "]\n";
	result += "+====================+\n";
	return result;
}
std::string Message_storage::show_storage( const std::vector <std::wstring> &current_storage, const std::string name )
{
	std::string result = "+==== STRING STORAGE " + name + " ====+\n";
	result += "+==== " + std::to_string( current_storage.size() ) + " elements\n";
	for( auto i : current_storage ) result += "# [" + wide_to_thin_line( i ) + "]\n";
	result += "+====================+\n";
	return result;
}

void Message_storage::show( void )
{
	// Тонкие строки
	std::cout << "Language [" << language << "]\n";
	std::cout << "Basic line [" << basic_line << "]\n";
	std::cout << "Basic line without translation [" << basic_line_without_translation << "]\n";
	std::cout << "Pure line thin [" << pure_line_thin << "]\n";
	
	// Толстые строки
	std::cout << "Pure line [" << wide_to_thin_line( pure_line ) << "]\n";
	std::cout << "Intact line [" << wide_to_thin_line( intact_line ) << "]\n";
	std::cout << "Punctuation line [" << wide_to_thin_line( punctuation_line ) << "]\n";
	std::cout << "Command line [" << wide_to_thin_line( command_line ) << "]\n";
	std::cout << "Basic line without parasite words [" << wide_to_thin_line( basic_without_parasite ) << "]\n";
	
	// Стопки
	std::cout << show_storage( word_storage, "Word storage" );
	std::cout << show_storage( word_storage_intact, "Word storage intact" );
	std::cout << show_storage( word_storage_sorted, "Word storage sorted" );
	std::cout << show_storage( word_storage_cmd, "Word storage cmd" );
	std::cout << show_storage( sentence_storage, "Sentence storage" );
	std::cout << show_storage( sentence_storage_sorted, "Sentence storage sorted" );
	std::cout << show_storage( pyramide_storage, "Pyramide storage" );
}

} // namespace Convspace
