/// Emil Hedemalm
/// 2014-01-24
/// General socket class inspired by Qt Socket classes

#ifndef SOCKET_H
#define SOCKET_H

#include "Network/NetworkIncludes.h"
#include <String/AEString.h>

class Peer;

namespace SocketType {
enum socketTypes{
	NULL_TYPE,
	TCP,
	UDP,
};};

/// Closes target socket.
void CloseSocket(SOCKET socket);
/// Returns state of a socket, if it is ready to be read or not!
bool ReadyToRead(SOCKET sock);
/// Returns state of a socket, if we can write to it.
bool ReadyToWrite(SOCKET sock);

class Socket {
	friend class Session;
	friend class TcpServer;
public:
	/// Constructor specifying type of socket and host address.
	Socket(int type);
	/// Destructor that closes sockets if required.
	virtual ~Socket();
	/// Writes bytes from buffer into socket. Returns bytes written or -1 if the socket is closed.
	virtual int Write(const char * fromBuffer, int maxBytesToWrite);
	/// Reads bytes from the socket. If the socket has closed -1 will be returned.
	virtual int Read(char * intoBuffer, int maxBytesToRead);

	/// If the delete flag has been set.
	bool ShouldDelete();
	/// Closes the socket. Returns false if any error occured.
	bool Close();
	/// Attempts to connect to target host.
	virtual bool ConnectTo(String hostAddress, int port);

	/// Performs a select check to see if it currently has any errors.
	bool HasError();

	/// Enables non-blocking mode.
	void SetNonBlocking(bool nonBlockingEnabled = true);

	/// Socket address, binary format with type of connection, port number, etc.
	sockaddr socketAddress;
	/// Peer-address, e.g. 127.0.0.1 for localhost. Identified when socket is connected.
	String peerAddress;
	/// from 1 to 65536 or something.
	int port;
	/// See SocketType above.
	int type;

	enum states {
		NULL_STATE,
		CONNECTED,
	};
	/// Simple enumerator to get an idea of how many times send has been used.
	int messagesSent;
	/// Current state of this socket.
	int state;
	/// Peer associated with this socket.
	Peer * peer;
	/// Retrieves the last error string, resetting it.
	String GetLastErrorString();

	/// For analysis, based on the amount of data sent since the last call to the same function.
	int BytesSentPerSecond();
	int BytesReceivedPerSecond();
	/// Total statistics
	int TotalBytesSent();
	int TotalBytesReceived();

protected:
	/// Bytes sent/received per second.
	int bsps, brps;
	/// Transmission data for later analysis.
	int bytesSent, bytesReceived;
	/// Previous values of the two above, for comparison later.
	int lastBytesSent, lastBytesReceived;
	/// For the perSecond-analysis.
	long long lastSentCheckTime, lastReceivedCheckTime;

	/// If delete flag is set to 1, it should be deleted. If set to 2, it should only be closed and remain re-usable in new connections. Default is 0.
	int deleteFlag;
	/// If set to non-blocking, flag this too since it affects certain operations.
	bool nonBlocking;
	String lastErrorString;
	/// Info about server to search for, found servers and selected server
	struct addrinfo hints, *servInfo, *server;
	/// Main socket
	SOCKET sockfd;

};

#endif
