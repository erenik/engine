/// Emil Hedemalm
/// 2013-10-20
/// #ifndef'd includes for all networking

#ifndef NETWORK_INCLUDES_H
#define NETWORK_INCLUDES_H

/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include "OS/OS.h"
#if PLATFORM == PLATFORM_WINDOWS
	// Only for Windows
	#include <WS2tcpip.h>
	#include <WinSock2.h>	// For sockets in Windows
	#include <Windows.h>	// Windows stuff
	#include <process.h>	// For multi threading
	#pragma comment(lib, "Ws2_32.lib")
	#ifdef _MSC_VER
		#define sockerrno WSAGetLastError()
	#else
		#include <ws2tcpip.h>	// More windows socket
		#define sockerrno errno
	#endif
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
	// Only for MAC and Unix

// Windows to linux compability defines
#define GetLastError() (1)
#define WSAGetLastError() GetLastError()
#define closesocket(b)  close(b)

    /// To set non-blocking for sockets. http://stackoverflow.com/questions/1543466/how-do-i-change-a-tcp-socket-to-be-non-blocking
    #import <fcntl.h>
    /// Used for big byte transfer operations like memcpy
    #include <cstring>

    /// for poll instead of select: http://pic.dhe.ibm.com/infocenter/iseries/v6r1m0/index.jsp?topic=/rzab6/poll.htm
    #include <poll.h>

	#include <sys/time.h>
	#include <unistd.h>
    #include <sys/select.h>
    #include <sys/ioctl.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
    #include <arpa/inet.h>      // sockaddr_in
    #include <netinet/tcp.h>    // TCP_NODELAY
	#if PLATFORM == PLATFORM_MAC
		#define SOL_IP IPPROTO_IP
	#endif
	#ifndef SO_DONTLINGER
        #define SO_DONTLINGER   (~SO_LINGER)  /* Older SunOS compat. hack */
	#endif
	#ifndef INVALID_SOCKET
		#define INVALID_SOCKET -1
    #endif
    #ifndef SOCKET_ERROR
        #define SOCKET_ERROR -1
	#endif
	#define sockerrno errno
    #define SOCKET int
#endif // MAC/Linux

#endif // NETWORK_INCLUDES_H
