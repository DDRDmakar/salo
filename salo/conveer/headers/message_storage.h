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


#ifndef __SEQUENCE_CHECK_MESSAGE_STORAGE__
#define __SEQUENCE_CHECK_MESSAGE_STORAGE__


#include <unordered_map>
#include <vector>
#include <regex>
#include "class_heads.h" // TODO remove it
#include "config.h"
#include "database_manager.h"

namespace Convspace
{

class Message_storage
{
	
private:
	
	// COMMAND PHRASE ALIAS SHOULD NOT END WITH PUNCTUATION SYMBOL
	class Command_detector
	{
		
	private:
		
		Config *config;
		Community_organizer *source_info;
		
		struct Command_node
		{
			Command_node(void);
			
			bool is_regex, is_part, icase, active;
			std::wregex reg;
			std::string ID, heh;
			std::wstring sep;
			std::vector<std::wstring> word, command;
		};
		
		bool ready;
		int config_version;
		// Map of pairs interface<-->list_of_commands
		std::unordered_map<std::string, std::list<Command_node>> alias_storage;
		
	public:
		
		Command_detector(
			Config *config = nullptr,
			Community_organizer *source_info = nullptr
		);
		~Command_detector(void);
		void refresh(void);
		std::wstring convert(std::wstring current_line);
		bool is_ready(void);
	};
	
	
	Config *config; // Конфиг данного бота
	Community_organizer *source_info; // Информация об источнике
	Database_manager *dbm; // Система управления БД
	
	int config_version;
	Command_detector detector;
	
	// Переменная для хранения командных преобразований
	YAML::Node config_storage;
	
	std::vector <std::vector <std::wstring>> split_group_parts( const std::vector <std::wstring> &current_group ); // Разделить группы строк
	
	// Показать содержимое
	std::string show_storage( const std::vector <std::string>  &current_storage, const std::string name = "" );
	std::string show_storage( const std::vector <std::wstring> &current_storage, const std::string name = "" );
	
	// Detectors
	std::string detect_message_form( void ); // Определяет тип сообщения
	bool detect_mathe( std::wstring current_string ); // Определение в сообщении математического примера
	bool detect_word_set( const std::wstring &target ); // Определить словосочетание в строке
	bool detect_word_set_begin( const std::wstring &target ); // Определить словосочетание в начале фразы
	
	// Modifiers
	void format_phrase_punctuation( std::wstring& current_line ); // Приведение текста к стандартному виду
	void format_phrase( std::wstring& current_line ); // Приведение текста к стандартному виду ( без сохранения пунктуации и удаления слов - паразитов ) - для побочных строк
	void basic_format_phrase( std::wstring& current_line ); // Поверхностное форматирование
	std::wstring split_symbol_groups( std::wstring current_line ); // Разбить символы из разных групп
	
	// Replacers
	std::wstring replace_and_get_punctuation( std::wstring &current_string ); // Удаление пунктуации с получением строки с ней
	std::wstring replace_punctuation( const std::wstring &current_string ); // Удаление пунктуации
	void remove_short_words( std::vector <std::wstring> & current_storage ); // Удалить слишком короткие слова
	void remove_parasite_words( std::wstring& current_string ); // Вырезать слова - паразиты
	void remove_primary_keyword( std::string& current_string ); // Удаление ключевого слова ( обращения к боту )
	void remove_word( std::wstring& current_string, const std::wstring &target ); // Удаление слова из строки ( узкое применение )
	void remove_first( std::wstring& current_string, const std::wstring &target ); // Удаление первого слова в сообщении
	
public:
	
	bool is_clear;
	unsigned int n_words; // Количество слов
	std::string message_form; // Тип сообщения ( "" = неизвестен )
	bool message_fetched_without_bot_name; // Если ключевое слово фетчера не является обращением к боту
	
	// Тонкие строки
	std::string language; // Язык входного сообщения
	std::string basic_line; // Первоначальное мультибайтное сообщение ( перевод )
	std::string basic_line_without_translation; // Первоначальное мультибайтное сообщение
	std::string pure_line_thin; // Очищенная строка в мультибайтном виде
	
	// Толстые строки
	std::wstring pure_line; // Сообщение в очищенном виде
	std::wstring intact_line; // Сообщение без форматирования ( с пунктуацией и заглавными буквами )
	std::wstring punctuation_line; // Строка для хранения знаков пунктуации
	std::wstring command_line; // Команда ( в том числе преобразованная )
	std::wstring basic_without_parasite; // Первоначальное сообщение без слов - паразитов
	
	// Стопки
	std::vector<std::wstring> word_storage; // Хранилище всех слов по отдельности
	std::vector<std::wstring> word_storage_intact; // Хранилище всех слов в первозданном виде
	std::vector<std::wstring> word_storage_sorted; // Сортированных ( по длине ) + удалены короткие слова
	std::vector<std::wstring> word_storage_cmd; // Хранилище командной последовательности
	std::vector<std::wstring> sentence_storage; // Хранилище предложений
	std::vector<std::wstring> sentence_storage_sorted;
	std::vector<std::wstring> pyramide_storage; // Хранилище слов группами по возрастанию
	std::vector<std::wstring> non_bot_name_storage; // Хранилище слов, на которые реагирует бот, но которые не обращения
	int non_bot_name_storage_version; // Версия для синхронизации с конфигом
	
	Message_storage
	(
		Config *transit_config = NULL,
		Community_organizer *transit_source_info = NULL,
		Database_manager *transit_dbm = NULL
	); // Констркутор
	
	~Message_storage(void); // Деструктор
	
	// Verificators
	// Проверка возможность/невозможность анализа сообщения
	bool dont_dig_history( void );
	bool dont_dig_history_with_attachment( void );
	bool dont_parse_before_typecheck( void );
	bool dont_parse_before_typecheck_with_attachment( void );
	
	void add_message_form( const std::string &transit_message_form ); // Добавить тип сообщения
	void put( std::string mes ); // Записать и проанализировать новое сообщение
	void clear( void ); // Очистить все поля
	void show( void ); // Показать все хранилища
	
};

} // namespace Convspace

#endif
