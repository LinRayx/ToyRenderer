#ifndef VEL_COMMAND_H
#define VEL_COMMAND_H
#include "Utils/VELPrerequisites.h"

#define VEL_NORETURN __declspec(noreturn)
namespace VEL
{
	const String BLANKSTRING;

	/// Name / value parameter pair (first = name, second = value)
	typedef std::map<String, String> NameValuePairList;
	
	typedef uint32_t uint32;
	typedef uint8_t uint8;
}

#endif // !VEL_COMMAND_H
