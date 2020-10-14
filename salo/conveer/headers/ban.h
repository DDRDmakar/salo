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

// Класс, работающий с бан-листом


#ifndef __SEQUENCE_CHECK_BAN__
#define __SEQUENCE_CHECK_BAN__

#include <string>

#include "muxamed.h"
#include "community.h"
#include "config.h"
#include "database_manager.h"

namespace Convspace
{

class Ban_organizer
{
	
	Config *config;
	Community_organizer *source_info;
	Database_manager *dbm;
	
public:
	
	Ban_organizer
	(
		Config *transit_config = NULL,
		Community_organizer *transit_source_info = NULL,
		Database_manager *transit_dbm = NULL
	);
	
	void ban( const unsigned int hours, std::string id = "", const std::string &reason = "" ); // Забанить пользователя
	void unban( std::string id = "" ); // Разбанить пользователя
	void remove_old( void ); // разбанить тех, чьё время вышло
	bool is_banned( std::string id = "" ); // Проверить, состоит ли пользователь в бан-листе
	bool detect_flood( const std::string &current_line, const unsigned int n_repeat ); // Задетектить флуд
	// TODO (пока детектит только флуд)
};

} // namespace Convspace

#endif
