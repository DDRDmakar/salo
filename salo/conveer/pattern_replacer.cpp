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

#include "headers/pattern_replacer.h"
#include "headers/parse_tools.h"

namespace Convspace
{
	Pattern_replacer::Pattern_replacer(const std::string &source_line)
	{
		pattern_line = source_line;
	}
	
	bool Pattern_replacer::has(const std::string &target)
	{
		// @} is a forbidden combination in pattern
		if (appears_in_this_string(target, "@}")) return false;
		return appears_in_this_string(pattern_line, "{@" + target + "@}");
	}
	
	void Pattern_replacer::replace(const std::string &pattern, const std::string &target)
	{
		if (appears_in_this_string(pattern, "@}")) return;
		replace_in_text(pattern_line, "{@" + pattern + "@}", target);
	}
	
	std::string Pattern_replacer::extract(void)
	{
		return from_safestring(pattern_line);
	}
	
	std::string pattern_replace_single(std::string source_line, const std::string &pattern, const std::string &target)
	{
		replace_in_text(source_line, "{@" + pattern + "@}", target);
		return from_safestring(source_line);
	}
}
