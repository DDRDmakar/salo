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

#include "headers/conveer.h"
#include "headers/parse_tools.h"

namespace Convspace
{

std::mutex Conveer::conveer_mutex;
unsigned int Conveer::transit_id = 0;
unsigned int Conveer::count = 0;

// Конструктор
Conveer::Conveer( const std::string &transit_config_name )
{
	logs -> CommitConveer( F, L, "Object initialization - Conveer" );
	
	// Получение идентификатора
	conveer_mutex.lock();
	conveer_id = transit_id ++;
	++ count;
	conveer_mutex.unlock();
	
	// Счётчики
	first_run = true;
	repeats = 0;
	total_count = 0;
	fetcher_keyword_storage_version = -1;
	
	// Формирование конфига
	config_name = transit_config_name;
	config = Config("bot_config/" + config_name);
	workdat = &config;
	
	// Первое считывание динамического конфига
	// const std::string dynamic_config_path = "resources/config/bot_config/" + config_name + "dynamic.yaml";
	// dyn_config = file_exists(dynamic_config_path) ? Config_dyn(wide_to_thin_line(read_file(dynamic_config_path))) : Config_dyn();
	
	// При добавлении нового указателя, нужно добавить новый аргумент:
	//		в вызов конструктора в конвеере
	//		объект - член соответствующего класса
	//		аргумент в объявлении конструктора (с дефолтным значением NULL)
	//		аргумент в самой функции конструктора
	//		присваивание значения указателю внутри объекта
	
	// Внутриконвеерные объекты получают ссылки друг на друга
	/*
	 * 
	 * &config
	 * &source_info 
	 * &message 
	 * &dbm 
	 * &dyn_config
	 * &historian 
	 * &generator 
	 * &security 
	 * &sensei 
	 * &information 
	 * &game_magic_field 
	 * &game_word_chain
	 * 
	 */
	
	//config_user = Config_user();
	generator = Answer_generator(
		&config,
		&source_info,
		&message,
		&dbm,
		&dyn_config,
		&security,
		&sensei,
		&information
	);
	dbm = Database_manager(
		&config,
		&source_info,
		&message,
		&historian,
		&generator
	);
	message = Message_storage(
		&config,
		&source_info,
		&dbm
	);
	historian = History_organizer(
		&config,
		&source_info,
		&message,
		&dbm,
		&dyn_config
	);
	information = Information(
		&config,
		&source_info,
		&dbm
	);
	source_info = Community_organizer(
		&config,
		&dbm
	);
	security = Ban_organizer(
		&config,
		&source_info,
		&dbm
	);
	sensei = Teach_organizer(
		&config,
		&source_info,
		&dbm,
		&dyn_config
	);
	dyn_config = Config_dyn(
		&config
	);
	
	
	// Read dynamic config data
	dyn_config.read_from_file();
	
}

Conveer::~Conveer( void )
{
	conveer_mutex.lock();
	-- count;
	conveer_mutex.unlock();
}

unsigned int Conveer::get_count( void ) { return count; }
unsigned int Conveer::get_id( void ) { return conveer_id; }
std::string Conveer::get_config_name( void ) { return config_name; }
std::string Conveer::get_database_name( void ) { return config.get_keyword( "database_name" ); }

Answerbox Conveer::generate_answer(  const Person &user, const CONFIGURATION &bot_config, const std::string &mes )
{
	// Заполнение информации об источнике
	source_info.add_basic_data( user, bot_config );
	// Заполнение текстового хранилища
	message.put( mes );
	
	// TEST
	// TEST
	//================================//
	
	// Генерация ответа
	return internal_answer();
}

void Conveer::put_data( const Person &user, const CONFIGURATION &bot_config/*, const std::string &mes*/ )
{
	source_info.add_basic_data( user, bot_config );
	//message.put( mes );
}

Answerbox Conveer::internal_answer( void )
{
	logs -> CommitConveer( F, L, "	*** Conveer main function start ***" );
	logs -> CommitConveer( F, L, "	User: \"" + source_info.get_unique_id() + "\"" );
	logs -> CommitConveer( F, L, "	Message text: \"" + message.basic_line + "\"" );
	logs -> CommitConveer( F, L, "	Message attachment: \"" + source_info.get_attachment() + "\"" );
	
	//message.show();
	
// Переменные конвеера
	std::string part_query_result; // Результат запроса к базе по всему тексту
	Answerbox answer;              // Структура окончательного ответа
	
// Восстановление пользователя из резервной базы данных, если он там находится.
	dbm.recover_user(source_info.get_unique_id());
	
// Создание записи о пользователе в базе, если её ещё нет
	historian.add_new_user();
	
// Добавление фразы в оперативную память истории, независимо от ответа
	if (config.get_binary("available/history")) historian.add_phrase( message.basic_line );
	
// Проверка бан-листа
	security.remove_old();
	if( config.get_binary("available_ban") && security.is_banned() )
	{
		logs -> CommitConveer( F, L, "Current user is banned" );
		message.add_message_form( "banned_user" );
		answer.is_active = false;
		goto finish_nomemory;
	}
	
// Проверка игнор-листа
	if( config.get_binary("available/ignore") && config.one_matches( "ignore_list", source_info.get_bot_interface_and_id() ) )
	{
		logs -> CommitConveer( F, L, "Current user is ignored" );
		message.add_message_form( "ignored_user" );
		answer.is_active = false;
		goto finish_nomemory;
	}
	
// Защита
	if
	(   // Если
		config.get_binary( L"available_ban" ) &&                // можно банить людей автоматически
		! generator.detect_cmd( message.word_storage_cmd ) &&   // и сообщение - не команда
		source_info.get_attachment().empty()                    // и нет медиа-вложений
	)
	{
		// Если человек нафлудил - банить
		if ( security.detect_flood( message.basic_line, config.get_coefficient( L"flood_mes_count" ) ) )
		{
			security.ban( config.get_coefficient( L"ban_time" ), "", "flood" ); // Забанить
			message.add_message_form( "ban_flood" );                            // Изменить тип сообщения в базе
			answer.text = wide_to_thin_line( config.get_keyword( L"you_banned_24" ) ); // Формирование текста ответа
			answer.is_active = source_info.get_configuration().isConf;          // Если надо, блокировать отправку ответа
			goto finish_nounsolved;
		}
		// Если скоро нафлудит - кидать предупреждение
		else if ( security.detect_flood( message.basic_line, config.get_coefficient( L"flood_mes_count_pre" ) ) )
		{
			answer.text = config.get_keyword( "ban_soon" ); // Формирование текста предупреждения
			message.add_message_form( "ban_warning" );      // Изменить тип сообщения в базе
			goto finish_nounsolved;
		}
	}
	
// Обработка команд в строке
	if( config.get_binary("available/cmd") && generator.detect_cmd( message.word_storage_cmd ) )
	{
		// Заменяю фразу на преобразованную команду
		message.add_message_form( "command" );
		if (config.get_binary("available/history")) historian.add_phrase( wide_to_thin_line( message.command_line ) );
		answer = generator.analyse_cmd();
		if( answer.text == "{@proceed@}" )
		{
			answer.text.clear();
			goto proceed;
		}
		else goto finish_nounsolved;
	}
	
// Обработка вложений ( не текста )
	
	if // Если вложение
	(
		config.get_binary("available/attachment") &&
		(message.pure_line.empty() || config.get_binary("available/react_to_attachment_with_text")) &&
		! source_info.get_attachment().empty() &&
		source_info.get_attachment() != "null"
	)
	{
		logs -> CommitConveer( F, L, "Attachments detected" );
		message.add_message_form( "attachment_" + source_info.get_attachment() );
		answer = generator.answer_attachment( source_info.get_attachment() ); // Получение ответа на вложение
		goto finish_nounsolved; // Перебрасывает в конец конвеера ( без запоминания в нераспознанные )
	}
	
// Режим обучения
	if
	(
		config.get_binary("available/teach") &&
		sensei.is_participant() &&
		source_info.user_is_stable()
	)
	{
		message.add_message_form("teachmode");
		
		if (source_info.is_conf() && !config.get_binary("teachmode/in_conf"))
			answer.text = config.get_keyword("teachmode/error/forbidden_in_conf");
		
		// Проверка на ложное срабатывание бота (без обращения)
		else if (source_info.is_conf() && message.message_fetched_without_bot_name)
			answer.text = config.get_keyword("teachmode/error/fetched_without_botname");
		else
			answer.text = sensei.teach( message.intact_line );
		
		goto finish_nounsolved;
	}
	
// Игры
	
	if( config.get_binary("available/game") && generator.user_in_game() )
	{
		message.add_message_form( "game" );
		answer.text = generator.play_game();
		goto finish_nounsolved;
	}
	
//	О Т В Е Т ____________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
	
	proceed: // ### Сюда, если нужно добавить текстовый ответ.
	logs -> CommitConveer( F, L, "Proceed ( no special modes found )" );
	
// Переброс непроверяемых типов
	if( message.dont_parse_before_typecheck_with_attachment() ) goto non_parsable;
	
// Если сообщение пустое
	if( message.pure_line.empty() && source_info.get_attachment().empty() )
	{
		logs -> CommitConveer( F, L, "0 words found in message" );
		message.add_message_form( "empty" );
		
		if (config.get_binary("available/answer_empty_message"))
		{ answer.text = dbm.answer_empty_message(); goto finish_nounsolved; } // Перебрасывает в конец конвеера
		else
		{ answer.is_active = false; goto finish_nomemory; }
	}
	
// Проверка на повторения пользователя
	if
	(
		message.basic_line.length() >= config.get_coefficient( L"len_min_repeat_control" ) &&
		config.get_possibility( "poss_repeat_control" ) &&
		! message.dont_parse_before_typecheck() &&
		historian.repeat_control( message.basic_line, "phrase" )
	)
	{
		logs -> CommitConveer( F, L, "Repeat control alerted" );
		message.add_message_form( "user_repeat" );
		answer.text = wide_to_thin_line( config.get_keyword( L"repeat_control" ) );
		goto finish_nounsolved;
	}
	
	if (!config.get_binary("available/chat")) goto non_parsable;
	
// Запрос с полным текстом сообщения
	logs -> CommitConveer( F, L, "SQL QUERY - full message query" );
	if
	(
		message.n_words < config.get_coefficient( L"len_min_words_part_message_query" ) &&
		message.pure_line.length() < config.get_coefficient( L"len_min_part_message_query" )
	) part_query_result = dbm.return_query_random( "SELECT `answer` FROM `{@database_name@}`.`typical_questions` WHERE `keyphrase` LIKE '" + screen_sql( message.pure_line_thin ) + "' AND (`bot` = '' OR `bot` = '" + source_info.get_bot_name() + "') LIMIT " + std::to_string( config.get_coefficient( L"query_limit_small" ) ) + " ;" );
	if( ! part_query_result.empty() )
	{
		// Если есть ответ по целому сообщению
		logs -> CommitConveer( F, L, "Full message query was successful !" );
		message.add_message_form( "" );
		answer.text = part_query_result; // Рандомный ответ из списка  подходящих
		goto finish_nounsolved;
	}
	
	// Поиск сообщения как части текста
	// Если нельзя парсить сообщение по частям до определения типа, то перебрасывает
	else
	{
		if( message.dont_parse_before_typecheck() || all_symbols_too_wide( message.pure_line_thin ) ) goto non_parsable;
		else
		{
			part_query_result = dbm.return_query_random( "SELECT `answer` FROM `{@database_name@}`.`typical_questions` WHERE `keyphrase` LIKE '%" + screen_sql( message.pure_line_thin ) + "%' AND (`bot` = '' OR `bot` = '" + source_info.get_bot_name() + "') LIMIT " + std::to_string( config.get_coefficient ( L"query_limit_small" ) ) + " ;" );
			if( part_query_result.empty() )
			{
				// Если не нашлось ответа по целому сообщению, то глубокий анализ ( первого порядка )
				part_query_result = dbm.drill_message( 1 );
			}
			
			if( ! part_query_result.empty() )
			{
				message.add_message_form( "" );
				if( message.pure_line.length() <= config.get_coefficient( L"len_max_unsolved_little_message" ) ) dbm.save_unsolved_phrase( message.pure_line_thin ); // Запоминает, что это сообщение не было нормально обработано
				answer.text = part_query_result;
				goto finish_nounsolved;
			}
		}
	}
	
	non_parsable: // ### Сюда, если не нужно разбирать части сообщения до разбора по типу.
	
// Поиск ответа в истории, если можно
	if
	(
		config.get_binary("available/history") &&
		config.get_binary("available/search_answer_in_history") &&
		answer.text.empty() &&
		! all_symbols_too_wide( message.intact_line ) &&
		! message.dont_dig_history() &&
		! message.dont_dig_history_with_attachment()
	)
	{
		logs -> CommitConveer( F, L, "Searching for answer in history" );
		answer.text = dbm.find_answer_in_history( message.intact_line );
	}
	if( ! answer.text.empty() ) goto finish_nounsolved;
	
	
// Если не нашлось ответа на целое сообщение
	if( answer.text.empty() ) 
	{
		char message_form_first_letter = message.message_form.empty() ? '!' : message.message_form[0];
		switch( message_form_first_letter ) // Определяем тип сообщения
		{
// Математический пример
			case 'm':
			{
				logs -> CommitConveer( F, L, "Type: m ( Mathematics )" );
				answer.text = getMathematicsResult( message.basic_line );
				
				goto finish_nounsolved;
				break;
			}
// Отвеn да/нет ( степень уверенности )
			case 'b':
			{
				logs -> CommitConveer( F, L, "Type: b ( Binary answer )" );
				
				answer.text = dbm.generate_confidence_level();
				
				goto finish;
				break;
			}
// Количество (рандом)
			case 'x':
			{
				logs -> CommitConveer( F, L, "Type: x ( Random number )" );
				
				answer.text = to_string( generator.generate_random_number() );
				
				goto finish;
				break;
			}
// Время
			case 't':
			{
				logs -> CommitConveer( F, L, "Type: t ( Time / date )" );
				
				answer.text = dbm.check_time( message.pure_line );
				
				goto finish;
				break;
			}
// Место
			case 'w':
			{
				logs -> CommitConveer( F, L, "Type: w ( Place )" );
				
				answer.text = dbm.check_place( message.pure_line );
				
				goto finish;
				break;
			}
// Кто - то
			case 'p':
			{
				logs -> CommitConveer( F, L, "Type: p ( Person )" );
				
				answer.text = dbm.check_person( message.pure_line );
				
				goto finish;
				break;
			}
// Причина
			case '?':
			{
				logs -> CommitConveer( F, L, "Type: ? ( Reason )" );
				
				answer.text = dbm.explain_reason();
				
				goto finish;
				break;
			}
// Повторение фразы
			case 'r':
			{
				logs -> CommitConveer( F, L, " Type: r ( Repeat phrase )" );
				std::wstring temporary_repeated_message =  message.basic_without_parasite;
				remove_keyword( temporary_repeated_message );
				
				answer.text = wide_to_thin_line( temporary_repeated_message );
				if (std::regex_search(to_single_line(to_lowercase(thin_to_wide_line(answer.text))), config.get_regex(L"regex/part/forbidden_answer")))
			 		answer.text = config.get_keyword( "error/forbidden_answer" );
				
				goto finish;
				break;
			}
// Выбор между вариантами
			case 'c':
			{
				logs -> CommitConveer( F, L, "Type: c ( Choose options )" );
				
				answer.text = generator.choose_between_options( message.pure_line );
				if (std::regex_search(to_single_line(to_lowercase(thin_to_wide_line(answer.text))), config.get_regex(L"regex/part/forbidden_answer")))
					answer.text = config.get_keyword( "error/forbidden_answer" );
				
				goto finish;
				break;
			}
// Смайл
			case 's':
			{
				logs -> CommitConveer( F, L, "Type: s ( Smile )" );
				answer.text = generator.make_smile();
				goto finish;
				break;
			}
// Анализ числа
			case 'n':
			{
				logs -> CommitConveer( F, L, "Type: n ( Analyse of number )" );
				answer.text = generator.analyse_num( message.basic_line );
				
				goto finish_nounsolved;
				break;
			}
// Вероятность (%)
			case 'l':
			{
				logs -> CommitConveer( F, L, " Type: l ( Probability )" );
				
				answer.text = "\n( " + message.pure_line_thin + " )\n";
				answer.text += std::string( std::to_string( rand() % 101 ) + "\%" );
				
				goto finish_nounsolved;
				break;
			}
// Оценивание
			case 'e':
			{
				logs -> CommitConveer( F, L, " Type: e ( Evaluate )" );
				
				answer.text = "\n( " + message.pure_line_thin + " )\n" + dbm.evaluate(); // Оценка чего-либо
				
				goto finish;
				break;
			}
// Определение из интернета
			case 'i':
			{
				logs -> CommitConveer ( F, L, " Type: i (Info)" );
				// Проверка допустимой длины запроса
				if (message.pure_line_thin.length() > config.get_coefficient("len_max_description_query"))
				{
					answer.text = config.get_keyword("error/too_long_description_query");
					break;
				}
				answer.text = wide_to_thin_line(generator.get_description(message.pure_line));
				if(answer.text.empty())
				{
					message.add_message_form("");
					goto deep_search; // Углублённый поиск по базе
				}
				else answer.text.insert(0, "\n");
				
				goto finish_nounsolved;
				break;
			}
// Совет
			case 'o':
			{
				logs -> CommitConveer ( F, L, " Type: o ( Solution )" );
				
				answer.text = dbm.explain_solution();
				
				goto finish_nounsolved;
				break;
			}
			
// Если тип не определился
			default:
			{
				deep_search: // Сюда, если нужно искать по словам
				
				logs -> CommitConveer( F, L, "Type: default ( Message form not recognized )" );
				if (config.get_binary("available/chat"))
				{
					answer.text = dbm.drill_message( 2 );
					if( answer.text.empty() ) answer.text = config.get_keyword( "error_empty_answer" );
					else goto finish;
				}
				else
				{
					if (config.get_binary("available/throw_default_message")) answer = Answerbox(config.get_keyword("default_message"));
					else answer.is_active = false;
					goto finish_nomemory;
				}
				
				break;
			}
		};
	}
	
	finish: // ### Сюда, если нужно запомнить историю и нераспознанную фразу
	logs -> CommitConveer( F, L, "Answer generating finished" );
	
	if (
		config.get_binary("available/save_unsolved") &&
		!std::regex_search(to_single_line(to_lowercase(message.intact_line)), config.get_regex(L"regex/part/forbidden_answer"))
	) dbm.save_unsolved_phrase( message.pure_line_thin ); // Запоминает, что это сообщение не было нормально обработано
	
	finish_nounsolved: // ### Сюда, если нужно запоминать историю, но не нужно запоминать нераспознанную фразу
	
	replace_typical_sequence( answer.text );
	logs -> CommitConveer( F, L, "Typical sequences replaced: " + answer.text );
	
	if (config.get_binary("available/history"))
	{
		historian.add_answer( answer.text ); // Запись ответа ( в оперативку )
		historian.add_message_form( message.message_form );
	}
	
	if( historian.repeat_control( answer.text, "answer" ) && answer.attachment.empty() ) // Проверка на повторение бота
	{
		logs -> CommitConveer( F, L, "Answer repeat found" );
		++ repeats;
		answer.text += " [ " + std::to_string( repeats ) + " ]";
	}
	else repeats = 0;
	
	information.increment( message.message_form );
	historian.close_session(); // Запись результатов в базу. // if (config.get_binary("available/history")) внутри
	dyn_config.write_to_file(); // Запись динамического конфига в файл
	
	finish_nomemory: // ### Сюда, если не нужно запоминать историю и нераспознанную фразу
	
	answer.name = dbm.get_user_name();
	if (!config.one_matches("message_form_change_text_style_forbidden", message.message_form))
		answer = generator.change_text_style(answer);
	
	first_run = false;
	++ total_count;
	
	logs -> CommitConveer( F, L, "	Answer text: \"" + answer.text + "\"" );
	logs -> CommitConveer( F, L, "	Answer attachment: \"" + answer.attachment + "\"" );
	logs -> CommitConveer( F, L, "	*** Conveer main function finish ***" );
	
	return answer;
}

std::wstring Conveer::replace_typical_sequence( std::wstring &target )
{
	replace_in_text ( target, L"{@empty@}", L"" );
	replace_in_text ( target, L"{@proceed@}", L"" );
	replace_in_text ( target, L"{@fancy_text@}", L"" );
	replace_in_text ( target, L"{@bot_name@}", config.get_keyword( L"bot_name_public" ) );
	
	if( target.empty() ) target = L"{@empty@}";
	
	return target;
}
std::wstring Conveer::replace_typical_sequence( std::string &target )
{
	std::wstring temp_target = thin_to_wide_line( target );
	
	replace_typical_sequence( temp_target );
	target = wide_to_thin_line( temp_target );
	return temp_target;
}

std::vector <std::string> Conveer::get_fetcher_keywords( void )
{
	if( config.there_is_element(L"fetcher_keyword") && fetcher_keyword_storage_version != config.get_version() )
	{
		// Список слов для фетчера
		std::vector <std::wstring> fetcher_keyword_storage_wide = config.get_group( L"fetcher_keyword" );
		//   lowercase не делаю, в конфиг писать маленькими буквами
		fetcher_keyword_storage_wide = sort_words( fetcher_keyword_storage_wide );
		fetcher_keyword_storage = wide_to_thin_storage( fetcher_keyword_storage_wide );
		fetcher_keyword_storage_version = config.get_version();
	}
	
	return fetcher_keyword_storage;
}

void Conveer::refresh_config( void ) { config.refresh(); }


} // namespace Convspace
