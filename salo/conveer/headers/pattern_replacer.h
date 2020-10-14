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


#ifndef __SEQUENCE_CHECK_PATTERN_REPLACER__
#define __SEQUENCE_CHECK_PATTERN_REPLACER__

#include <string>

namespace Convspace
{
	
	class Pattern_replacer
	{
		
	private:
		
		std::string pattern_line;
		
	public:
		
		// Constructor
		Pattern_replacer(const std::string &source_line);
		
		// Check, if such pattern exists in string
		bool has(const std::string &target);
		
		// Replace template {@template@} with another string
		void replace(const std::string &pattern, const std::string &target);
		
		// Unescape symbols in pattern line and return it
		std::string extract(void);
	};
	
	// Get, replace and extract
	std::string pattern_replace_single(std::string source_line, const std::string &pattern, const std::string &target);
	
} // namespace Convspace
#endif
