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

#include "headers/dynamic_operation.h"
#include "headers/community.h"
#include "headers/muxamed.h"
#include "headers/database_manager.h"

namespace Convspace
{

Dynamic_organizer::Dynamic_organizer
(
	Config *transit_config,
	Community_organizer *transit_source_info,
	Config_dyn *transit_dyn
)
{
	logs -> CommitConveer( F, L, "Object initialization - Dynamic_organizer" );
	config = transit_config;
	source_info = transit_source_info;
	dyn = transit_dyn;
	open_session();
}

void Dynamic_organizer::stop( void )
{
	remove_from_list( source_info -> get_unique_id() );
	source_info->userconfig->editable_subtree()["game"]["gamemode"] = 0; // Выключаю игровой режим в структуре юзера
}

void Dynamic_organizer::add_basic_data( const std::wstring &transit_phrase )
{
	phrase = transit_phrase;
}

void Dynamic_organizer::open_session( void )
{
	phrase.clear();
	clr_list();
}

void Dynamic_organizer::clr_list( void )
{
	id_list.clear();
}

bool Dynamic_organizer::is_on( void )
{
	return ! id_list.empty();
}

void Dynamic_organizer::add_to_list( const std::string &id )
{
	if( is_participant( id ) ) return;
	else
	{
		// Если превышено число участников, то они "выталкиваются" из списка с другого конца.
		id_list.push_back( id );
		if( id_list.size() >= config -> get_coefficient( "max_count_dynamic_operation_user" ) ) id_list.erase( id_list.begin() );
	}
}
void Dynamic_organizer::add_to_list( void )
{
	add_to_list( source_info -> get_unique_id() );
}

void Dynamic_organizer::remove_from_list( const std::string &id )
{
	for( int i = 0; i < id_list.size(); ++i ) if( id_list[i] == id ) { id_list.erase( id_list.begin() + i ); }
}
void Dynamic_organizer::remove_from_list( void )
{
	remove_from_list( source_info -> get_unique_id() );
}

bool Dynamic_organizer::is_participant( const std::string &id )
{
	for( auto i : id_list )
	{
		if( i == id ) return true;
	}
	return false;
}
bool Dynamic_organizer::is_participant( void )
{
	return is_participant( source_info -> get_unique_id() );
}

} // namespace Convspace
