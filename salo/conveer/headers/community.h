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


#ifndef __SEQUENCE_CHECK_COMMUNITY__
#define __SEQUENCE_CHECK_COMMUNITY__

#include <string>
#include <vector>

#include "muxamed.h"
#include "config.h"
#include "config_user.h"

/*
 * Класс для получения всевозможной инфы
 * о сообщении, его источнике и авторе
*/

using namespace std;

namespace Convspace
{

class Database_manager;

class Community_organizer
{
private:
	
	Config *config;
	Database_manager *dbm;
	
	Person user;
	CONFIGURATION bot_config;
	
	int user_status;
	bool stable;
	// bool is_new; // Первое ли это сообщение
	std::string bot_name;
	std::string unique_id; // Уникальный идентификатор ( имя бота + интерфейс + id )
	
public:
	
	Config_user *userconfig;
	
	Community_organizer( Config *transit_config = NULL, Database_manager *transit_dbm = NULL );
	void add_basic_data // Запись базовых данных ( начало работы )
	(
		const Person &transit_user,
		const CONFIGURATION &transit_config
	);
	
	Person get_user( void );
	CONFIGURATION get_configuration( void );
	
	int get_user_status( const std::string &id = "" ); // Статус пользователя ( 0/1/2 ... )
	bool user_is_stable( void ); // Постоянны ли данные пользователя?
	std::string get_message_form( void );
	std::string get_user_id( void );
	std::string get_unique_id( void );
	std::string get_user_ip( void );
	std::string get_user_first( void );
	std::string get_user_last( void );
	std::string get_interface( void ); // Источник сообщения
	std::string get_userhash ( void );
	std::string get_attachment ( void );
	std::string get_bot_name( void );
	std::string get_bot_name_and_interface( void );
	std::string get_bot_interface_and_id( void );
	std::string get_saloapi_login( void );
	void* get_bot_pointer( void );
	std::string get_interface_unique_identificator( void );
	bool user_is_new( void ); // Является ли сообщение первым
	bool is_conf(void); // Пришло ли сообщение из группового диалога
	
	size_t gethash(std::string pat);
	/*
	 * Генерирует hash (std::size_t) по заданному шаблону.
	 * 
	 * Шаблоны задаются в скобках {@ @} для обработки через инструмент Pattern_replacer
	 * Доступные шаблоны:
	 * 
	 * id
	 * uid
	 * name
	 * year
	 * mon
	 * yday
	 * mday
	 * wday
	 * hour
	 * min
	 * sec
	 * 
	 * А также любые их комбинации
	 * 
	 * Все остальные шаблоны будут проигнорированы
	 */
};

} // namespace Convspace

#endif
