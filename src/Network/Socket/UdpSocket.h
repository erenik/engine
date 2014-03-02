/// Emil Hedemalm
/// 2014-01-24
/// Udp socket class inspired by Qt Socket classes

#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include "Socket.h"

/// Udp socket class inspired by Qt Socket classes. Currently lacks support for re-sending.
class UdpSocket : public Socket{
public:
	UdpSocket();
	/// Virtual destructor for proper deallocation.
	virtual ~UdpSocket();
	
	/// Here just to overload so they are not used.
	virtual int Write(const char * fromBuffer, int maxBytesToWrite);
	/// Here just to overload so they are not used.
	virtual int Read(char * intoBuffer, int maxBytesToRead);
	/// Here just to overload so they are not used.
	virtual bool ConnectTo(String hostAddress, int port);


	/// Writes bytes from buffer to target address and port. Returns bytes written or -1 if the socket is closed.
	virtual int WriteTo(String ip, String port, const char * fromBuffer, int maxBytesToWrite);
	/// Reads bytes from the socket. If the socket has closed -1 will be returned.
	virtual int ReadFrom(sockaddr * address, char * intoBuffer, int maxBytesToRead);
	/// Closes the socket.
	virtual void Close();
	/// Attempts to connect to target host.
	virtual bool Bind(int port);
private:

};

#endif


/*

	// If we have UDP, configure the host client
//	if(Network.connectionType == SOCK_DGRAM)
//	{
	//	clients[0].clientAddr.sin_family		= server->ai_family;
	//	clients[0].clientAddr.sin_port			= Network.port;
	//	clients[0].clientAddr.sin_addr.s_addr	= inet_addr(server->ai_addr->sa_data);
	//	clients[0].clientAddrSize				= sizeof(clients[0].clientAddr);
//	}

*/