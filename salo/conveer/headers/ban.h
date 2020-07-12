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
