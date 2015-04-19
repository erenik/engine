/// Emil Hedemalm
/// 2014-07-01
/// For checking System/OS memory status

#ifndef MEMORY_H
#define MEMORY_H

#include "DataTypes.h"

/// Returns amount of available memory in mega-bytes.
int64 AvailableMemoryMB();

/// Returns memory load, expressed as a percentage integer.
int MemoryLoad();


#ifdef WINDOWS
	#define Align(bytes)	__declspec(align(bytes)) 
#elif defined LINUX
	#define Align(bytes) //
#endif

#ifndef NULL
#define NULL 0
#endif

/// For aligned memory allocation, OS-specific
/// Allocates numBytes, using alignment of specified bytes. Returns pointer to allocated memory.
/// If ok is specified as non-null, the success of the operation will be stored there.
void * AllocateAligned(int numBytes, int alignment, bool * ok = NULL);
/// Allocates memory for a given object.
#define AllocAligned(a) (a*) AllocateAligned(1 * sizeof(a), 16)

#endif
