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

#ifndef __SEQUENCE_CHECK_ANSWERBOX__
#define __SEQUENCE_CHECK_ANSWERBOX__

#include <string>

namespace Convspace
{

class Answerbox
{
	
public:
	
	std::string text;
	std::string attachment;
	std::string type;
	std::string name;
	bool is_active;
	
	Answerbox
	(
		const std::string &transit_text = "",
		const std::string &transit_attachment = "",
		const std::string &transit_type = "",
		const std::string &transit_name = ""
	)
	{
		name = transit_name;
		text = transit_text;
		attachment = transit_attachment;
		type = transit_type; // = transit_type.empty() ? "null" : transit_type;
		is_active = true;
	}
};

} // namespace Convspace

#endif
