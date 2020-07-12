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
