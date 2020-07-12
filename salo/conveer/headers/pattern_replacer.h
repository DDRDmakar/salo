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
