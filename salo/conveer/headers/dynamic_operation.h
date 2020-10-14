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


// Класс для поддержки различных динамических режимов


#ifndef __SEQUENCE_CHECK_DYNAMIC__
#define __SEQUENCE_CHECK_DYNAMIC__

#include <vector>
#include <string>
#include "config.h"
#include "config_dyn.h"
//#include "community.h"

using namespace std;

namespace Convspace
{

class Community_organizer;

class Dynamic_organizer
{
private:
	
	std::vector <std::string> id_list; // Список участников
	
protected:
	
	Config *config;
	Community_organizer *source_info;
	Config_dyn *dyn;
	
	std::wstring phrase; // Текущее сообщение
	
public:
	
	// Конструктор
	Dynamic_organizer
	(
		Config *transit_config = NULL,
		Community_organizer *transit_source_info = NULL,
		Config_dyn *transit_dyn = NULL
	);
	
	void add_basic_data( const std::wstring &transit_phrase ); // Запись базовых данных ( начало работы )
	void open_session( void ); // Очистка полей
	void clr_list( void ); // Удалить все записи
	bool is_on( void ); // Активность режима
	void add_to_list( const std::string &id );
	void add_to_list( void ); // Добавить пользователя в список
	void remove_from_list( const std::string &id );
	void remove_from_list( void ); // Убрать из списка
	bool is_participant( const std::string &id );
	bool is_participant( void ); // Можно ли включать режим этому пользователю ( является ли он участником )
	void stop( void ); // Завершение процесса
	int get_participant_count( void ); // Узнать количество участников
	
};

} // namespace Convspace

#endif
