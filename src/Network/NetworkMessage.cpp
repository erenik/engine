/// Emil Hedemalm
/// 2013-10-20
/// Messages that will be posted to the MessageManager when events occur in the NetworkManager


#include "Message/MessageTypes.h"
#include "NetworkMessage.h"

OnServerStarted::OnServerStarted():Message(NETWORK_SERVER_STARTED) 
{
}

OnClientConnected::OnClientConnected(NetworkClient * client)
: Message(NETWORK_CLIENT_CONNECTED),
nc(client)
{
}

OnClientDisconnected::OnClientDisconnected(NetworkClient * client)
: Message(NETWORK_CLIENT_DISCONNECTED),
nc(client)
{
}

/// Client-side packets
OnConnectionEstablished::OnConnectionEstablished()
: Message(NETWORK_CONNECTION_ESTABLISHED)
{

}
OnConnectionFailed::OnConnectionFailed(String message)
: Message(NETWORK_CONNECTION_FAILED),
errorMessage(message)
{

}

