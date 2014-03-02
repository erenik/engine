/// Emil Hedemalm
/// 2013-10-20
/// Messages that will be posted to the MessageManager when events occur in the NetworkManager

#include "Message/Message.h"

enum networkMessages {
	NETWORK_NULL_MESSAGE = MessageType::FIRST_NETWORK_MESSAGE,
	/// Server-side
	NETWORK_SERVER_STARTED,
	NETWORK_SERVER_STOPPED,
	NETWORK_CLIENT_CONNECTING,
	NETWORK_CLIENT_CONNECTED,
	NETWORK_CLIENT_TIME_OUT,
	NETWORK_CLIENT_DISCONNECTING,
	NETWORK_CLIENT_DISCONNECTED,
	/// Client-side
	NETWORK_CONNECTION_FAILED,
	NETWORK_CONNECTION_ESTABLISHED,
};

class NetworkClient;

class OnServerStarted : public Message {
public:
	OnServerStarted();
};
class OnServerStopped : public Message {
public:
	OnServerStopped();
};
class OnClientConnecting : public Message {
public:
	OnClientConnecting();
};
class OnClientConnected : public Message {
public:
	OnClientConnected(NetworkClient * nc);
	NetworkClient * nc;
};

class OnClientTimeOut : public Message {
	OnClientTimeOut(NetworkClient * nc);
	NetworkClient * nc;
};
class OnClientDisconnecting : public Message {
	OnClientDisconnecting(NetworkClient * nc);
	NetworkClient * nc;
};
class OnClientDisconnected : public Message {
public:
	OnClientDisconnected(NetworkClient * nc);
	NetworkClient * nc;
};


// Client-side.
class OnConnectionEstablished : public Message {
public:
	OnConnectionEstablished();
};
/// Failing to connect.
class OnConnectionFailed : public Message {
public:
	/// Include host and/or error info?
	OnConnectionFailed(String message);
	String errorMessage;
};
