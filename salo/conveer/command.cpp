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


#include "headers/answer_generator.h"
#include "headers/information.h"
#include "headers/arg_parser.h"
#include "headers/pattern_replacer.h"
#include "headers/parse_tools.h"
#include "headers/exception.h"

#define ARG_ERR   (config->get_keyword("error_incorrect_arguments"))
#define ARG_EMPTY (config->get_keyword("error_empty_arguments"))

namespace Convspace
{

bool Answer_generator::detect_cmd( const std::vector <std::wstring> &current_storage )
{
	logs -> CommitConveer( F, L, "Detecting cmd" );
	
	std::wstring key = current_storage.empty() ? L"" : current_storage[0];
	lowercase( key );
	return( key == L"cmd" );
}

Answerbox Answer_generator::get_help( const std::wstring &key ) // Достать справку
{
	logs -> CommitConveer( F, L, "Getting help from config" );
	return Answerbox( config -> get_keyword( "help_contact_link" ) + "\n" + config -> get_keyword( "help_" + wide_to_thin_line( key ) ) );
}

std::string Answer_generator::call_console(const std::wstring &cmd)
{
	return onCommand(
		wide_to_thin_line(cmd),
		false,
		source_info->get_user(),
		source_info->get_configuration(),
		source_info->get_bot_pointer(),
		source_info->get_interface_unique_identificator()
	);
}

void Answer_generator::changeuserstatus(const std::string &id, const unsigned int status)
{
	dbm->recover_user(id);
	
	logs->CommitConveer(F, L, std::string("SQL QUERY - setting status " + std::to_string(status) + " for " + std::string(id)));
	dbm->DB_simple_query( "UPDATE `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` SET `status` = " + screen_sql(std::to_string(status))  + " WHERE `id` = '"  + screen_sql(id) + "'" );
	
	// Записать статус в пользовательский конфиг
	dbm->recover_user(id); // Если искомый пользователь в резерве сейчас
	const std::string that_user_plain_text = dbm->return_query_one("SELECT `config` FROM `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` WHERE `id` = '" + screen_sql(id) + "' ;");
	if (!that_user_plain_text.empty())
	{
		try
		{
			Config_user that_user_structure(that_user_plain_text);
			that_user_structure.unsafe_tree()["user_status"] = std::to_string(status);
			dbm->DB_simple_query("UPDATE `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` SET `config` = " + that_user_structure.show() + " `id` = '" + screen_sql(id) + "' ;");
		}
		catch (YAML::Exception e) { error_out("Error parsing userconfig. User id is: " + id); }
	}
}

std::string Answer_generator::normalize_id(std::string id)
{
	if (config->get_binary("interfaces_are_multiple")) // Если интерфейсов много
	{
		if (!begins_with(id, source_info->get_bot_name())) throw Convspace::Exception("cross-ban");
	}
	else // Если интерфейс только один, можно сделать коррекцию
	{
		if (!begins_with(id, source_info->get_bot_name())) // Если id не начинается с имени бота
		{
			// Если начинается с имени интерфейса
			if (begins_with(id, source_info->get_interface())) id.insert(0, source_info->get_bot_name() + "_");
			// Если не начинается с имени интерфейса
			else id.insert(0, source_info->get_bot_name_and_interface() + "_");
		}
		// Если начинается с имени бота, но нет имени интерфейса
		else if (!begins_with(id, source_info->get_bot_name_and_interface())) throw Convspace::Exception("arg-err");
	}
	
	return id;
}

// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________


Answerbox Answer_generator::analyse_cmd( void ) // Обработка команд в строке
{
	std::wstring sample_current_line = message->command_line; // Строка в неизменённом виде
	std::vector <std::wstring> sample_current_storage = message -> word_storage_cmd; // Чистый текст для распознавания команд
	Answerbox answer;
	
	if( sample_current_storage.empty() || sample_current_line.empty() ) return Answerbox();
	if( sample_current_storage.size() == 1 ) // Обработка пустого запроса
	{
		logs -> CommitConveer( F, L, "	Empty command" );
		return Answerbox( config -> get_keyword( "error_empty_command" ) );
	}
	
	// Слово, определяющее команду
	const std::wstring cmd = sample_current_storage[1];
	
	// Отрезаю слово "cmd" и командное слово
	remove_keyword( sample_current_line );
	remove_keyword( sample_current_line );
	sample_current_storage.erase( sample_current_storage.begin(), sample_current_storage.begin() + 2 );
	
	const int user_status = source_info -> get_user_status(); // Считываем приоритет для текущего пользователя
	const std::wstring current_line = sample_current_line; // Строка с первозданным видом команды
	const std::vector <std::wstring> current_storage = sample_current_storage; // Хранилище частей команды, разделённых пробелами
	
	try
	{
		// Проверка прав на команды
		
		if (!config->there_is_element("cmd_rights"))
		{
			error_out("Error reading cmd rights config");
			return Answerbox();
		}
		// Есть ли такая команда вообще
		if (!config->subtree("cmd_rights")[wide_to_thin_line(cmd)])
			return Answerbox(config->get_keyword("error_incorrect_command"));
		// Есть ли у пользователя права на эту команду
		if (
			(
				config->subtree("cmd_rights")[wide_to_thin_line(cmd)]["rights"] &&
				!config->one_matches("cmd_rights/" + wide_to_thin_line(cmd) + "/rights", std::to_string(user_status)) // Права
			) ||
			(
				config->subtree("cmd_rights")[wide_to_thin_line(cmd)]["stable"] &&
				source_info->user_is_stable() ^ config->get_binary("cmd_rights/" + wide_to_thin_line(cmd) + "/stable") // Стабильность
			) ||
			(
				std::regex_match(message->command_line, std::wregex(config->get_keyword(L"regex/full/forbidden_command/" + std::to_wstring(user_status)), std::regex::icase))
			)
		) return Answerbox(config->get_keyword("error_user_rights"));
		
		// Генерация ответа
		if (config->subtree("cmd_rights")[wide_to_thin_line(cmd)]["console"] && config->get_binary("cmd_rights/" + wide_to_thin_line(cmd) + "/console"))
			answer = Answerbox(call_console(cmd + (current_line.empty() ? L"" : (L" " + current_line))));
		else answer = analyse_cmd_internal(cmd, current_line, current_storage);
		
		// Нужно ли отвечать на это сообщение
		// default true
		if (config->subtree("cmd_rights")[wide_to_thin_line(cmd)]["noreply"] && config->get_binary("cmd_rights/" + wide_to_thin_line(cmd) + "/noreply"))
			answer.is_active = false;
	}
	catch (YAML::Exception) { return Answerbox(config->get_keyword("error/yaml_error")); }
	
	return answer;
}

// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
	
Answerbox Answer_generator::analyse_cmd_internal( const std::wstring &cmd, const std::wstring &current_line, const std::vector <std::wstring> &current_storage )
{
	const int user_status = source_info -> get_user_status();
	
	// Анализируем аргументы
	const Arg_parser arg_storage(wide_to_thin_line(current_line));
	// ТЕСТ ПАРСЕРА АРГУМЕНТОВ
	/*
	std::cout << "Source string: " << wide_to_thin_line(current_line) << std::endl;
	std::cout << "Unnamed args count: " << arg_storage.arg_unnamed_size() << std::endl;
	auto rfergergnvte = arg_storage.get_args_unnamed();
	for (const auto eflvnwljr : rfergergnvte) std::cout << '	' << wide_to_thin_line(eflvnwljr) << std::endl;
	std::cout << "Named args count:   " << arg_storage.arg_named_size() << std::endl;
	std::cout << "Named arrays count: " << arg_storage.array_named_size() << std::endl;
	*/
	
	if( current_storage.empty() ) // Однословные (элементарные) команды
	{
		
// Список всех имеющихся типов вложений и их количество
		if( cmd == L"attachment_list" )
		{
			logs -> CommitConveer( F, L, "	Cmd - getting attachment list" );
			return Answerbox( wide_to_thin_line( information -> attachment_list() ) );
		}
		
// Информация о себе
		if( cmd == L"me" )
		{
			logs -> CommitConveer( F, L, "	Cmd - info about me" );
			//return Answerbox( wide_to_thin_line( information -> information_about_user() ) );
			std::string temp_ans = to_safestring_16(source_info->userconfig->show());
			replace_in_text( temp_ans, "\n  ", "\n->" );
			return Answerbox( "\n" + temp_ans );
		}
		
// Увидеть текущую дату и время сервера
		if( cmd == L"time" )
		{
			logs -> CommitConveer( F, L, "	Cmd - time" );
			return Answerbox( "\n" + get_datetime( config -> get_keyword( "datetime_answer" ) ) );
		}
				
// Увидеть текущую дату и время сервера
		if( cmd == L"id" )
		{
			logs -> CommitConveer( F, L, "	Cmd - id" );
			return Answerbox( source_info->get_unique_id() );
		}
		
// Выйти из всех режимов
		if (cmd == L"reset")
		{
			logs->CommitConveer( F, L, "Cmd - reset" );
			
			// Исключаю из списков игр
			game_magic_field.stop(); 
			game_word_chain.stop();
			// Выключаю игровой режим в структуре юзера
			source_info->userconfig->editable_subtree()["game"]["gamemode"] = 0;
			// Выключаю шрифты
			source_info->userconfig->editable_subtree()["leet_type"] = 0;
			// Смена имени на стандартное
			// // Записать имя в БД
			dbm->DB_simple_query("UPDATE `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` SET `name` = '" + screen_sql(source_info->get_user_first()) + "' WHERE `id` = '"  + screen_sql( source_info -> get_unique_id()) + "';");
			// // Записать имя в пользовательский конфиг
			source_info->userconfig->unsafe_tree()["name"] = source_info->get_user_first();
			// Ответ
			return Answerbox(config->get_keyword("reset_made"));
		}
		
	}
	
// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
// _______________________________________________________________________________________________________________________________________________
// Команды с параметрами (не элементарные)
	
// Запрос на получение медиа - контента
	if( cmd == L"get" )
	{
		logs -> CommitConveer( F, L, "	Cmd - Getting media content" );
		
		// Проверка наличия аргументов
		if( current_storage.size() != 1 && current_storage.size() != 2 ) return Answerbox( ARG_ERR );
		
		std::string attachment_type, attachment_type_sub;
		attachment_type = wide_to_thin_line( current_storage[0] );
		attachment_type_sub = ( current_storage.size() == 2 ) ? wide_to_thin_line( current_storage[1] ) : "";
		
		std::vector <std::wstring> valid_attachment_type = config -> get_group(L"valid_attachment_type");
		for( int i = 0; i < valid_attachment_type.size(); ++ i )
		{
			if( valid_attachment_type[i] == current_storage[0] )
			{
				// Достаём из базы контент нужного типа и выбираем рандомный
				return dbm -> get_attachment(attachment_type, attachment_type_sub);
			}
		}
		return Answerbox( ARG_ERR );
	}
	
// Запрос на получение текста из конфига.
	if (cmd == L"get_config")
	{
		logs->CommitConveer(F, L, "	Cmd - Getting text from config");
		if (current_line.empty()) return Answerbox(ARG_EMPTY);
		if (!config->there_is_element(wide_to_thin_line(current_line))) return Answerbox(config->get_keyword("error/empty_get_config"));
		Answerbox current_answer("\n" + config->show(wide_to_thin_line(current_line)));
		return current_answer.text.empty() ? Answerbox(config->get_keyword("error/empty_get_config")) : current_answer;
	}
	
// Запрос на получение медиа - контента из конфига.
	if ( cmd == L"get_config_media" )
	{
		/*
		 * -addr ""
		 * -hash ""
		 */
		
		logs -> CommitConveer( F, L, "	Cmd - Getting media content from config" );
		
		if (!arg_storage.arg_named_not_empty("addr")) return Answerbox(ARG_EMPTY);
		
		const size_t keynum = arg_storage.arg_named_not_empty("hash") ? source_info->gethash(arg_storage.get_arg_named("hash")) : rand();
		
		return config->there_is_element("media/" + source_info->get_interface() + "/" + arg_storage.get_arg_named("addr")) ? get_media(arg_storage.get_arg_named("addr"), keynum) : Answerbox(config->get_keyword("error/empty_get_config"));
	}
	
// Запрос на получение текста из определённой части конфига
	if (cmd == L"get_config_text")
	{
		/*
		 * ""
		 * -addr ""
		 * -hash ""
		 * -begin ""
		 * -sep ""
		 * --list "addr1" "addr2" ... "addrN"
		 * 
		 */
		
		logs->CommitConveer(F, L, "	Cmd - Getting text from config");
		
		if (current_line.empty()) return Answerbox(ARG_EMPTY);
		
		const size_t keynum = arg_storage.arg_named_not_empty("hash") ? source_info->gethash(arg_storage.get_arg_named("hash")) : rand();
		
		const std::string addr_begin = arg_storage.arg_named_exists("begin") ? arg_storage.get_arg_named("begin") : "";
		
		const std::string sep = arg_storage.arg_named_exists("sep") ? arg_storage.get_arg_named("sep") : "\n";
		if (sep != "\n" && std::regex_search(to_single_line(to_lowercase(thin_to_wide_line(sep))), config->get_regex(L"regex/part/forbidden_answer")))
			return Answerbox(config->get_keyword("error/forbidden_answer"));
		
		Answerbox current_answer;
		
		// --list "" "" ""
		if (arg_storage.array_named_not_empty("list"))
		{
			for (int i = 0; i < arg_storage.array_named_size("list"); ++i)
			{
				if (!config->there_is_element("media_text/" + addr_begin + arg_storage.get_array_named_element("list", i))) return Answerbox(config->get_keyword("error/empty_get_config"));
				current_answer.text += sep + get_text(addr_begin + arg_storage.get_array_named_element("list", i), keynum) + sep;
			}
		}
		// -addr ""
		else if (arg_storage.arg_named_exists("addr"))
		{
			if (!config->there_is_element("media_text/" + addr_begin + arg_storage.get_arg_named("addr"))) return Answerbox(config->get_keyword("error/empty_get_config"));
			current_answer = sep + get_text(addr_begin + arg_storage.get_arg_named("addr"), keynum);
		}
		// ""
		else
		{
			if (!config->there_is_element("media_text/" + wide_to_thin_line(current_line))) return Answerbox(config->get_keyword("error/empty_get_config"));
			current_answer = get_text(wide_to_thin_line(current_line), keynum);
		}
		return current_answer.text.empty() ? Answerbox(config->get_keyword("error/empty_get_config")) : current_answer;
	}
	
// Достать из базы текст
	if( cmd == L"fetch" )
	{
		/*
		 * -type ""
		 * -sub  ""
		 */
		
		logs -> CommitConveer( F, L, "	Cmd - getting text" );
		
		if (!arg_storage.arg_named_not_empty("type")) return Answerbox( ARG_ERR );
		
		// Достаём из базы текст нужного типа и выбираем рандомный
		return dbm->fetch_text(arg_storage.get_arg_named("type"), arg_storage.get_arg_named("sub"));
	}
	
// Запись в базу медиа - контента
	if( cmd == L"add_attachment" )
	{
		// -type ""
		// -subtype ""
		// -link ""
		// -bot ""
		// -interface ""
		
		logs -> CommitConveer( F, L, "	Cmd - Adding media content" );
		
		// Проверка наличия аргументов
		if (
			!arg_storage.arg_named_not_empty("type") || 
			!arg_storage.arg_named_not_empty("link")
		) return Answerbox( ARG_ERR );
		
		
		// (type should contain interface, but can be without bot name (for all bots))
		// Only creator can add attachments without bot name
		// If "add_answers_as_default" mode is active, everyone can do it
		// If attachment type is not in proper condition
		
		std::string attachment_type = arg_storage.get_arg_named("type");
		
		// Attach interface to attachment type
		if (arg_storage.arg_named_not_empty("interface")) attachment_type.insert(0, arg_storage.get_arg_named("interface") + "_");
		else attachment_type.insert(0, source_info->get_interface() + "_");
		
		// Attach botname to attachment type
		if (arg_storage.arg_named_not_empty("bot") && user_status == 0) attachment_type.insert(0, arg_storage.get_arg_named("bot") + "_");
		else
		{
			if (
				!arg_storage.arg_named_not_empty("bot") || 
				(user_status != 0 && !config->get_binary("teachmode/add_answers_as_default"))
			) attachment_type.insert(0, source_info->get_bot_name() + "_");
		}
		
		logs -> CommitConveer( F, L, "SQL QUERY - add media content" );
		const std::string subtype = (arg_storage.arg_named_exists("subtype")) ? arg_storage.get_arg_named("subtype") : "";
		dbm -> DB_simple_query( "INSERT INTO `{@database_name@}`.`attachment`( `link`, `type`, `type_sub` ) VALUES( '" + screen_sql(arg_storage.get_arg_named("link")) + "', '" + screen_sql(attachment_type) + "', '" + screen_sql(subtype) + "' ) ;" );
		
		// Create answer message text
		Pattern_replacer answer_text(config->get_keyword("answer_template/attachment_added"));
		answer_text.replace("link", arg_storage.get_arg_named("link"));
		answer_text.replace("type", attachment_type);
		answer_text.replace("subtype", subtype);
		
		return Answerbox (answer_text.extract(), arg_storage.get_arg_named("link"), attachment_type);
	}
	
// Удаление из базы медиа - контента
	if( cmd == L"remove_attachment" )
	{
		// -type ""
		// -subtype ""
		// -link ""
		// -bot ""
		// -interface ""
		
		//  LINK  BOT  INTERFACE  TYPE  ---  SUBTYPE
		
		logs -> CommitConveer( F, L, "	Cmd - Removing media content" );
		
		if(!arg_storage.arg_named_not_empty("link") || (!arg_storage.arg_named_not_empty("type") && !config->get_binary("teachmode/add_answers_as_default"))) return Answerbox(ARG_ERR);
		
		std::string remove_link = arg_storage.get_arg_named("link"); // Ссылка
		std::string attachment_type = arg_storage.get_arg_named("type");
		
		// Attach interface to attachment type
		if (arg_storage.arg_named_not_empty("interface")) attachment_type.insert(0, arg_storage.get_arg_named("interface") + "_");
		else attachment_type.insert(0, source_info->get_interface() + "_");
		
		// Attach botname to attachment type
		if (arg_storage.arg_named_not_empty("bot") && user_status == 0) attachment_type.insert(0, arg_storage.get_arg_named("bot") + "_");
		else
		{
			if (
				!arg_storage.arg_named_not_empty("bot") || 
				(user_status != 0 && !config->get_binary("teachmode/add_answers_as_default"))
			) attachment_type.insert(0, source_info->get_bot_name() + "_");
		}
		
		// Внимание - удаляет со всех интерфейсов! // TODO
		logs -> CommitConveer( F, L, "SQL QUERY - remove media content" );
		dbm -> DB_simple_query("DELETE FROM `{@database_name@}`.`attachment` WHERE `link` = '" + remove_link + "'" + std::string((!arg_storage.arg_named_not_empty("type") && config->get_binary("teachmode/add_answers_as_default")) ? "" : " AND `type` = '" + attachment_type + "'") + ";");
		
		return Answerbox(pattern_replace_single(config->get_keyword("answer_template/attachment_deleted"), "link", remove_link));
	}
	
// Изменеие приоритета пользователя( только для высших операторов )
	if( cmd == L"changeuserstatus" )
	{
		logs -> CommitConveer( F, L, "	Cmd - Changing user status" );
		
		if( user_status != 0 || ! source_info -> user_is_stable() ) return Answerbox( config -> get_keyword( "error_user_rights" ) );
		if( current_storage.size() != 2 ) return Answerbox( ARG_ERR );
		
		// Проверка, является ли аргумент числом
		if( ! detect_range_all( current_storage[1], 0x30, 0x39 ) ) return Answerbox( ARG_ERR );
		
		std::string current_user_id = wide_to_thin_line( current_storage[0] ); // id
		std::string current_user_status = wide_to_thin_line( current_storage[1] ); // status
		
		changeuserstatus(current_user_id, std::stoi(current_user_status));
		
		// Формирование ответа в ВК
		std::string answer = "\"" + std::string( current_user_id ) + "\" " + config -> get_keyword( "part_changed_to" ) + " " + std::string( current_user_status ) + " ";
		switch( current_user_status[0] )
		{
			case '0': { answer += config -> get_keyword( "status_creator" ); break;}
			case '1': { answer += config -> get_keyword( "status_admin" ); break;}
			case '2': { answer += config -> get_keyword( "status_user" ); break;}
			case '3': { answer += config -> get_keyword( "status_superadmin" ); break;}
			default:  { answer += config -> get_keyword( "status_unknown" ); break;}
		}
		
		return Answerbox( answer );
	}
	
// Режим обучения
	if( cmd == L"teachmode" )
	{
		logs -> CommitConveer( F, L, "	Cmd - Teaching mode" );
		
		// Проверка формата аргументов
		if
		(
			current_storage.size() != 1 ||
			(
				current_storage[0] != L"on" &&
				current_storage[0] != L"off"
			)
		) return Answerbox( ARG_ERR );
		
		if( current_storage[0] == L"on" ) // Если команда "включить"
		{
			logs -> CommitConveer( F, L, "	Turning teachmode on" );
			
			// Добавляет пользователя в список учителей
			// Выдаёт первый запрос к учителю и справку
			
            // If user already is a teacher
			if (sensei->is_participant())
				return Answerbox(config->get_keyword("teachmode/error/double_participant"));
			
			// If user is in conversation
			else if (source_info->is_conf() && !config->get_binary("teachmode/in_conf"))
				return Answerbox(config->get_keyword("teachmode/error/forbidden_start_in_conf"));
			
			// If user is running any game or changed output mode
			else if (
				!config->get_binary("teachmode/in_game_or_leet") &&
				(user_in_game() || source_info->userconfig->get_keyword("leet_type") != "0")
			) return Answerbox(config->get_keyword("teachmode/error/forbidden_start_in_game_or_leet"));
			
			// Start teachmode
			else return Answerbox(sensei -> start());
		}
		
		if( current_storage[0] == L"off" ) // Если команда "выключить"
		{
			logs -> CommitConveer( F, L, "	Turning teachmode off" );
			
			// Удаляет из списка учителей
			if( sensei -> is_participant() )
			{
				sensei -> remove_from_list(); // Удаляет пользоваетля из списка учителей
				return Answerbox( config -> get_keyword( "teachmode/teachmode_off" ) );
			}
			else return Answerbox( config -> get_keyword( "teachmode/error/not_participant" ) );
		}
	}
	
// Забанить пользователя
	if( cmd == L"ban" )
	{
		/*
		 * "id"
		 * "time"
		 * -reason ""
		 */
		
		logs -> CommitConveer( F, L, "	Cmd - Banning user" );
		
		if(current_storage.empty()) return Answerbox(config->get_keyword("ban_help"));
		
		// Проверка правильности аргументов
		if
		(
			!(
				(arg_storage.arg_unnamed_size() == 2) &&
				detect_range_all(thin_to_wide_line(arg_storage.get_arg_unnamed(1)), 0x30, 0x39)
			)
		) return Answerbox( ARG_ERR );
		
		std::string ban_id = arg_storage.get_arg_unnamed(0); // id
		
		try { ban_id = normalize_id(ban_id); }
		catch (Convspace::Exception &e)
		{
			if (e.what() == "cross-ban") return Answerbox(config->get_keyword("error/cross_ban"));
			else if (e.what() == "arg-err") return Answerbox(ARG_ERR);
			else  return Answerbox(config->get_keyword("error/unknown_exception"));
		}
		
		logs->CommitConveer(F, L, std::string("SQL QUERY - banning user \"" + ban_id + "\" for " + arg_storage.get_arg_unnamed(1) + " hours."));
		
		int ban_id_status = source_info->get_user_status(ban_id);
		if( ban_id_status == 0 || ban_id_status == 1 || ban_id_status == 3) return Answerbox(config->get_keyword("error_ban_admin" ));
		else
		{
			if (arg_storage.get_arg_unnamed(1).length() > 7) return Answerbox(config->get_keyword("error/out_of_range"));
			try { security->ban(std::stoi(arg_storage.get_arg_unnamed(1)), ban_id, arg_storage.get_arg_named("reason")); }
			catch (std::out_of_range) { return Answerbox(config->get_keyword("error/out_of_range")); }
			
			// Create answer message text
			Pattern_replacer answer_text(config->get_keyword("answer_template/banned"));
			answer_text.replace("id", ban_id);
			answer_text.replace("time", arg_storage.get_arg_unnamed(1));
			return Answerbox(answer_text.extract());
		}
	}

// Забанить себя
	if( cmd == L"banme" )
	{
		/*
		* -time n
		*/
		logs->CommitConveer(F, L, "	Cmd - Banning user");
		logs->CommitConveer(F, L, std::string("SQL QUERY - banning user " + source_info->get_unique_id()));
		
		security->ban(
			(arg_storage.arg_named_not_empty("time") && detect_range_all(thin_to_wide_line(arg_storage.get_arg_named("time")), 0x30, 0x39)) ?
			std::stoi((arg_storage.get_arg_named("time"))) :
			config->get_coefficient(L"ban_time")
		);
		
		// Формирование ответа в ВК
		return Answerbox( "\"" + source_info -> get_unique_id() + "\" " + config -> get_keyword( "part_banned" ) );
	}
	
// Разбанить пользователя
	if( cmd == L"unban" )
	{
		/*
		 * "id"
		 */
		
		logs -> CommitConveer( F, L, "	Cmd - Unbanning user" );
		
		if (current_storage.size() != 1) return Answerbox(ARG_ERR);
		
		std::string unban_id = wide_to_thin_line(current_storage[0]);
		
		try { unban_id = normalize_id(unban_id); }
		catch (Convspace::Exception &e)
		{
			if (e.what() == "cross-ban") return Answerbox(config->get_keyword("error/cross_ban"));
			else if (e.what() == "arg-err") return Answerbox(ARG_ERR);
			else  return Answerbox(config->get_keyword("error/unknown_exception"));
		}
		
		logs->CommitConveer(F, L, std::string("SQL QUERY - unbanning user \"" + unban_id + "\""));
		security->unban(unban_id); // id
		
		// Формирование ответа
		return Answerbox(pattern_replace_single(config->get_keyword("answer_template/unbanned"), "id", unban_id));
	}
	
// Редактирование базы ответов
	if( cmd == L"db" ) // TODO Заменить квадратные скобки на кавычки, как и в остальных командах
	{
		logs -> CommitConveer( F, L, "Cmd - Change database" );
		
		/*
		 * add
		 * "phrase"
		 * "answer"
		 * -bot "botname"
		 */
		
		/*
		 * remove
		 * -p "phrase"
		 * -a "answer"
		 * -bot "botname"
		 * #all
		 */
		
		/*
		 * look
		 * phrase / answer
		 * "text"
		 * -bot "botname"
		 * #part
		 */
		
		// If command is empty
		if (arg_storage.arg_unnamed_size() < 1) return Answerbox(ARG_EMPTY);
		
		std::string botlabel = config->get_binary("teachmode/add_answers_as_default") ? "" : source_info->get_bot_name();
		if (user_status == 0 && arg_storage.arg_named_exists("bot")) botlabel = arg_storage.get_arg_named("bot");
		
		if
		( // Добавить ответы в базу через чат
			arg_storage.get_arg_unnamed(0) == "add" &&
			arg_storage.arg_unnamed_not_empty(1) &&
			arg_storage.arg_unnamed_not_empty(2)
		)
		{
			std::wregex forbidden_teachmode_phrases = config->get_regex(L"regex/full/forbidden_teachmode_phrases");
			
			if // Проверка на запрещённые слова
			(
				( // Если фраза или ответ содержат запрещённую фразу И у пользователя нет прав на их добавление
					std::regex_match(to_single_line(to_lowercase(thin_to_wide_line(arg_storage.get_all_args_line()))), config->get_regex(L"regex/full/forbidden_teachmode_phrases")) ||
					std::regex_search(to_single_line(to_lowercase(thin_to_wide_line(arg_storage.get_all_args_line()))), config->get_regex(L"regex/part/forbidden_answer"))
				) && !config->one_matches("answer_db_add_forbidden_words_rights", std::to_string(user_status))
			) return Answerbox(config->get_keyword("teachmode/error/forbidden_phrase"));
			
			dbm->DB_simple_query("INSERT INTO `{@database_name@}`.`typical_questions`(`keyphrase`, `answer`, `attitude_param`, `id`, `bot`) VALUES('" + screen_sql(to_lowercase(arg_storage.get_arg_unnamed(1))) + "', '" + screen_sql(arg_storage.get_arg_unnamed(2)) + "', '0', NULL, '" + screen_sql(botlabel) + "') ;" );
			return Answerbox(config->get_keyword("an_db_answer_added"));
		}
		
		else if // Удалить ответы из базы через чат
		(arg_storage.get_arg_unnamed(0) == "remove" && (arg_storage.arg_named_not_empty("p") || arg_storage.arg_named_not_empty("a")))
		{
			const std::string condition = "1" 
				+ std::string(arg_storage.arg_named_not_empty("p") ? " AND `keyphrase` = '" + screen_sql(to_lowercase(arg_storage.get_arg_named("p"))) + "'" : "")
				+ std::string(arg_storage.arg_named_not_empty("a") ? " AND `answer` = '"    + screen_sql(arg_storage.get_arg_named("a")) + "'" : "")
				+ " AND `bot` = '" + screen_sql(botlabel) + "';";
			// If answer exists
			const int n_answers_found = dbm->return_query_count("SELECT COUNT(*) FROM `{@database_name@}`.`typical_questions` WHERE " + condition + ";");
			if (n_answers_found > 1 && !(arg_storage.arg_named_not_empty("p") && arg_storage.arg_named_not_empty("a")) && !arg_storage.arg_unnamed_exists("#all"))
				return Answerbox(config->get_keyword("an_db_answer_doubled") + "\n" + dbm->return_query_list( "SELECT `keyphrase`, `answer` FROM `{@database_name@}`.`typical_questions` WHERE " + condition + ";" ) );
			if (n_answers_found > 0)
			{
				dbm -> DB_simple_query( "DELETE FROM `{@database_name@}`.`typical_questions` WHERE " + condition + ";" );
				return Answerbox(config->get_keyword("an_db_answer_deleted"));
			}
			else return Answerbox(config->get_keyword("an_db_answer_not_found"));
		}
		
		// Посмотреть записи в базе с таким ответом
		else if (arg_storage.get_arg_unnamed(0) == "look" && arg_storage.arg_unnamed_size() >= 3 && arg_storage.arg_unnamed_not_empty(1))
		{
			const std::string current_column_name = arg_storage.get_arg_unnamed(1) == "phrase" ? "keyphrase" : "answer";
			return Answerbox(dbm->return_query_list("SELECT `bot`, `keyphrase`, `answer` FROM `{@database_name@}`.`typical_questions` WHERE `" + current_column_name + "`" + std::string(arg_storage.arg_unnamed_exists("#part") ? " LIKE '%" + screen_sql(arg_storage.get_arg_unnamed(2)) + "%'" : " = '" + screen_sql(arg_storage.get_arg_unnamed(2)) + "'") + " AND (`bot` = '' OR `bot` = '" + screen_sql(botlabel) + "') LIMIT " + std::to_string(config -> get_coefficient("query_limit_small")) + ";"));
		}
		
		else return Answerbox(ARG_ERR);
	}
	
// Рандом в заданной области значений
	if( cmd == L"random" )
	{
		/*
		 * "" - begin
		 * "" - end
		 * -count "" (1 - 100. default: 1)
		 */
		
		logs -> CommitConveer( F, L, "	Cmd - Random" );
		// Проверка аргументов, числа ли это
		if (
			(
				arg_storage.arg_unnamed_size() >= 2 && (
					! detect_range_all(thin_to_wide_line(arg_storage.get_arg_unnamed(0)), 0x30, 0x39) ||
					! detect_range_all(thin_to_wide_line(arg_storage.get_arg_unnamed(1)), 0x30, 0x39)
				)
			) || (
				arg_storage.arg_named_not_empty("count") && 
				! detect_range_all(thin_to_wide_line(arg_storage.get_arg_named("count")), 0x30, 0x39)
			)
		) return Answerbox(ARG_ERR);
		
		if (arg_storage.arg_unnamed_size() < 2)
		{
			return Answerbox( (rand() % 2) ? config -> get_keyword( "random_true" ) : config -> get_keyword( "random_false" ) );
		}
		
		// Проверка на переполнение
		if(
			arg_storage.get_arg_unnamed(0).length() > 6 ||
			arg_storage.get_arg_unnamed(1).length() > 6 ||
			(arg_storage.arg_named_not_empty("count") && arg_storage.get_arg_named("count").length() > 2)
		) return Answerbox(config -> get_keyword("error_big_value"));
		
		//Перевод в int
		const unsigned int range_beg = std::stoul(arg_storage.get_arg_unnamed(0).c_str(), NULL, 10);
		const unsigned int range_end = std::stoul(arg_storage.get_arg_unnamed(1).c_str(), NULL, 10);
		
		const unsigned int cnt = arg_storage.arg_named_not_empty("count") ? std::stoul(arg_storage.get_arg_named("count").c_str(), NULL, 10) : 1;
		
		// Начало области значений должно быть меньше конца области
		if(range_beg >= range_end) return Answerbox(ARG_ERR);
		// Ответ
		else
		{
			std::stringstream answerline;
			for (unsigned i = 0; i < cnt; ++i)
				answerline << " " << std::to_string(rand() % (range_end-range_beg) + range_beg);
			
			return Answerbox(config -> get_keyword( "part_result" ) + answerline.str());
		}
	}
	
// Сделать большую надпись символами
	if( cmd == L"write" )
	{
		logs -> CommitConveer( F, L, "	Cmd - Writing big sign" );
		
		if( current_line.empty() ) return Answerbox( ARG_EMPTY );
		
		std::wstring phrase = current_line.substr( 0, config -> get_coefficient( L"len_max_big_text" ) );
		
		int fonts_amount, cur_font;
		fonts_amount = ( config -> get_group( L"font_letter" ) ).size();
		if( fonts_amount == 0 ) return Answerbox( config -> get_keyword( "error_incorrect_config" ) );
		cur_font = rand() % fonts_amount;
		std::wstring target = L"\n" + draw_text_with_symbols( phrase, config -> get_group( L"font_letter", cur_font ).c_str(), config -> get_group( L"font_background", cur_font ).c_str() );
		
		return Answerbox( wide_to_thin_line( target ) );
	}
	
// Повторить фразу
	if( cmd == L"say" )
	{
		logs -> CommitConveer( F, L, "	Cmd - Repeating" );
		
		if( current_line.empty() ) return Answerbox( ARG_EMPTY );
		
		std::wstring phrase = current_line;
		if (std::regex_search(to_single_line(to_lowercase(phrase)), config->get_regex(L"regex/part/forbidden_answer"))) return Answerbox(config->get_keyword("error/forbidden_answer"));
		if (phrase.length() >= config -> get_coefficient( L"len_max_mes" ) ) phrase = phrase.substr( 0, config -> get_coefficient( L"len_max_mes" ) );
		
		return Answerbox( wide_to_thin_line( phrase ) );
	}
	
// Изменить обращение бота к тебе
	if( cmd == L"callme" )
	{
		logs -> CommitConveer( F, L, "	Cmd - changing username" );
		
		if( current_line.length() > config -> get_coefficient( L"len_max_username" ) ) return Answerbox( config -> get_keyword( "error_long_name" ) );
		
		if (std::regex_search(to_single_line(to_lowercase(current_line)), config->get_regex(L"regex/part/forbidden_answer")))
			return Answerbox( config -> get_keyword( "error/forbidden_answer" ) );
		
		std::string thin_phrase = wide_to_thin_line( current_line );
		if
		(
			// Разрешено повторение имён ИЛИ новое имя - пустая строка ИЛИ
			config->get_binary("available_repeat_name") || current_line.empty() ||
			// Такое имя не существует у другого пользователя ИЛИ
			dbm -> return_query_count( "SELECT count(*) FROM `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` WHERE `name` <> '' AND `name` = '" + screen_sql( thin_phrase ) + "' AND `id` LIKE '" + screen_sql( source_info -> get_bot_name_and_interface() ) + "%' AND `id` <> '" + screen_sql( source_info -> get_unique_id() ) + "' ;" ) <= 0 ||
			// У этого пользователя настоящее имя == phrase
			dbm -> return_query_count( "SELECT count(*) FROM `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` WHERE ( `first` LIKE '" + screen_sql( thin_phrase ) + "' OR `last` LIKE '" + screen_sql( thin_phrase ) + "' ) AND `id` = '" + screen_sql( source_info -> get_unique_id() ) + "' ;" )
		)
		{
			// Записать имя в БД
			dbm -> DB_simple_query( "UPDATE `{@database_name@}`.`" + std::string(source_info->user_is_stable() ? "user" : "web_user") + "` SET `name` = '" + screen_sql( thin_phrase ) + "' WHERE `id` = '"  + screen_sql( source_info -> get_unique_id() ) + "';" );
			// Записать имя в пользовательский конфиг
			source_info -> userconfig->unsafe_tree()["name"] = thin_phrase;
			return Answerbox( std::string( config -> get_keyword( "part_username" ) + " \"" + source_info -> get_unique_id() + "\" " + config -> get_keyword( "part_changed_to" ) + " \"" + thin_phrase + "\"" ) );
		}
		else return Answerbox( config -> get_keyword( "error_repeat_name" ) );
	}
	
// Получить справку из интернета
	if( cmd == L"description" )
	{
		logs -> CommitConveer( F, L, "	Cmd - getting wiki info" );

		if (current_line.empty() ) return Answerbox(ARG_EMPTY);
		else if (wide_to_thin_line(current_line).length() > config->get_coefficient("len_max_description_query")) return Answerbox(config->get_keyword("error/too_long_description_query"));
		else
		{
			Answerbox answer;
			const std::string wiki_answer = wide_to_thin_line(get_description(current_line));
			if (wiki_answer.empty()) answer.text = config->get_keyword("error_empty_description");
			else answer.text = "\n" + wiki_answer;
			return answer;
		}
	}
	
// Генерировать текст из мудрённых символов
	if( cmd == L"fancytext" )
	{
		logs -> CommitConveer( F, L, "	Cmd - generating fancy text" );
		
		if( current_line.empty() ) return Answerbox( ARG_EMPTY );
		std::wstring answer = generate_fancy_text(current_line);
		if (std::regex_search(to_single_line(to_lowercase(current_line)), config->get_regex(L"regex/part/forbidden_answer"))) return Answerbox(config->get_keyword("error/forbidden_answer"));
		else return Answerbox(wide_to_thin_line(answer));
	}
	
// Капсула с посланием
	if( cmd == L"capsule" )
	{
		logs -> CommitConveer( F, L, "	Cmd - saving capsule" );
		
		if (!current_line.empty() && std::regex_search(to_single_line(to_lowercase(current_line)), config->get_regex(L"regex/part/forbidden_answer"))) return Answerbox(config->get_keyword("error/forbidden_answer"));
		
		Answerbox answer( ( current_line.empty() ? config -> get_keyword( "capsule_prefix_empty" ) : config -> get_keyword( "capsule_prefix" ) ) + "\n" );
		
		answer.text += "{\n";
		for( auto e : capsule ) answer.text += wide_to_thin_line( std::get <0> ( e ) ) + ": [" + wide_to_thin_line( std::get <1> ( e ) ) + "]\n";
		answer.text += "}\n";
		
		if( !current_line.empty() ) capsule.push_front(
			std::make_pair(
				thin_to_wide_line( dbm -> get_user_name() ),
				current_line.substr( 0, config -> get_coefficient( L"len_max_mes" ) )
			)
		);
		if( !capsule.empty() && capsule.size() > config -> get_coefficient( "max_count_capsule_messages" ) ) capsule.pop_back();
		return answer;
	}
	
// Транслит
	if( cmd == L"translit" )
	{
		logs -> CommitConveer( F, L, "	Cmd - generating translit" );
		
		if( current_line.empty() ) return Answerbox( ARG_EMPTY );
		if (std::regex_search(to_single_line(to_lowercase(current_line)), config->get_regex(L"regex/part/forbidden_answer"))) return Answerbox(config->get_keyword("error/forbidden_answer"));
		else return Answerbox( wide_to_thin_line( generate_translit( L"translit_letters", to_lowercase( current_line ) ) ) );
	}
	
// Спонсорские команды( переход в консоль )
	if( cmd == L"sponsor" )
	{
		logs -> CommitConveer( F, L, "	Cmd - sponsor statistics" );
		
		if( current_line.empty() ) return Answerbox( ARG_EMPTY );
		else return Answerbox(
			onCommand(
				wide_to_thin_line(
					cmd +
					( current_line.empty() ? L"" : ( L" " + current_line ))
				),
				false,
				source_info->get_user(),
				source_info->get_configuration(),
				source_info->get_bot_pointer(),
				source_info->get_interface_unique_identificator()
			)
		);
	}
	
// Игры
	if( cmd == L"play" )
	{
		logs -> CommitConveer( F, L, "Cmd - start game" );
		
		if( current_storage.empty() ) return Answerbox( ARG_EMPTY );
		else if( current_storage[0] == L"stop" ) // Останов игр
		{
			if( !user_in_game() ) return Answerbox( config -> get_keyword( "game_error_not_participant" ) );
			game_magic_field.stop();
			game_word_chain.stop();
			return Answerbox( config -> get_keyword( "game_stop" ) );
		}
		else if( user_in_game() ) return Answerbox( config -> get_keyword( "game_error_double_participant" ) ); // Если пользователь уже в игровом режиме
		else if( current_storage[0] == L"guess_word" ) return Answerbox( wide_to_thin_line( game_magic_field.start() ) ); // Игра "угадай слово"
		else if( current_storage[0] == L"word_chain" ) return Answerbox( wide_to_thin_line( game_word_chain.start() ) ); // Игра "города"
		else return Answerbox( ARG_ERR );
	}
	
// Наспамить много текста
	if( cmd == L"spam" )
	{
		logs -> CommitConveer( F, L, "	Cmd - spam" );
		
		if( current_line.empty() ) return Answerbox( ARG_EMPTY );
		if (std::regex_search(to_single_line(to_lowercase(current_line)), config->get_regex(L"regex/part/forbidden_answer"))) return Answerbox(config->get_keyword("error/forbidden_answer"));
		return Answerbox( wide_to_thin_line( generate_spam( current_line, config -> get_coefficient( "len_max_spam_message" ) ) ) );
	}
	
// Изменить шрифт бота
	if ( cmd == L"changeleet" )
	{
		logs -> CommitConveer( F, L, "	Cmd - Changing leet" );
		
		if ( // Проверка, является ли аргумент числом
			current_storage.size() != 1 ||
			!detect_range_all( current_storage[0], 0x30, 0x39 ) ||
			!config->one_matches(L"valid_leet_type", current_storage[0])
		) return Answerbox( ARG_ERR );
		
		try { source_info->userconfig->unsafe_tree()["leet_type"] = std::stoi(current_storage[0]); }
		catch  (std::out_of_range) { return Answerbox(config->get_keyword("error/out_of_range")); }
		
		// Формирование ответа в ВК
		return Answerbox(
			config->get_keyword("leet_changed") + " " +
			wide_to_thin_line( current_storage[0] ) + "\n" + 
			config->get_keyword("leet_changed_2")
		);
	}
	
// Закодировать текст в HEX сущности
	if (cmd == L"encode")
	{
		
		/*
		 *  ""
		 */
		
		logs -> CommitConveer(F, L, "	Cmd - spam");
		
		if (arg_storage.arg_unnamed_size() != 1) return Answerbox(ARG_ERR);
		if (std::regex_search(to_single_line(to_lowercase(current_line)), config->get_regex(L"regex/part/forbidden_answer"))) return Answerbox(config->get_keyword("error/forbidden_answer"));
		return Answerbox("\n" + to_safestring(arg_storage.get_arg_unnamed(0), 0));
	}
	
// Обработка изображений
	if (cmd == L"image")
	{
		// // // // cmd image -filter flip -ver top -hor no
		// cmd image -preset pixeled -text "hello world"
		
		logs -> CommitConveer( F, L, "	Cmd - image" );
		
		try
		{
			Answerbox answer;
			answer.type = "edited image";
			answer.is_active = config->get_binary("available/answer_cmd_image");
			
			// If we should take preset from folder
			if (arg_storage.arg_named_exists("preset"))
			{
				// New node with config
				Config_yaml *filter_node = new Config_yaml(config->show("image_preset/" + arg_storage.get_arg_named("preset")));
				
				// Check, if we have any filters present in preset
				if (!filter_node->there_is_element("filters")) return Answerbox(config->get_keyword("error/image_no_filters_found"));
				
				// Get index of the main filter
				// If mainfilter index is defined and it is a valid index, use it.
				// If we have only one filter, index is 0.
				// Else main_filter_index = -1
				const int main_filter_index = (filter_node->there_is_element("mainfilter") && filter_node->get_coefficient("mainfilter") < filter_node->get_group_size("filters")) ?
				filter_node->get_coefficient("mainfilter") : filter_node->get_group_size("filters") == 1 ? 0 : -1;
				
				// If we use only one filter, or main filter is defined
				if (main_filter_index != -1)
				{
					// Add extra information to node
					
					
					// Add unnamed parameters to filter
					
					// Get name of filter
					const std::string filter_name = filter_node->subtree("filters")[main_filter_index].begin()->first.as<std::string>();
					// Values of parameters to change
					const std::vector<std::string> unnamed_array = arg_storage.get_array_unnamed();
					// Iterate through filter's parameters
					YAML::iterator it = filter_node->editable_subtree("filters")[main_filter_index][filter_name].begin();
					for (const auto &e : unnamed_array)
					{
						// If loop reached end of parameters list
						if (it == filter_node->subtree("filters")[main_filter_index][filter_name].end())
						{
							answer.text = config->get_keyword("error/too_many_arguments");
							answer.is_active = true;
							return answer;
						}
						// If parameter is immutable
						if (begins_with(it->first.as<std::string>(), "_"))
						{
							answer.text = config->get_keyword("error/image_immutable_parameter");
							answer.is_active = true;
							return answer;
						}
						
						it->second = e;
						++it;
					}
					
					// Add named parameters to filter
					
					// Keys of parameters to change
					const std::vector<std::string> keys_array = arg_storage.get_keys_of_single();
					// Iterate through keys
					for (const auto &e : keys_array)
					{
						// If parameter is immutable
						if (begins_with(e, "_"))
						{
							answer.text = config->get_keyword("error/image_immutable_parameter");
							answer.is_active = true;
							return answer;
						}
						// If key is not "preset" (reserved), change value.
						if (e != "preset")
							filter_node->editable_subtree("filters")[main_filter_index][filter_name][e] = arg_storage.get_arg_named(e);
					}
				}
				
				// Send node to SCRS
				answer.text = call_console(L"krapiva " + thin_to_wide_line(filter_node->show()));
				delete filter_node;
				return answer;
			}
			
		}
		catch (YAML::Exception) { return Answerbox(config->get_keyword("error/yaml_error")); }
		
		return Answerbox(ARG_ERR);
	} // cmd image
	
// Выдать другому пользователю адмиинские права или разжаловать
	if (cmd == L"admin")
	{
		logs->CommitConveer( F, L, "Cmd - admin" );
		
		if (!arg_storage.arg_named_exists("id")) return Answerbox(ARG_ERR);
		
		std::string current_id;
		try { current_id = normalize_id(arg_storage.get_arg_named("id")); }
		catch (Convspace::Exception &e)
		{
			if (e.what() == "cross-ban") return Answerbox(config->get_keyword("error/cross_ban"));
			else if (e.what() == "arg-err") return Answerbox(ARG_ERR);
			else  return Answerbox(config->get_keyword("error/unknown_exception"));
		}
		
		if (!begins_with(current_id, source_info->get_bot_name())) return Answerbox(ARG_ERR);
		
		const int current_status = arg_storage.arg_unnamed_exists("u") ?
		2 : ((arg_storage.arg_unnamed_exists("++")) ? 3 : 1);
		
		changeuserstatus(current_id, current_status);
		
		// Create answer message text
		Pattern_replacer answer_text(config->get_keyword("answer_template/changeuserstatus"));
		answer_text.replace("id", current_id);
		answer_text.replace("status", std::to_string(current_status));
		switch(current_status)
		{
			case 1:  { answer_text.replace("statusname", config->get_keyword("status_admin")); break; }
			case 2:  { answer_text.replace("statusname", config->get_keyword("status_user")); break; }
			case 3:  { answer_text.replace("statusname", config->get_keyword("status_superadmin")); break; }
			default: { answer_text.replace("statusname", config->get_keyword("status_unknown")); break; }
		}
		return Answerbox(answer_text.extract());
	}
	
	//_________________________________________________________________________________________
	
	command_finish:;
	// Если не удалось определить команду
	return Answerbox( config -> get_keyword( "error_incorrect_command" ) );
}

} // namespace Convspace
