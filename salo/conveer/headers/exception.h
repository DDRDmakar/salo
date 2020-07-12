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

#ifndef __SEQUENCE_CHECK_CONVEER_EXCEPTION__
#define __SEQUENCE_CHECK_CONVEER_EXCEPTION__

#include <iostream>
#include <iomanip>

namespace Convspace
{
	
	// This is the base class for all exceptions
	class Exception : public std::exception
	{
		
	protected:
		const std::string description;
		
	public:
		
		//! Constructor
		Exception(const std::string &desc = "") : description(desc) {}
		Exception(const Exception&) = default;
		
		//! The destructor is not allowed to throw exceptions
		// virtual ~Exception() throw() {}
		
		//! Show message
		const std::string what() { return "Conveer exception: " + description; }
	};
	
	// Config exception
	class Exception_config : public Exception
	{
	public:
		Exception_config(const std::string &desc = "") : Exception(desc) {}
		// virtual ~Config_exception() throw() {}
		const std::string what() { return "Conveer config exception: " + description; }
	};
	
	// Config exception
	class Exception_command : public Exception
	{
	public:
		Exception_command(const std::string &desc = "") : Exception(desc) {}
		// virtual ~Config_exception() throw() {}
		const std::string what() { return "Conveer command exception: " + description; }
	};
}

#endif
