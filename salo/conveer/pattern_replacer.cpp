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
