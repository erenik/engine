/// Emil Hedemalm
/// 2014-01-24
/// Tcp socket class inspired by Qt Socket classes

#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "Socket.h"

class TcpSocket : public Socket{
public:
	TcpSocket();
	/// Writes bytes from buffer into socket. Returns bytes written or -1 if the socket is closed.
	virtual int Write(const char * fromBuffer, int maxBytesToWrite);
	/// Reads bytes from the socket. If the socket has closed -1 will be returned.
	virtual int Read(char * intoBuffer, int maxBytesToRead);
	/// Closes the socket.
	virtual void Close();
	/// Attempts to connect to target host.
	virtual bool ConnectTo(String hostAddress, int port);
private:
};

#endif