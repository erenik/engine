/// Emil Hedemalm
/// 2015-01-25
/// Re-defines macros and even new and delete if activated? D:


#include "SSE.h"

#include <exception> // for std::bad_alloc
#include <new>
#include <cstdlib> // for malloc() and free()

// Visual C++ fix of operator new

void* operator new (size_t size)
{
	void *p=malloc(size); 
	if (p==0) // did malloc succeed?
	throw std::bad_alloc(); // ANSI/ISO compliant behavior
	return p;
}