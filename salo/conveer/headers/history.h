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


/*
 * Класс для работы с историей сообщений
 * и списком пользователей
*/



#ifndef __SEQUENCE_CHECK_HISTORY__
#define __SEQUENCE_CHECK_HISTORY__

#include <string>

#include "config.h"
#include "config_dyn.h"
#include "community.h"
#include "message_storage.h"
#include "database_manager.h"

using namespace std;

namespace Convspace
{

class History_organizer
{
	
private:
	
	Config *config;
	Community_organizer *source_info;
	Message_storage *message;
	Database_manager *dbm;
	Config_dyn *dyn;
	
	std::string input; // Сообщение пользователя ( без форматирования )
	std::string output; // Ответ бота
	std::string theme; // Новая тема, если есть
	std::string unsolved; // Если нераспознанное, то здесь сообщение в очищенном виде
	std::string message_form; // Тип сообщения
	
	void write_down_max_messages(void);
	
public:
	
	History_organizer
	(
		Config *transit_config = NULL,
		Community_organizer *transit_source_info = NULL,
		Message_storage *transit_message = NULL,
		Database_manager *transit_dbm = NULL,
		Config_dyn *transit_dyn = NULL
	);
	
	void open_session( void ); // Очистка полей
	void close_session( void ); // Запись всех накопленных данных в БД
	
	// Сохранение в оперативную память
	void add_phrase( const std::string& transit_input ); // Сохранение входящего сообщения
	void add_answer( const std::string &transit_output ); // Сохранение результата ( ответа )
	void add_theme( const std::string &transit_theme ); // Сохранение темы разговора
	void add_unsolved( const std::string &transit_unsolved ); // Сохранение нераспознанного сообщения
	void add_message_form( const std::string &transit_message_form ); // Сохранение типа сообщения
	void add_new_user( void ); // Добавление пользователя в базу ( если необходимо )
	
	std::string get_theme( void ); // Вынуть ответ по текущей теме из базы
	bool repeat_control( const std::string &current_line, const std::string &column ); // Проверка на повторения в данной колонке
	
};

} // namespace Convspace

#endif
