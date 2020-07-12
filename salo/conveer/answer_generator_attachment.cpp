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
#include "headers/parse_tools.h"

namespace Convspace
{

// Ответ на вложения
Answerbox Answer_generator::answer_attachment( const std::string &current_attachment )
{
	logs -> CommitConveer( F, L, std::string( "Attachments detected: " + current_attachment ) );
	
	if( current_attachment.empty() || current_attachment == "null" )
		return Answerbox( config -> get_keyword( "error_incorrect_attachment_type" ) );

	if( current_attachment == "photo" ) return dbm -> get_attachment( "photo" );

	if( current_attachment == "audio" ) return dbm -> get_attachment( "audio" );

	if( current_attachment == "video" ) return dbm -> get_attachment( "video" );

	if( current_attachment == "doc" || current_attachment == "document" )
		return dbm -> get_attachment( "photo" );
	
	if( current_attachment == "sticker" )
		return Answerbox( make_smile() );
	
	if( current_attachment == "wall" || current_attachment == "wall_reply" )
		return Answerbox( make_smile() );
	
	if( current_attachment == "link" ) return dbm -> get_attachment( "photo" );
	
	if( current_attachment == "gift" ) return dbm -> get_attachment( "photo" );
	
	if( current_attachment == "fwd_messages" ) return Answerbox( make_smile() );
	
	if( current_attachment == "voice" ) return dbm -> get_attachment( "audio" );
	
	if( current_attachment == "contact" ) return dbm -> check_person();
	
	if( current_attachment == "location" || current_attachment == "venue" )
		return dbm -> check_place();
	
	
	// Если тип вложения указан неверно
	logs -> CommitConveer( F, L, "Incorrect attachment type" );
	return Answerbox( config -> get_keyword ( "error_incorrect_attachment_type" ) );
}

Answerbox Answer_generator::get_media(const std::string &current_path, const size_t keynum) // Get attachment from media node in config
{
	// Формирование адреса списка вложений
	const std::string storage_address = "media/" + source_info->get_interface() + "/" + current_path;
	
	// Проверка, существует ли такой медиа-пакет
	if (!config->there_is_element(storage_address))
	{
		//error_out("Error - incorrect config " + storage_address);
		return Answerbox(config -> get_keyword("error_incorrect_config"));
	}
	
	// Если существует
	else
	{
		try
		{
			Answerbox current_answer;
			
			// Node для списка вложений, чтобы сократить код
			const YAML::Node current_node = config->subtree(storage_address);
			// If we have list of media packages
			if (current_node.IsSequence() && current_node.size() > 0)
			{
				const int r_index = keynum % current_node.size();
				
				if (current_node[r_index]["type"])       current_answer.type = current_node[r_index]["type"].as<std::string>();
				if (current_node[r_index]["text"])       current_answer.text = current_node[r_index]["text"].as<std::string>();
				if (current_node[r_index]["attachment"]) current_answer.attachment = current_node[r_index]["attachment"].as<std::string>();
			}
			// If we have one media package
			else if (current_node.IsMap())
			{
				if (current_node["type"])       current_answer.type = current_node["type"].as<std::string>();
				if (current_node["text"])       current_answer.text = current_node["text"].as<std::string>();
				if (current_node["attachment"]) current_answer.attachment = current_node["attachment"].as<std::string>();
			}
			
			if (
				(current_answer.text.empty() && current_answer.attachment.empty()) ||
				(!current_answer.type.empty() && current_answer.attachment.empty())
			) return Answerbox(config -> get_keyword("error_empty_config_media"));
			
			if ( current_answer.text.empty() && !current_answer.attachment.empty() )
				current_answer.text = current_answer.type.empty() ? current_answer.attachment : current_answer.type;
			
			return current_answer;
		}
		catch (YAML::Exception e)
		{
			std::string err = config->get_keyword("error_incorrect_config");
			error_out(err +  + " " + std::string(e.what()));
			return Answerbox(err);
		}
	}
}

std::string Answer_generator::get_text(const std::string &current_path, const size_t keynum) // Get text from config
{
	// Формирование адреса
	const std::string storage_address = "media_text/" + current_path;
	
	// Проверка, существует ли такой элемент
	if (!config->there_is_element(storage_address))
	{
		//error_out("Error - incorrect config, address not found: " + storage_address);
		return config->get_keyword("error_incorrect_config");
	}
	
	// Если существует
	else
	{
		try
		{
			const YAML::Node current_node = config->subtree(storage_address);
			// If we have list of strings
			if (current_node.IsSequence() && current_node.size() > 0 && current_node[keynum % current_node.size()].IsScalar())
			{
				return current_node[keynum % current_node.size()].as<std::string>();
			}
			// If we have one string
			else if (current_node.IsScalar()) return current_node.as<std::string>();
			
			// Config is incorrect
			else return config->get_keyword("error_incorrect_config");
		}
		catch (YAML::Exception e)
		{
			std::string err = config->get_keyword("error_incorrect_config");
			error_out(err +  + " " + std::string(e.what()));
			return err;
		}
	}
}

} // namespace Convspace
