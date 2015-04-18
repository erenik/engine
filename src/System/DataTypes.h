/// Emil Hedemalm
/// 2014-05-01
/// Getting annoying typing "long long" as it is both long and ambiguous.

#ifndef SYSTEM_DATA_TYPES_H
#define SYSTEM_DATA_TYPES_H

#include "OS/OS.h"

// Defines new types for the compiler. Mostly just referring to existing types.
#ifdef WINDOWS
	typedef	__int64	int64;
	typedef unsigned __int64 uint64;
	typedef	int	int32;
	typedef unsigned int uint32;
	typedef unsigned char uchar;
	typedef unsigned short ushort;
#elif defined LINUX
	#include <inttypes.h>
	typedef	int64_t	int64;
	typedef uint64_t uint64;
	typedef	int	int32;
	typedef unsigned int uint32;
	typedef unsigned char uchar;
	typedef unsigned short ushort;
#endif // OSs

#endif