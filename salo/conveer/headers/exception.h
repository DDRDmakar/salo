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
