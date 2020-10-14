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

#include "headers/vkcom.h"
#include "../headers/misc.h"
#include "../headers/database.h"
#include "../headers/bots.h"


/*

Функция автомтически принимает заявки в друзья

*/

void networking_vkcom::autoAcceptFriends()
{
	std::string apiurl = "https://api.vk.com/method/execute.autoAccept?v=5.50&access_token=" + VKONTAKTE_ACCESS_TOKEN;
	NetworkResult result = pNetwork->network_HTTPS_GET(apiurl.c_str());

	//std::cout << result.DATA << std::endl;

	if(result.DATA == "{\"response\":\"nofriends\"}")
	{
		return;
	}
	else
	{
		if(result.DATA == "{\"response\":\"error\"}")
		{
			std::cout << "Error while accepting friends" << std::endl;
			logs->CommitGeneric(F, L, "Error while friendship request accepting");
		}
		else
		{
			char* unbanid = miscFindAndCopy(result.DATA.c_str(), "\"toBeUnblocked\":", "}");

			if(unbanid != NULL)
			{
					bot* Bot = static_cast<bot*>(thisBot);
					std::string botid = Bot->getName();
					std::string dname = "`"+Bot->getConveerPtr()->get_database_name()+"`.";

				database -> database_simpleQuery( "INSERT INTO "+dname+"`ban_list` ( `id`, `time` ) VALUES ( '"+botid+"_networking_vkcom_" 
					+ database->FilterString(std::string(unbanid)) + "', NOW() + INTERVAL 1 HOUR );" );



				logs->CommitGeneric(F, L, "User ban record added to database due to inactive-friends-kick policy.");
				//std::cout << "User ban record added to database due to inactive-friends-kick policy." << std::endl;

				//AlertDevelopers("inactive-friends-kick-policy: banned user: " + botid+"_networking_vkcom_" 
				//	+ database->FilterString(std::string(unbanid)) + "\n\nVK says: \n\n" + result.DATA);

				delete[] unbanid;
			}
			else
			{
				//AlertDevelopers("inactive-friends-kick-policy: fatal: BAN ID IS NULL!!!\n\nVK says: \n\n" + result.DATA);
			}

			//std::cout << "Friendship request accepted" << std::endl;
			//logs->CommitGeneric(F, L, "Friendship request accepted");
		}
	}

	return;
}
