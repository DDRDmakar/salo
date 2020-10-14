
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

#include <string>
#include <vector>
#include <algorithm>
#include <yaml-cpp/yaml.h>

#include "stats.h"
#include "../headers/database.h"

namespace Stats
{

// Update statistics summary in generic database.
// If bot name is empty, update for all bots in this database.
// If database name is empty, update for all databases
//	Statistics database structure:
//
//	database_1:
//		bot_1:
//			interface_1:
//				parameter_1: 0
//				parameter_2: 0
//				...
//			interface_2:
//				parameter_2: 0
//				...
//			interface_n:
//				...
//		bot_2:
//		...
//	database_2:
//	...


// PARAMETERS:
//	Total messages count (message_count)
//	Total users count    (user_count)
//

// CONFIG

/*
all_databases: {
	{name: SALODATABASE, bots: [salo, kim, shava, doshik]},
	{name: MARYANA, bots: [maryana]},
	{name: GRIBDATABASE, bots: [grib]},
	{name: COMMERCIALBOTS, bots: []}
	# For commercial bots is allowed to get new bot names from database
}
all_interfaces: [networking_vkcom, networking_vkgroup, networking_telegram, SaloAPI, webfrontend]
unstable_interfaces: [webfrontend]
refresh_period: {
	user_count: '10 MINUTE',
	message_count: '10 MINUTE'
}
*/

inline bool is_unsigned_int(const std::string &current_line) { return !current_line.empty() && (current_line.find_first_not_of("0123456789") == std::string::npos); }

void update_stats(const std::string &dbname, const std::string &botname)
{
	std::cout << "Updating stats ... \n";
	
	try
	{
		std::vector<std::string> interfaces_all =
		{
			"networking_vkcom",
			"networking_vkgroup",
			"networking_telegram",
			"SaloAPI",
			"webfrontend"
		};
		
		std::vector<std::string> interfaces_unstable =
		{
			"webfrontend"
		};
		
		//std::string cfg = "all_databases: [\n	{name: SALODATABASE, bots: [salo, kim, shava, doshik]},\n	{name: MARYANA, bots: [maryana]},\n	{name: GRIBDATABASE, bots: [grib]},\n	{name: COMMERCIALBOTS, bots: []}\n]\nall_interfaces: [networking_vkcom, networking_vkgroup, networking_telegram, SaloAPI, webfrontend]\nunstable_interfaces: [webfrontend]\nrefresh_period: {\n	user_count: '10 MINUTE',\n	message_count: '10 MINUTE'\n}\n";
		
		YAML::Node config;
		if (true)
		{
			const std::vector<std::string> DBres = database->database_returnQuery("SELECT `value` FROM `GENERICDATABASE`.`generic_configs` WHERE `name` = 'stats';").RowStrings;
			const std::string config_plain_text = DBres.empty() ? "" : DBres[0];
			config = YAML::Load(config_plain_text);
		}
		
		// Iterate through database names
		for (YAML::const_iterator it_1 = config["all_databases"].begin(); it_1 != config["all_databases"].end(); ++it_1)
		{
			// If we don`t need this database, SKIP
			if (!dbname.empty() && dbname != it_1->operator[]("name").as<std::string>()) continue;
			
			// // CREATE LIST OF BOTS TO UPDATE STATS FOR IN THIS DATABASE
			
			// Create list of bot names in this database
			std::vector<std::string> botnames;
			if (botname.empty())
			{
				// Collect names to check in this database
				for (YAML::const_iterator it_2 = it_1->operator[]("bots").begin(); it_2 != it_1->operator[]("bots").end(); ++it_2)
				{
					botnames.push_back(it_2->as<std::string>());
				}
				// If bots are commercial, read bot names from commercial bots DB
				if ("COMMERCIALBOTS" == it_1->operator[]("name").as<std::string>())
				{
					const std::vector<std::string> DBres = database->database_returnQuery("SELECT `bot_uid` FROM `GENERICDATABASE`.`bots` GROUP BY `bot_uid`;").RowStrings;
					for (const auto &e : DBres) botnames.push_back(e);
				}
			}
			else botnames.push_back(botname);
			
			// UPDATE STATS FOR SELECTED BOTS
			
			const std::string current_db_name = it_1->operator[]("name").as<std::string>();
			// Iterate through botnames from this database
			for (const auto &e : botnames)
			{
				// Iterate through all_interfaces
				for (const auto &i : interfaces_all)
				{
					const bool interface_is_stable = std::find(interfaces_unstable.begin(), interfaces_unstable.end(), i) == interfaces_unstable.end();
					std::vector<std::string> DBres;
					int counter = 0;
					
					// Total user count
					DBres = database->database_returnQuery("SELECT COUNT(*) FROM `GENERICDATABASE`.`generic_configs` WHERE `name` = 'stats_activity_user_count." + current_db_name + "." + e + "." + i + "';").RowStrings;
					if (DBres.empty() || DBres[0] == "0") database->database_simpleQuery("INSERT INTO `GENERICDATABASE`.`generic_configs` (`name`, `value`) VALUES ('stats_activity_user_count." + current_db_name + "." + e + "." + i + "', '');");
					DBres = database->database_returnQuery("SELECT `name` FROM `GENERICDATABASE`.`generic_configs` WHERE `name` = 'stats_activity_user_count." + current_db_name + "." + e + "." + i + "' AND `value` < DATE_SUB(NOW(), INTERVAL " + config["refresh_period"]["user_count"].as<std::string>() + ");").RowStrings;
					if (!DBres.empty())
					{
						DBres = database->database_returnQuery("SELECT COUNT(*) FROM `" + current_db_name + "`.`" + std::string(interface_is_stable ? "user" : "web_user") + "` WHERE `id` LIKE '" + e + "_" + i + "_%'").RowStrings;
						if (!DBres.empty() && is_unsigned_int(DBres[0])) counter += std::stoi(DBres[0]);
						if (interface_is_stable)
						{
							DBres = database->database_returnQuery("SELECT COUNT(*) FROM `" + current_db_name + "`.`user_reserve` WHERE `id` LIKE '" + e + "_" + i + "_%'").RowStrings;
							if (!DBres.empty() && is_unsigned_int(DBres[0])) counter += std::stoi(DBres[0]);
						}
						DBres = database->database_returnQuery("SELECT COUNT(*) FROM `GENERICDATABASE`.`stats` WHERE `db` = '" + current_db_name + "' AND `bot` = '" + e + "' AND `interface` = '" + i + "' AND `parameter` = 'user_count';").RowStrings;
						if (DBres.empty() || DBres[0] == "0") database->database_simpleQuery("INSERT INTO `GENERICDATABASE`.`stats` (`db`, `bot`, `interface`, `parameter`, `value`) VALUES ('" + current_db_name + "', '" + e + "', '" + i + "', 'user_count', '0');");
						database->database_simpleQuery("UPDATE `GENERICDATABASE`.`stats` SET `value` = '" + std::to_string(counter) + "' WHERE `db` = '" + current_db_name + "' AND `bot` = '" + e + "' AND `interface` = '" + i + "' AND `parameter` = 'user_count';");
						
						// Update activity time
						database->database_simpleQuery("UPDATE `GENERICDATABASE`.`generic_configs` SET `value` = NOW() WHERE `name` = 'stats_activity_user_count." + current_db_name + "." + e + "." + i + "';");
					}
					
					// Total messages count
					DBres = database->database_returnQuery("SELECT COUNT(*) FROM `GENERICDATABASE`.`generic_configs` WHERE `name` = 'stats_activity_message_count." + current_db_name + "." + e + "." + i + "';").RowStrings;
					if (DBres.empty() || DBres[0] == "0") database->database_simpleQuery("INSERT INTO `GENERICDATABASE`.`generic_configs` (`name`, `value`) VALUES ('stats_activity_message_count." + current_db_name + "." + e + "." + i + "', '');");
					DBres = database->database_returnQuery("SELECT `name` FROM `GENERICDATABASE`.`generic_configs` WHERE `name` = 'stats_activity_message_count." + current_db_name + "." + e + "." + i + "' AND `value` < DATE_SUB(NOW(), INTERVAL " + config["refresh_period"]["message_count"].as<std::string>() + ");").RowStrings;
					if (!DBres.empty())
					{
						DBres = database->database_returnQuery("SELECT SUM(`messages_sent`) FROM `" + current_db_name + "`.`" + std::string(interface_is_stable ? "user" : "web_user") + "` WHERE `id` LIKE '" + e + "_" + i + "_%'").RowStrings;
						if (!DBres.empty() && is_unsigned_int(DBres[0])) counter += std::stoi(DBres[0]);
						if (interface_is_stable)
						{
							DBres = database->database_returnQuery("SELECT SUM(`messages_sent`) FROM `" + current_db_name + "`.`user_reserve` WHERE `id` LIKE '" + e + "_" + i + "_%'").RowStrings;
							if (!DBres.empty() && is_unsigned_int(DBres[0])) counter += std::stoi(DBres[0]);
						}
						DBres = database->database_returnQuery("SELECT COUNT(*) FROM `GENERICDATABASE`.`stats` WHERE `db` = '" + current_db_name + "' AND `bot` = '" + e + "' AND `interface` = '" + i + "' AND `parameter` = 'message_count';").RowStrings;
						if (DBres.empty() || DBres[0] == "0") database->database_simpleQuery("INSERT INTO `GENERICDATABASE`.`stats` (`db`, `bot`, `interface`, `parameter`, `value`) VALUES ('" + current_db_name + "', '" + e + "', '" + i + "', 'message_count', '0');");
						database->database_simpleQuery("UPDATE `GENERICDATABASE`.`stats` SET `value` = '" + std::to_string(counter) + "' WHERE `db` = '" + current_db_name + "' AND `bot` = '" + e + "' AND `interface` = '" + i + "' AND `parameter` = 'message_count';");
						
						// Update activity time
						database->database_simpleQuery("UPDATE `GENERICDATABASE`.`generic_configs` SET `value` = NOW() WHERE `name` = 'stats_activity_message_count." + current_db_name + "." + e + "." + i + "';");
					}
				} // for
			} // for
		} // for
	} // try
	catch (const YAML::Exception &e) { std::cout << "\033[1;33m Error reading stats config from database: " << e.what() << " \033[0m" << std::endl; }
	catch (const std::invalid_argument &e) { std::cout << "\033[1;33m Error - bad database query result while updating stats: " << e.what() << " \033[0m" << std::endl; }
	
	std::cout << "Done \n";
}

} // Namespace stats
