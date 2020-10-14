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
#include <string>
//#include "../headers/conveer_information.h"
#include "../headers/message.h"


/*

Конструктор

*/

flood_protection::flood_protection()
{
	flood_lock = 0;
	current_streak = 0;
	times_enabled = 0;
	cooldown_enabled = false;

	load = new bool[5];

	for(int i = 0; i < 5; i++)
	{
		load[i] = false;
	}

	empty_ticks = 0;
}


/*

Деструктор

*/

flood_protection::~flood_protection()
{
	delete[] load;
}

/*

Анализ действия при обновлении

*/

void flood_protection::Tick(bool send)
{
	
	load[4] = load[3];
	load[3] = load[2];
	load[2] = load[1];
	load[1] = load[0];
	load[0] = send;

	int maxstreak = 0;
	int streak = 0;

	for(int i = 0; i < 5; i++)
	{
		if(load[i])
		{
			streak++;
		}
		else
		{
			if(streak > maxstreak)
			{
				maxstreak = streak;
			}

			streak = 0;
		}
	}

	if(streak > maxstreak) { maxstreak = streak; }
	current_streak = maxstreak;

	if(flood_lock > 0) 
	{
		return;
	}

	if(current_streak > 3)
	{
		flood_lock = 15;
		logs->CommitGeneric(F, L, "SALO experiences high load (streak 4 or more)");
		logs->CommitGeneric(F, L, "FLOOD PROTECTION MODE ENABLED");
		//AlertDevelopers("FLOOD PROTECTION ENABLED\nEnabled for 15 nearest ticks, reason: too big streak");
		times_enabled++;
		return;
	}

	if (!send)
	{
		empty_ticks++;

		if(empty_ticks > 120 && flood_lock > 0)
		{
			flood_lock = 0;
			AlertDevelopers("FLOOD PROTECTION DISABLED\nReason: 120 ticks of inactivity");
		}

		return;
	}
	else
	{
		empty_ticks = 0;
	}

	int lastmin = 100; //last_time_messages(1, "MINUTE"); // TODO

	if(lastmin > 20)
	{
		flood_lock = 15;
		logs->CommitGeneric(F, L, "SALO experiences high load (20 messages limit)");
		logs->CommitGeneric(F, L, "FLOOD PROTECTION MODE ENABLED");
		//AlertDevelopers("FLOOD PROTECTION ENABLED\nEnabled for 15 nearest ticks, reason: too many messages per minute");
		times_enabled++;
		return;
	}



	//printf("Flood table status: %d%d%d%d%d Streak = %d Protected mode = %d\n", 
		//load[0], load[1], load[2], load[3], load[4], current_streak, flood_lock);
}


/*

Возвращает true, еслизащита от флуда включена

*/

bool flood_protection::floodprotection_status()
{
	bool status;

	if(flood_lock > 0)
	{
		status = true;
		flood_lock--;
	}
	else
	{
		status = false;
	}

	return status;
}


/*

Возвращает текстовую строку со статусом защиты от флуда

*/

std::string flood_protection::load_status()
{

	std::string t = "\nVK:\n";
	t += "Load: ";

	int lm = 10; //last_time_messages(1, "MINUTE"); // TODO

	if(current_streak < 3)
	{
		if(lm < 5) { t += "not loaded\n"; }
		if(lm > 4 && lm < 10) { t += "low\n"; }
		if(lm > 9 && lm < 20) { t += "normal\n"; }
		if(lm > 19) { t += "under load\n"; }
	} 
	else
	{
		switch(current_streak)
		{
			case 3: t += "under load\n"; break;
			case 4: t += "flooding\n"; break;
			case 5: t += "overloaded\n"; break;
		}
	}

	t += "Current streak: " + std::to_string(current_streak) + "\n";
	t += "Messages (last minute): " + std::to_string(lm) + "\n";

	if(flood_lock > 0)
	{
		t += "Protected mode: ENABLED (" + std::to_string(flood_lock) + " ticks)\n";
	}
	else
	{
		t += "Protected mode: disabled\n";
	}	

	t += "In this session: enabled " + std::to_string(times_enabled) + " times\n";
	t += "Cooldown Enabled: " + std::to_string(cooldown_enabled) + "\n";
	
	return t;
}
