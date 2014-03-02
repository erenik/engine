/// Emil Hedemalm
/// 2013-10-20
/// Thread that handles receivement of any packets through sockets.

#include "../NetworkSettings.h"

#ifdef USE_NETWORK
/*

#include "../NetworkManager.h"
#include "../NetworkClient.h"
#include "../NetworkStatus.h"
#include "../Packet/Packets.h"
#include "../Packet.h"

#include "Message/MessageManager.h"

extern int packet_size[PACKET_TYPES];

#define print(a) // a
#include "OS/Sleep.h"


#ifdef WINDOWS
void NetworkManager::PacketReceiver(void *vArgs)
#else
void * NetworkManager::PacketReceiver(void *vArgs)
#endif
{
	char buff [RECV_BUFFER_SIZE];
	int readySockets;			// For checking active/ready clients
	int byteInSock;				// Bytes received from socket
	int clientCount;			// Used because the amount of clients in the array can be sparse
	int bytesReceived;
	int amountClientsToCheck;
	fd_set socketsToCheck;
	timeval timeout10;
	timeout10.tv_usec = 10;

	print("Packet listener thread started..");

	while(networkManager->packetListenerActive){
	
		switch(networkManager->networkStatus){
			case NETWORK_STATUS_HOSTED:{
				clientCount = 0;
				amountClientsToCheck = Network.ActiveClients() - 1;
				#ifdef WINDOWS
				socketsToCheck.fd_count = amountClientsToCheck;
				#else
				FD_ZERO(&socketsToCheck);
				#endif
				if(amountClientsToCheck < 1){
					continue;
				}
				for(int i = 0; i < MAX_CLIENTS; ++i){
					if(Network.info.yourClientIndex == i)	// Skip ourselves
						continue;
					if(Network.clients[i].clientSock == NULL)
						continue;
					if(Network.clients[i].connectionStatus == CONNECTION_STATUS_NULL)
						continue;
                    #ifdef WINDOWS
					socketsToCheck.fd_array[clientCount++] = Network.clients[i].clientSock;
					#else
					FD_SET(Network.clients[i].clientSock, &socketsToCheck);
					if(clientCount < Network.clients[i].clientSock)
                        clientCount = Network.clients[i].clientSock;
					#endif
				}

				// Check the sockets
				readySockets = select(clientCount+1, &socketsToCheck, 0, 0, &timeout10);
				if(readySockets == SOCKET_ERROR){
					print("\nChecking clientSocks - select error: ");
					PrintNetworkError();
					// TODO: Stop Network? DC client?
				}
				else if(readySockets == 0){
					// Timeout, needed for adding new clients to listen to
					break;
				}
				#ifdef WINDOWS
				else if(readySockets != socketsToCheck.fd_count)
				{
					print("\nAmount of clients to check doesn't compare to amount selected.");
				}
				#endif

				// Receive messages from clients
				#ifdef WINDOWS
				for(int i = 0; i < readySockets; ++i){
				    SOCKET clientSock = socketsToCheck.fd_array[i];
					byteInSock = recv(clientSock, buff, RECV_BUFFER_SIZE, MSG_PEEK);
                #else
                for(int i = 0; i < MAX_CLIENTS; ++i){
                    if(Network.clients[i].connectionStatus == CONNECTION_STATUS_NULL)
                        continue;
				    SOCKET clientSock = Network.clients[i].clientSock;
                    if(!FD_ISSET(clientSock, &socketsToCheck))
                        continue;
                    byteInSock = recv(clientSock, buff, RECV_BUFFER_SIZE, MSG_PEEK);
                #endif

					// Error, check if DCing client
					if(byteInSock == SOCKET_ERROR){
						NetworkClient *client = NULL;
						// Find evil client to DC
						for(int j = 0; j < MAX_CLIENTS; ++j)
						{
							if(Network.GetClient(j)->clientSock == clientSock)
							{
								client = Network.GetClient(j);
								break;
							}
						}
						int error = sockerrno;
						if(client)
						{
                            #ifdef WINDOWS
							if(error != WSAECONNABORTED)
							{
								print("\nError when receiving bytes: ");
								PrintNetworkError(error);
							}
							#endif // WINDOWS
							print(client->name << " disconnected.");
							Network.ResetClient(client);
							print("\nAmount of active clients: " << Network.ActiveClients() << " / " << MAX_CLIENTS);
						}
					}
					else if(byteInSock == 0){	// Check for disconnection
						// Find evil client to DC
						for(int j = 0; j < MAX_CLIENTS; ++j)
						{
							NetworkClient *client = Network.GetClient(j);
							if(client->clientSock == clientSock)
							{
								print(client->name << " disconnected.");
								Network.ResetClient(client);
								print("\nAmount of active clients: " << Network.ActiveClients() << " / " << MAX_CLIENTS);
								break;
							}
						}
					}
					else{
						// Get client
						// if his connectiontype is DISSCONNECTING, set to CONNECTED

						// Handle the data
						char type = ((Packet *)buff)->packetType;
						int size = ((Packet *)buff)->packetSize;
						int sender = ((Packet *)buff)->sender;

						// If the sender of the packet didn't know his index
						if(sender == -1)
						{
							for(int j = 0; j < MAX_CLIENTS; ++j)
							{
								// Check if we found sender
								if(clients[j].clientSock == clientSock)
								{
									sender = j;
									break;
								}
							}
						}

						if(sender != -1 && sender < MAX_CLIENTS)
							clients[sender].lastMessage = ((Packet *)buff)->timeCreated;

						if(packet_size[type] != size){
							print("\nPacket size mismatch! Size: " << size << " / " << packet_size[type]);
							bytesReceived = recv(clientSock, buff, byteInSock, NULL);
							break;
						}
						else if(size > byteInSock){
							print("\nBytes in sock don't correspond to packet size: " << size << " / " << packet_size[type]);
							bytesReceived = recv(clientSock, buff, byteInSock, NULL);
							break;
						}

						char * newPacket = new char[size];
						// Make sure we read all data!
						bytesReceived = 0;
						do {
							bytesReceived += recv(clientSock, newPacket + bytesReceived, size - bytesReceived, NULL);
						} while (bytesReceived != size);

						if(bytesReceived == SOCKET_ERROR){
							print("\nSocket error when receiving!");
							break;
						}
						else
						{
							// Set sender if it was not specified
							if(((Packet *)newPacket)->sender == -1)
								((Packet *)newPacket)->sender = sender;

							//print("Packet ID: " << (int)type << " received from: " << (int)((Packet *)newPacket)->sender << " - " << ((((Packet *)newPacket)->sender != -1)?Network.clients[((Packet *)newPacket)->sender].name : ""));
							if(((Packet *)buff)->target == PACKET_TARGET_ALL || ((Packet *)buff)->target == PACKET_TARGET_OTHERS){
								Network.QueuePacket((Packet *)newPacket);
							}
							else if(((Packet *)buff)->target != Network.info.yourClientIndex){
								Network.QueuePacket((Packet *)newPacket);
							}
							else{
								/// Process general packets internally.
								networkManager->ProcessPacket((Packet*)newPacket);
								MesMan.QueuePacket((Packet *)newPacket);
							}
						}
					}
				}
				break;
			}
			case NETWORK_STATUS_JOINED:{
                SOCKET clientSock;
				#ifdef WINDOWS
				socketsToCheck.fd_count = 0;
				#else
				FD_ZERO(&socketsToCheck);
				#endif
				if(clients[0].clientSock){
                    #ifdef WINDOWS
					socketsToCheck.fd_array[0] = clients[0].clientSock;
					socketsToCheck.fd_count = 1;
					clientSock = socketsToCheck.fd_array[0];
					#else
					FD_SET(Network.clients[0].clientSock, &socketsToCheck);
					clientSock = clients[0].clientSock;
					#endif
				}
				else{
					// We don't have a host
					continue;
				}

				byteInSock = recv(clientSock, buff, RECV_BUFFER_SIZE, MSG_PEEK);

				if(byteInSock == SOCKET_ERROR){
					print("\nError when receiving bytes: ");
					PrintNetworkError();

					print("Lost connection to server!");
					Network.Shutdown();
					#ifdef WINDOWS
					return;
					#else
					return NULL;
					#endif
				}
				else if(byteInSock == 0){	// Check for disconnection
					// Disconnected by the host
					print("Lost connection to the server.");
					Network.Shutdown();
					#ifdef WINDOWS
					return;
					#else
					return NULL;
					#endif
				}
				else{
					// Handle the data
					unsigned char type = ((Packet *)buff)->packetType;
					unsigned long size = ((Packet *)buff)->packetSize;
					int sender = ((Packet *)buff)->sender;

					if(sender != -1 && sender < MAX_CLIENTS)
						clients[sender].lastMessage = ((Packet *)buff)->timeCreated;

					if(packet_size[type] != size){
						std::cout<<"\nPacket size mismatch! Size: " << size << " / " << packet_size[type]<<" for type "<<type;
						/// Receive data so that the buffer can be emptied?
						char* newPacket = new char[size];
						bytesReceived += recv(clientSock, newPacket + bytesReceived, size - bytesReceived, NULL);
						delete[] newPacket;
						break;
					}

					char* newPacket = new char[size];
					// Make sure we read all data!
					bytesReceived = 0;
					do {
						bytesReceived += recv(clientSock, newPacket + bytesReceived, size - bytesReceived, NULL);
					} while (bytesReceived != size);

					if(bytesReceived == SOCKET_ERROR){
						print("\nSocket error when receiving!");
						break;
					}
					else
					{
						//print("Packet ID: " << (int)type << " received from: " << (int)((Packet *)newPacket)->sender << " - " << ((((Packet *)newPacket)->sender != -1)?Network.clients[((Packet *)newPacket)->sender].name : ""));
						networkManager->ProcessPacket((Packet *) newPacket);
						MesMan.QueuePacket((Packet *)newPacket);
					}
				}
				break;
			}
			default:{
				print("Machine is neither Host nor NetworkClient.");
				print("Should not be receiving packets.");
				break;
			}
		}
	}
	Network.packetSenderThread = NULL;
}

*/

#endif // USE_NETWORK
