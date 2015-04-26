/// Emil Hedemalm
/// 2015-02-23
/// Contains macros for declaration and implementation of threads, no matter the OS.

#ifndef OS_THREAD_H
#define OS_THREAD_H

#include "OS.h"
/*	All defines explained here.

	CREATE_THREAD(classAndFunctionName, threadHandle): creates and starts a thread using given function name (may be part of class or not). 
		The threadHandle will have the proper value after this. or may be checked with a macro (add later on!)
	THREAD_START(functionName): starts the thread with given name, adding code right up to the curcle braces start {}
	PROCESSOR_THREAD_DEC: declares a thread starting function in class of choice.
	PROCESSOR_THREAD_START(managerName): is replaces by the proper starting code for the thread, right before the curly brackets {}
	THREAD_HANDLE: declares a variable or pointer which is the handle which is used to store the thread which you want to start.
	RETURN_NULL(threadHandle): sets the thread handle to NULL, signifying to other threads that it has ended, and returns 0 if needed (depends on OS).
*/

/// Call if e.g. audio or graphics fail to set up. Be sure to Log the errors first! e.g. using LogFile
class String;
void QuitApplicationFatalError(const String & errorMessage);

#ifdef WINDOWS 
	// Windows
	#include <process.h>
	#define CREATE_AND_START_THREAD(classAndFunctionName, threadHandle)  	threadHandle = _beginthread(classAndFunctionName, NULL, NULL);
	#define THREAD_START(functionName) void functionName(void * vArgs)
	#define PROCESSOR_THREAD_DEC static void Processor(void * vArgs);
	#define PROCESSOR_THREAD_START(managerName) void managerName::Processor(void * vArgs)
	#define THREAD_HANDLE uintptr_t 
	#define RETURN_NULL(threadHandle) threadHandle = 0; 	return;
#elif defined LINUX | defined OSX
	// Linux/OS X
	#define CREATE_AND_START_THREAD(classAndFunctionName, threadHandle)     pthread_create(&threadHandle, NULL, classAndFunctionName, NULL);
	#define THREAD_START(functionName) void * functionName(void * vArgs)
	#define PROCESSOR_THREAD_DEC static void * Processor(void * vArgs);
	#define PROCESSOR_THREAD_START(managerName) void * managerName::Processor(void * vArgs)
	#define THREAD_HANDLE pthread_t
	#define RETURN_NULL(threadHandle) threadHandle = 0; return 0;
#endif

#endif // OS_THREAD_H