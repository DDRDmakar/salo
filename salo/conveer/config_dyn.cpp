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

#include <yaml-cpp/yaml.h>

#include "headers/config_dyn.h"
#include "headers/parse_tools.h"

namespace Convspace
{

Config_dyn::Config_dyn(Config *transit_config) : Config_yaml()
{
	if (transit_config == NULL) return;
	
	config = transit_config;
	path = "resources/config/" + config->get_dir_name() + "dynamic.yaml";
	file_exists(path) ? put_data(wide_to_thin_line(read_file(path))) : put_data();
}

void Config_dyn::read_from_file(void)
{
	const std::string plain_node_text = file_exists(path) ? wide_to_thin_line(read_file(path)) : "";
	put_data(plain_node_text);
}

void Config_dyn::write_to_file(void)
{
	const std::wstring plain_node_text = thin_to_wide_line(show());
	write_file(path, plain_node_text);
}

void Config_dyn::synchronize(void)
{
	read_from_file();
	write_to_file();
}

// Public open node editing
void Config_dyn::dyn_insert_into_set(const std::string &path, const std::string &value) { insert_into_set(path, value); }
void Config_dyn::dyn_set_value(const std::string &path, const std::string &value) { set_value(path, value); }
void Config_dyn::dyn_clear_node(const std::string &path) { clear_node(path); }
void Config_dyn::dyn_seq_delete_index(const std::string &path, const int index) { seq_delete_index(path, index); }
void Config_dyn::dyn_seq_delete_value(const std::string &path, const std::string &value) { seq_delete_value(path, value); }
void Config_dyn::dyn_create_node(const std::string &path) { create_node(path); }
void Config_dyn::dyn_add_to_string_num(const std::string &path, const int v) { add_to_string_num(path, v); }

} // namespace Convspace
