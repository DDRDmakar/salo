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
