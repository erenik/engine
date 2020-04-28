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
	typedef	int32_t	int32;
	typedef uint32_t uint32;
	typedef unsigned char uchar;
	typedef unsigned short ushort;
#endif // OSs


/// And some max/min values if not defined elsewhere.
#ifndef MIN_MAX_VALUES
#define MIN_MAX_VALUES
	#ifdef WINDOWS
		#ifndef UINT32_MAX
		#define UINT32_MAX 4294967295
		#endif
	#elif defined LINUX
		// available via stdint.h .. although in C++11 version onry
		#ifndef UINT32_MAX
		#define UINT32_MAX 4294967295
		#endif
	#endif
#endif

#endif