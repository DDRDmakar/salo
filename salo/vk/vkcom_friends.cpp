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
