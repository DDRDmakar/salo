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


// Класс для режима обучения


#ifndef __SEQUENCE_CHECK_TEACH__
#define __SEQUENCE_CHECK_TEACH__

#include <vector>
#include <string>

#include "community.h"
#include "dynamic_operation.h"
#include "database_manager.h"

using namespace std;


namespace Convspace
{

class Teach_organizer : public Dynamic_organizer
{
	
private:
	
	Database_manager *dbm;
	
	//                            userID, problemID
	std::vector <std::tuple <std::string, std::string>> user_info_storage;
	
	void clr_unsolved( void ); // Очистка базы нераспознанных сообщений
	void stop_teachmode( void ); // Завершение обучения
	
public:
	
	// Конструктор
	Teach_organizer
	(
		Config *transit_config = NULL,
		Community_organizer *transit_source_info = NULL,
		Database_manager *transit_dbm = NULL,
		Config_dyn *transit_dyn = NULL
	);
	
	void approve_all( void ); // Одобрить все записи с режима обучения
	void approve_serial( void ); // Одобрять по одной
	void delete_unsolved( void ); // Удалить все записи
	std::string teach( const std::wstring &transit_phrase = L"" ); // Главная функция обучения - кидает пользователю unsolved
	std::string start( void ); // Начало обучения - получение инструкций и стартовой фразы
	int get_current_user_index( std::string id = "" );
	void remove_from_list_2( std::string id = "" );
	void remove_old( void );
	
};

} // namespace Convspace

#endif
