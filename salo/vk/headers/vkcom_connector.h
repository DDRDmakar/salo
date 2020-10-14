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

#ifndef VKCONNECTOR
#define VKCONNECTOR


struct vkcomBasicEvent
{
	std::string eventType;
	std::string userId;
	bool isEmpty = true;
};


struct vkcomBasicMessage
{
	bool have_message;
	unsigned int id;
	std::string user_id;
	std::string text;
	bool is_conf;
	std::string conf_id;
	bool have_attachment;
	std::string attachment_type;
	std::string attachment_link;
	std::string attachment_token;
	std::string attachment_jpeg;

	bool loginError = false;
	
	bool haveEvent = false;
	vkcomBasicEvent event;
};


//абстрактный класс для fetcher-legacy и callbackapi-коннектора
class Connector
{
public:
	virtual vkcomBasicMessage go() = 0;

	std::string connectorType;

	int mess_lookups;
	int cach_lookups;
};

#endif