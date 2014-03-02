
#ifndef INITIALIZER_H
#define INITIALIZER_H

// Defines a thread class that sets up the environment and all managers
#ifdef WINDOWS
void Initialize(void * vArgs);
void Deallocate(void * vArgs);
#elif defined LINUX | defined OSX
void * Initialize(void * vArgs);
void * Deallocate(void * vArgs);
#endif

#endif // INITIALIZER_H
