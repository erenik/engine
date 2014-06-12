// Emil Hedemalm
// 2013-03-29
// updated 2013-06-13 by Fredrik Larsson

#ifndef AEONIC_OS_H
#define AEONIC_OS_H

// Mainly just OS definitions to make it simple for access
#define PLATFORM_WINDOWS	1
#define PLATFORM_MAC		2
#define PLATFORM_UNIX		3
                                                // windows
#if defined(_WIN32)                                 // 32-bit
	#define WINDOWS WIN32
	#define PLATFORM PLATFORM_WINDOWS
#elif defined(_WIN64)                               // 64-bit
	#define WINDOWS WIN64
	#define PLATFORM PLATFORM_WINDOWS
#elif defined(__CYGWIN__) && !defined(_WIN32)       // Cygwin POSIX under Windows
	#define WINDOWS CYGWIN
	#define PLATFORM PLATFORM_WINDOWS
#endif

// Perform windows-specific includes straight-away!
#if defined WINDOWS
// #include "WindowsIncludes.h"
	// Set target windows platform to Windows Vista and later (0x0600)
//	#undef _WIN32_WINNT
//	#define _WIN32_WINNT 	_WIN32_WINNT_WIN7
//	#define _WIN32_WINNT _WIN32_WINNT_VISTA 
#endif


                                                // Linux
#if defined(__linux__)
	#define LINUX
	#define PLATFORM PLATFORM_UNIX
#endif

                                                // OSX
#if defined(__APPLE__) && defined(__MACH__)         // Apple OSX or iOS

    #define OSX 1

    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR == 1                // iOS Xcode simulator
        #define APPLE XCODE
    #elif TARGET_OS_IPHONE == 1                     // iOS on iPad, iPhone, etc
        #define APPLE IPAD
    #elif TARGET_OS_MAC == 1                        // OSX
        #define APPLE OSX
	#endif
	#define PLATFORM PLATFORM_MAC
#endif

#endif // AEONIC_OS_H


