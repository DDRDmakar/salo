
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
#include "headers/config.h"
#include "headers/parse_tools.h"
#include "headers/exception.h"

namespace Convspace
{

Message_storage::Command_detector::Command_detector(
	Config *config,
	Community_organizer *source_info
)
{
	ready = false;
	if (config == nullptr || source_info == nullptr)
	{
		return;
	}
	this->config = config;
	this->source_info = source_info;
	config_version = 0;
}

Message_storage::Command_detector::~Command_detector(void) {}

Message_storage::Command_detector::Command_node::Command_node(void)
{
	is_regex = false;
	is_part = false;
	active = true;
	icase = true;
}

bool Message_storage::Command_detector::is_ready(void) { return ready; }

void Message_storage::Command_detector::refresh(void)
{
// Бьётся указатель на внешний объект.
// Потому что объект создали, а потом скопировали. Указатель поменялся.
// Сделать message_storage ссылкой.
	if (config == nullptr || source_info == nullptr)
	{
		ready = false;
		return;
	}
	
	YAML::Node config_storage = config->subtree("cmd");
	
	try
	{
		// Delete old command nodes
		alias_storage.clear();
		ready = false;
		
		// Список команд для текущего интерфейса
		std::list<Command_node> temp_alias_storage;
		
		if (!config_storage.IsMap()) return;
		
		for (YAML::const_iterator it1 = config_storage.begin(); it1 != config_storage.end(); ++it1)
		{
			const std::string current_interface = it1->first.as<std::string>();
			if (!config_storage[current_interface] || !config_storage[current_interface].IsMap()) continue;
			
			/*
			 *  command: {
			 *      active: no,                                             default is yes
			 *      part: yes,                                              default is false
			 *      hash: "{@year@}",                                       default without hash (random choice)
			 *      icase: yes,                                             default ignore case is true
			 *      sep:   " "                                              defauld is space
			 *      word: "regex" / [word1, word2, word3, ...]
			 *      command: command1 / [command1, command2, command3, ...]
			 *  }
			 * 
			 */
			
			// Итерируюсь по блокам, каждый под отдельную команду
			for (YAML::const_iterator it2 = config_storage[current_interface].begin(); it2 != config_storage[current_interface].end(); ++it2)
			{
				Command_node current_command; // Command properties storage
				
				// Hash template
				current_command.heh = (it2->second["hash"] && it2->second["hash"].IsScalar()) ? it2->second["hash"].as<std::string>() : "";
				// Command block name
				current_command.ID = it2->first.as<std::string>();
				// If we check part in the beginning of phrase, or full phrase
				current_command.is_part = (
					it2->second["part"] && 
					it2->second["part"].IsScalar() &&
					it2->second["part"].as<bool>()
				);
				// Separator between command beginning (changed) and ending
				// Default separator is space
				current_command.sep = (it2->second["sep"] && it2->second["sep"].IsScalar()) ? thin_to_wide_line(it2->second["sep"].as<std::string>()) : L" ";
				
				// Storage of command aliases
				if (it2->second["word"] && it2->second["command"])
				{
					if (it2->second["word"].IsSequence()) // Detect by list of phrases
					{
						current_command.is_regex = false;
						// Sort by length decreasingly
						current_command.word = 
							sort_words(
								config->get_group(
									L"cmd/" + 
									thin_to_wide_line(current_interface) + 
									L"/" + 
									thin_to_wide_line(it2->first.as<std::string>()) + 
									L"/word"
								)
							);
						
						if (it2->second["icase"] && it2->second["icase"].IsScalar() && !it2->second["icase"].as<bool>()) current_command.icase = false;
						else
						{
							// Ignore case
							current_command.icase = true;
							// Lowercase aliases in list
							for (auto &e : current_command.word) lowercase(e);
						}
					}
					else if (it2->second["word"].IsScalar()) // Detect by regex
					{
						current_command.is_regex = true;
						if (it2->second["icase"] && it2->second["icase"].IsScalar() && !it2->second["icase"].as<bool>())
						{
							current_command.icase = false;
							current_command.reg = std::wregex(thin_to_wide_line(it2->second["word"].as<std::string>()));
						}
						else
						{
							current_command.icase = true;
							current_command.reg = std::wregex(thin_to_wide_line(it2->second["word"].as<std::string>()), std::wregex::icase);
						}
					}
					else continue;
				}
				else continue;
				
				// Storage of formal commands
				if (it2->second["command"])
				{
					if (it2->second["command"].IsSequence())
						current_command.command = config->get_group(
							L"cmd/" + 
							thin_to_wide_line(current_interface) + 
							L"/" + 
							thin_to_wide_line(it2->first.as<std::string>()) + 
							L"/command"
						);
					else if (it2->second["command"].IsScalar())
						current_command.command.push_back(thin_to_wide_line(it2->second["command"].as<std::string>()));
					else continue;
				}
				else continue;
				
				// If command block is active or not
				current_command.active = !(
					it2->second["active"] && 
					it2->second["active"].IsScalar() &&
					!it2->second["active"].as<bool>()
				);
				
				// Add command to list of commands
				temp_alias_storage.push_back(current_command);
			}
			
			alias_storage.emplace(current_interface, temp_alias_storage);
			temp_alias_storage.clear();
		}
		// Command detector is ready
		ready = true;
	}
	catch ( YAML::BadConversion &e )
	{
		error_out( std::string( "ERROR in command converter constructor - node has incomplete format. " ) + e.what() );
		return;
	}
	catch ( YAML::KeyNotFound &e )
	{
		error_out( std::string( "ERROR in command converter constructor - key not found. " ) + e.what() );
		return;
	}
	catch ( YAML::InvalidNode &e )
	{
		error_out( std::string( "ERROR in command converter constructor - invalid node. " ) + e.what() );
		return;
	}
	catch ( YAML::Exception &e )
	{
		error_out( std::string( "ERROR in command converter constructor - " ) + e.what() );
		return;
	}
	catch (std::regex_error &e)
	{
		error_out( std::string( "ERROR in command converter constructor - regex error. " ) + e.what() );
		return;
	}
}

std::wstring Message_storage::Command_detector::convert(std::wstring current_line)
{
	if (!ready) throw Convspace::Exception_config("Critical error: command detector is not ready.");
	
	// Updating configuration
	if (!ready || this->config_version != config->get_version())
	{
		refresh();
		if (ready) this->config_version = config->get_version();
		if (!ready) return current_line;
	}
	
	// Preprocessing command
	// Check emptiness
	if (current_line.empty()) return current_line;
	
	std::wstring answer = current_line;
	
	// удаляет последний символ пунктуации, если он есть
	if (
		!config->get_binary("available/ignore_punctuation_after_command") && 
		current_line.length() > 1 && 
		detect_punctuation(current_line.back())
	) current_line.pop_back();
	
	// Needed interfaces
	std::string cmd_interface_storage[2] = { source_info->get_interface(), "all" };
	
	// Lowercased phrase
	const std::wstring lower_current_line = to_lowercase(current_line);
	// Line to insert command into
	std::wstring preparation_line = current_line;
	
	// Iterate through all needed interfaces
	//for (const auto &current_interface : cmd_interface_storage)
	for (int interface_index = 0; interface_index < 2; ++interface_index)
	{
		const auto &current_interface = cmd_interface_storage[interface_index];
		// If command list is not present for this interface
		if (alias_storage.find(current_interface) == alias_storage.end()) continue;
		
		// Iterate through command blocks
		for(const auto &e : alias_storage.at(current_interface))
		{
			if (!e.active || e.command.empty()) continue;
			
			if (e.is_part) // PART
			{
				if (e.is_regex) // PART REGEX
				{
					// Command pattern
					std::wsmatch intuitive_command;
					// Search
					std::regex_search(current_line, intuitive_command, e.reg);
					if (intuitive_command.size() > 0)
					{
						// Выбираю рандомный элемент из списка команд
						// И записываю его вместо соответствующей части фразы
						const std::wstring current_command_part = intuitive_command.str(0);
						unsigned int words_count = count_words(current_command_part);
						for (unsigned int k = 0; k < words_count; ++k) remove_keyword(preparation_line);
						
						const size_t keynum = e.heh.empty() ? rand() : source_info->gethash(e.heh);
						preparation_line.insert(0, e.command[keynum % e.command.size()] + e.sep);
						answer = preparation_line;
						
						goto fincheck;
					}
				}
				else // PART LIST
				{
					// Iterate through list of phrases
					for (const auto &alias : e.word)
					{
						if (
							e.icase ?
							(begins_with(lower_current_line, alias + L" ") || begins_with(lower_current_line, alias + L"\n") || lower_current_line == alias) :
							(begins_with(current_line, alias + L" ")       || begins_with(current_line, alias + L"\n")       || current_line == alias)
						)
						{
							// Выбираю рандомный элемент из списка команд
							// И записываю его вместо соответствующей части фразы
							unsigned int words_count = count_words(alias);
							for (unsigned int k = 0; k < words_count; ++k) remove_keyword(preparation_line);
							
							const size_t keynum = e.heh.empty() ? rand() : source_info->gethash(e.heh);
							preparation_line.insert(0, e.command[keynum % e.command.size()] + e.sep);
							answer = preparation_line;
							
							goto fincheck;
						}
					}
				}
			}
			else // FULL
			{
				if (e.is_regex) // FULL REGEX
				{
					// Match
					if (std::regex_match(current_line, e.reg))
					{
						const size_t keynum = e.heh.empty() ? rand() : source_info->gethash(e.heh);
						answer = e.command[keynum % e.command.size()];
						goto fincheck;
					}
				}
				else // FULL LIST
				{
					// Iterate through list of phrases
					for (const auto &alias : e.word)
					{
						if (
							(e.icase ? (lower_current_line == alias) : (current_line == alias)) && 
							e.command.size() > 0
						)
						{
							// Выбираю рандомный элемент из списка команд
							const size_t keynum = e.heh.empty() ? rand() : source_info->gethash(e.heh);
							answer = e.command[keynum % e.command.size()];
							goto fincheck;
						}
					}
				}
			}
		}
	}
	
	fincheck:;
	
	clear_spaces(answer);
	return answer;
}

} // namespace
