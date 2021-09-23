#ifndef __StdHeaders_H__
#define __StdHeaders_H__

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cstdarg>
#include <cmath>

// STL containers
#include <vector>
#include <map>
#include <string>
#include <set>
#include <list>
#include <unordered_map>

// STL algorithms & functions
#include <algorithm>

// C++ Stream stuff
#include <ostream>
#include <iosfwd>

#include <atomic>

#if VEL_PLATFORM == VEL_PLATFORM_WIN32 || VEL_PLATFORM == VEL_PLATFORM_WINRT
#  undef min
#  undef max
#endif

#endif
