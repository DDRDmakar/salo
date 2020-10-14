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

#include "../vk/headers/vkcom_blacklist.h"
#include "headers/ban.h"
#include "headers/parse_tools.h"

namespace Convspace
{

Ban_organizer::Ban_organizer
(
	Config *transit_config,
	Community_organizer *transit_source_info,
	Database_manager *transit_dbm
)
{
	logs -> CommitConveer( F, L, "Object initialization - Ban_organizer" );
	
	config = transit_config;
	source_info = transit_source_info;
	dbm = transit_dbm;
}

void Ban_organizer::ban( const unsigned int hours, std::string id, const std::string &reason ) // Добавление в ЧС
{
	logs -> CommitConveer( F, L, "Ban - Banning user" );
	if( id.empty() ) id = source_info -> get_unique_id();
	dbm -> DB_simple_query( "INSERT INTO `{@database_name@}`.`ban_list` ( `id`, `ip`, `reason`, `time` ) VALUES ( '" + screen_sql( id.empty() ? source_info -> get_unique_id() : id ) + "', '" + screen_sql( source_info -> get_user_ip() ) + "', '" + screen_sql( reason ) + "' , NOW() + INTERVAL " + std::to_string( hours ) + " HOUR );" );
	
	// Добавление в чс вк
	if(
		(int)id.find( "networking_vkcom" ) != std::string::npos &&
		begins_with(id, source_info->get_bot_name())
	)
	{
		int id_header_length = (int)id.find( "networking_vkcom" ) + 17; // source_info -> get_bot_name_and_interface().length() + 1;
		id.erase( 0, id_header_length );
		vkban( id, source_info -> get_bot_pointer(), source_info -> get_interface_unique_identificator() );
	}
}

void Ban_organizer::unban(std::string id) // Убрать пользователя из ЧС
{
	logs -> CommitConveer( F, L, "Ban - Unbanning user" );
	dbm -> DB_simple_query( "DELETE FROM `{@database_name@}`.`ban_list` WHERE `id` = '" + screen_sql( id.empty() ? source_info -> get_unique_id() : id ) + "' LIMIT 1;" );
	
	if (
		appears_in_this_string(id, "networking_vkcom") &&
		begins_with(id, source_info->get_bot_name_and_interface())
	)
	{
		int id_header_length = (int)id.find("networking_vkcom") + 17;
		id.erase(0, id_header_length);
		bool unbanned = vkunban( id, source_info -> get_bot_pointer(), source_info -> get_interface_unique_identificator());
		// Unban status checking is deprecated
	}
}

void Ban_organizer::remove_old(void) // Исключение из ЧС
{
	logs->CommitConveer(F, L, "Ban - Cleaning old ban_list");
	
	std::string id = dbm->return_query_one("SELECT `id` FROM `{@database_name@}`.`ban_list` WHERE `time` < NOW() AND `id` LIKE '" + source_info->get_bot_name() + "%' LIMIT 1;");
	
	if(!id.empty()) unban(id);
}

bool Ban_organizer::is_banned( std::string id )
{
	logs -> CommitConveer( F, L, "Ban - Checking if user is in ban-list" );
	if( id.empty() ) id = source_info -> get_unique_id();
	
	return dbm -> return_query_one( "SELECT `id` FROM `{@database_name@}`.`ban_list` WHERE " + ( source_info -> user_is_stable() ? std::string( "`ip` = '' AND `id` = '" + screen_sql( id ) + "' ;" ) : std::string( "`ip` = '" + screen_sql( source_info -> get_user_ip() ) + "' ;" ) ) ).length();
}

bool Ban_organizer::detect_flood( const std::string &current_line, const unsigned int n_repeat ) // Задетектить флуд
{
	logs -> CommitConveer( F, L, "Ban - Detecting flood" );
	if( !current_line.empty() && all_symbols_too_wide( current_line ) ) return false;
	return dbm -> return_query_count( "SELECT COUNT(*) FROM `{@database_name@}`.`history` WHERE `id` = '" + screen_sql( source_info -> get_unique_id() ) + "' AND `phrase` LIKE '" + screen_sql( current_line ) + "' AND `time` > DATE_SUB( NOW(), INTERVAL " + wide_to_thin_line( config -> get_keyword ( L"flood_check_interval" ) ) + " ) AND NOT( `message_form` = 'command' OR `message_form` LIKE 'attachment%' ); " ) + 1 >= n_repeat;
}

} // namespace Convspace
