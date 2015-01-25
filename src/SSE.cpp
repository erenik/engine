/// Emil Hedemalm
/// 2015-01-25
/// Re-defines macros and even new and delete if activated? D:


#include "SSE.h"

#include <cstdlib> // for malloc() and free()
#include <exception> // for std::bad_alloc

// Visual C++ fix of operator new

void* operator new (size_t size)
{
	void *p = _aligned_malloc(size, 16); 
	if (p==0) // did malloc succeed?
		throw std::bad_alloc(); // ANSI/ISO compliant behavior
	return p;
}

void operator delete(void * p) throw()
{
	_aligned_free(p);
}


void *operator new[](std::size_t size) throw(std::bad_alloc)
{
	void *p = _aligned_malloc(size, 16); 
	if (p==0) // did malloc succeed?
		throw std::bad_alloc(); // ANSI/ISO compliant behavior
	return p;
}
void operator delete[](void *p) throw()
{
	_aligned_free(p);
}

#include <new>
