/// Emil Hedemalm
/// 2015-01-25
/// Contains macros for SIMD SSE compliant 16-byte data alignment construction of memory, including constructor/destructors.

#ifndef SSE_MACROS_H
#define SSE_MACROS_H

#ifdef WINDOWS
#define USE_SSE
#endif

#ifdef USE_SSE
#include <xmmintrin.h>
#endif

// Declarations.
#ifdef USE_SSE
// Useful union. Makes it look like a regular vector. :)
typedef union {
	struct{
		float x,y,z,w;
	};
	__m128 data;
	float v[4];
} SSEVec;
#else 
// Same name, but without the SSE-specific types.
typedef union {
	struct{
		float x,y,z,w;
	};
	float v[4];
} SSEVec;
#endif

/// Allocates aligned memory. E.g. AppWindow * AppWindow = AllocAligned(AppWindow);
//#define AllocAligned(a) (a*) _aligned_malloc(1 * sizeof(a), 16)
#include "System/Memory.h"

/// Runs given constructor on the newly allocated memory.
#define Construct(a,b) new ((void*)a) b
/// Runs AllocAligned and then Construct using given constructor.
#define NewAlignedSpecificConstructor(a,b)	 Construct(AllocAligned(a), b);
#define NewAlignedDefaultConstructor(a)	Construct(AllocAligned(a), a);

// Runs destructor
#define DestructAligned(a)  a.~T()
#define DeallocateAligned(a) _aligned_free(a)
#define DeleteAligned(a) DeallocateAligned(DestructAligned(a))


/// Short-name macros.
#define NewAS(a,b) NewAlignedSpecificConstructor(a,b)
#define NewA(a) NewAlignedDefaultConstructor(a)
#define DeleteA(a) DeleteAligned(a)

#endif
