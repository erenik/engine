/// Emil Hedemalm
/// 2013-10-20
/// Thread for handling incoming connections.

#include "../NetworkSettings.h"

#ifdef USE_NETWORK

#include "../NetworkManager.h"
#include "../NetworkClient.h"

#define print(a) // a

/*

#ifdef WINDOWS
void NetworkManager::AcceptClients(void *vArgs)
#else
void * NetworkManager::AcceptClients(void *vArgs)
#endif
{
	print("\nBegin checking for pending connections.");
	if(listen(Network.sockfd, MAX_CLIENTS) == SOCKET_ERROR)
	{
		print("Failed to start listening from socket!");
		PrintNetworkError();
		Network.Shutdown();
		//Network.clientListenerActive = false;
	}


	while(Network.clientListenerActive){

		sockaddr_in clientAddress;
		socklen_t addressSize = sizeof(clientAddress);

		// Wait here 'til receiving a client connection
		SOCKET newClientSocket = accept(sockfd, (sockaddr*)&clientAddress, &addressSize);

		print("\nReceiving connection from a client..");

		if(newClientSocket == INVALID_SOCKET){
			print("\nClient accept error: ");
			PrintNetworkError();
			continue;
		}

		// Make sure there is room for the client
		int clientIndex;
		for(clientIndex = 0; clientIndex < MAX_CLIENTS; ++clientIndex)
		{
			if(clients[clientIndex].connectionStatus == CONNECTION_STATUS_NULL)
				break;
		}

		// Check if we're disconnecting the new client because the server is full
		if(clientIndex == MAX_CLIENTS)
		{
			print("Could not accept any more clients.");
			CloseSocket(newClientSocket);
			continue;
		}

		int x = 1;
		// If TCP, setup socket
		if(Network.connectionType == SOCK_STREAM)
		{
			// Disables internal buffering mechanism, speedier sending of packets
			setsockopt(newClientSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&x, sizeof(x));
		}
		setsockopt(newClientSocket, SOL_SOCKET, SO_DONTLINGER, (char *)&x, sizeof(x));
		// Assign socket as non-blocking so it doesn't stop at receive calls ie.
		unsigned long iMode = 1;		// Set mode to non-blocking. 0 = blocking, 1 = non-blocking
		#ifdef WINDOWS
		int result = ioctlsocket(newClientSocket, FIONBIO, &iMode);
		#else
		int result = ioctl(newClientSocket, FIONBIO, &iMode);
		#endif
		if(result == SOCKET_ERROR){
			print("\nioctlsocket failed with error: ");
			PrintNetworkError();
			CloseSocket(newClientSocket);
			continue;
		}

		// Assign client socket
		NetworkClient *client = &clients[clientIndex];
		client->connectionStatus = CONNECTION_STATUS_PENDING;
		client->clientSock = newClientSocket;
		client->index = clientIndex;
		
		print("\nClient accepted.");

		print("\nAmount of active clients: " << Network.ActiveClients() << " / " << MAX_CLIENTS);

	}
	Network.clientListenerThread = NULL;
}

*/

#endif
