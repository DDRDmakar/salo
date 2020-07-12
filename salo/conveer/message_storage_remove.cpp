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

#include "headers/message_storage.h"
#include "headers/parse_tools.h"

namespace Convspace
{


// Удаление пунктуации
std::wstring Message_storage::replace_and_get_punctuation( std::wstring &current_string )
{
	std::wstring temp_punctuation;
	
	for( int i = 0; i < current_string.length(); ++ i ) // Пробегает по всем символам
	{
		if( detect_punctuation( current_string[i] ) )
		{
			temp_punctuation += current_string[i]; // Запись символа в хранилище знаков пунктуации
			current_string.replace( i, 1 , L" " ); // Замена символа на пробел
		}
	}
	
	clear_spaces( current_string ); // Удаление лишних пробелов
	return temp_punctuation;
}

std::wstring Message_storage::replace_punctuation( const std::wstring &current_string )
{
	std::wstring changed_line = current_string;
	replace_and_get_punctuation( changed_line );
	return changed_line;
}

// Удалить слишком короткие слова
void Message_storage::remove_short_words( std::vector <std::wstring> &current_storage ) // Удаляет из хранилища слова меньше N символов длиной
{
	for ( int i = 0; i < current_storage.size(); ++ i )
	{
		if
		(
			config -> get_binary( "available_delete_short_words" ) &&
			current_storage[i].length() <= config -> get_coefficient( L"len_max_delete_short_words" ) &&
			current_storage.size() >= config -> get_coefficient( L"len_min_words_del_short_words" )
		)
		{
			current_storage.erase( current_storage.begin() + i );
			-- i;
		}
	}
}

// Вырезать слова - паразиты
void Message_storage::remove_parasite_words( std::wstring &current_string ) // Удаление ненужных слов
{
	int i, j;
	std::vector < std::wstring > keyword_storage = config -> get_group( L"parasite_word" ); // Поиск слов - паразитов
	
	if( current_string.empty() ) return;
	clear_spaces( current_string );
	
	for ( i = 0; i < keyword_storage.size(); ++ i )
	{
		remove_first( current_string, std::wstring( keyword_storage [i] + std::wstring( L" " ) ) );
		remove_first( current_string, std::wstring( keyword_storage [i] + std::wstring( L"," ) ) );
	}
	
	clear_spaces( current_string );
}

// Удаление ключевого слова ( обращения к боту )
void Message_storage::remove_primary_keyword ( std::string &current_string )
{
	logs -> CommitConveer( F, L, "Removing primary keyword" );
	
	// Контейнеры для этой функции
	// keyword storage должен быть lowercase и отсортирован по убыванию длины
	
	std::string current_string_temp = current_string; // Строка в нормальнов ыиде
	std::string current_string_temp_lowercase = to_lowercase(current_string_temp); // Строка в нижнем регистре
	
	std::vector <std::string> keyword_storage = config -> get_group("primary_symbol"); // Поиск символов, предшествующих обращению
	keyword_storage = wide_to_thin_storage(sort_words(thin_to_wide_storage(keyword_storage))); // Сортировка по убыванию длины ключевого слова
	
	// Отрезание символов, предшествующих обращению
	for (auto i : keyword_storage)
	{
		if(begins_with(current_string_temp_lowercase, i))
		{
			current_string_temp.erase(0, i.length());
			current_string_temp_lowercase.erase(0, i.length()); // Для того, чтобы второй цикл нормально искал
			clear_spaces(current_string_temp);
			clear_spaces(current_string_temp_lowercase);
			break; // Если найдено, то удаляет и завершает цикл
		}
	}
	
	// TODO vk group
	
	keyword_storage = config->get_group("primary_keyword"); // Поиск обращения
	keyword_storage = wide_to_thin_storage(sort_words(thin_to_wide_storage(keyword_storage))); // Сортировка по убыванию длины ключевого слова
	bool primary_keyword_found = false;
	for(auto i : keyword_storage)
	{
		if(begins_with(current_string_temp_lowercase, i))
		{
			current_string_temp.erase( 0, i.length() );
			clear_spaces( current_string_temp );
			while(!current_string_temp.empty() && (current_string_temp.front() == ' ' || current_string_temp.front() == ','))
				current_string_temp.erase(current_string_temp.begin());
			clear_spaces( current_string_temp );
			primary_keyword_found = true;
			break; // Если найдено, то удаляет и завершает цикл
		}
	}
	
	if (non_bot_name_storage_version != config->get_version())
	{
		// Синхронизация списка non_bot_name_storage с конфигом
		const std::vector<std::wstring> fetcher_keyword_storage = config->get_group(L"fetcher_keyword");
		const std::vector<std::wstring> primary_keyword_storage = config->get_group(L"primary_keyword");
		for (auto e1 : fetcher_keyword_storage) // Итерируюсь по ключевым словам фетчера сообщений
		{
			// Сравниваю их со словами, которые отрезает от сообщения бот
			int e2;
			for (e2 = 0; e2 < primary_keyword_storage.size(); ++e2) if (begins_with(primary_keyword_storage[e2], e1)) break;
			// Если ни с одним не совпало, значит это слово - не имя бота.
			if (e2 == primary_keyword_storage.size()) non_bot_name_storage.push_back(e1);
		}
		non_bot_name_storage_version = config->get_version();
	}
	
	message_fetched_without_bot_name = false;
	if (!primary_keyword_found)
	{
		// Проверяю, начинается ли сообщение со слова фетчера, которое не отрезается ботом
		for (auto &e : non_bot_name_storage) if (begins_with(thin_to_wide_line(current_string_temp_lowercase), e))
		{
			// Если да, то запоминаю это
			message_fetched_without_bot_name = true;
			break;
		}
	}
	
	current_string = current_string_temp;
}

// Удаление слова из строки (узкое применение)
void Message_storage::remove_word( std::wstring& current_string, const std::wstring &target )
{
	clear_spaces( current_string );
	if( current_string.length() < target.length() * config -> get_coefficient ( L"len_min_remove_word" ) ) return;
	
	replace_from_text( current_string, target );
	replace_from_text( current_string, L"  ", L" " ); // Удаление двойных пробелов
	
	clear_spaces( current_string ); // Удаление оставшихся пробелов
}

// Удаление первого слова в сообщении
void Message_storage::remove_first( std::wstring& current_string, const std::wstring &target )
{
	clear_spaces( current_string );
	if( current_string.length() < target.length() * config -> get_coefficient ( L"len_min_remove_word" ) ) return;
	if( current_string.compare( 0, target.length(), target ) == 0 ) current_string.erase( 0, target.length() );
	
	clear_spaces( current_string ); // Удаление оставшихся пробелов
}


} // namespace Convspace
