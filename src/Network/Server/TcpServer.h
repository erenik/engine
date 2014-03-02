/// Emil Hedemalm
/// 2014-01-24
/// A server handling incoming connections.

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "Network/NetworkIncludes.h"
#include <String/AEString.h>

class Socket;

namespace ServerStatus {
enum serverStatus{
	NOT_STARTED,
	RUNNING,
};};

class TcpServer {
public:
	TcpServer();
	/// Starts listening. Returns false if the network interfaces are busy/inaccessible.
	bool Start();
	/// Stops listening, unbinding the listen socket.
	void Stop();
	/// Returns the next pending connection, if any available. If not, returns NULL.
	Socket * NextPendingConnection();

	void SetServerPort(int port);

	String GetLastErrorString();

	int Port(){return port;};
private:
	/// Closes ze socket, setting it to NULL upon success.
	void CloseSocket(SOCKET & sock);

	/// Fer debugsing
	String lastErrorString;
	/// Port
	int port;
	int status;
	/// Info about server to search for, found servers and selected server
	struct addrinfo hints, *servInfo, *server;
	/// Main socket
	SOCKET listenSocket;

};

#endif
