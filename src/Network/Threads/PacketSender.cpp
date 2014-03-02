/// Emil Hedemalm
/// 2013-10-20
/// Thread that handles posting of any packets through sockets.

#include "../NetworkSettings.h"

#ifdef USE_NETWORK

/*

#include "../NetworkManager.h"
#include "../NetworkClient.h"
#include "../Packet/Packets.h"
#include "../Packet.h"
#include "Message/MessageManager.h"

#include "OS/Sleep.h"

#define print(a) // a

extern int packet_size[PACKET_TYPES];

#ifdef WINDOWS
void NetworkManager::PacketSender(void *vArgs)
#else
void * NetworkManager::PacketSender(void *vArgs)
#endif
{

	// Disable Nagle's algorithm
	/*
	flag = 1;
	int result = setsockopt(m_Socket,IPPROTO_TCP,TCP_NODELAY,(char *) &flag,sizeof(int));
	*/
/*
	Packet *packet;
	unsigned long packetSize;
	int packetTarget;
	int packetType;
	int bytesSent;
	int targets = 0;
	NetworkClient *targetClients[MAX_CLIENTS];
	Network.pingTimer.Start();

	print("Packet sender thread started..");

	while(Network.packetSenderActive)
	{
		// First ping
		#ifdef USE_PING
		Network.Ping();
		#endif

		// Then check if we need to send FTP data
		#ifdef USE_FTP
		Ftp.SendFtp();
		#endif

		// Then check if there are any packets in the packet queue
		if(!Network.packetQueue.isOff())
		{
			targets = 0;
			memset(targetClients, NULL, sizeof(NetworkClient *) * MAX_CLIENTS);

			/// TODO: Consider funcitonizing this whole send-packet block, using Mutexes for the packetQueue at the start/end.
			packet = (Packet *)Network.packetQueue.Pop();
			packetSize = packet->packetSize;
			packetType = packet->packetType;
			packetTarget = packet->target;

			// Discard invalid packets
			if(packetSize <= 0)
			{
				print("Discarding packet type: " << packet->packetType << " with size of 0.");
				delete packet;
				continue;
			}


			// First we must get the targets to send to and valify them
			// If we are Host
			if(Network.userType == HOST)
			{
				// Check if we're sending to all players or others
				if(packetTarget == PACKET_TARGET_ALL || packetTarget == PACKET_TARGET_OTHERS)
				{
					// Go through all clients/users
					for(int i = 0; i < MAX_CLIENTS; ++i)
					{
						// Don't send to inactive players, but send to pending, disconnecting and error clients
						if(clients[i].connectionStatus == CONNECTION_STATUS_NULL)
							continue;

						// Check if we're at our client
						if(i == networkManager->info.yourClientIndex)
						{
							// Don't send to ourselves, just Queue it
							if(packetTarget == PACKET_TARGET_ALL || (packetTarget == PACKET_TARGET_OTHERS && packet->sender != Network.info.yourClientIndex))
							{
								// Allocate memory for packet
								char *newPacket = (char *)malloc(packet->packetSize);
								// Copy all data
								memcpy(newPacket, packet, packet_size[packetType]);
								// Make it a Packet and queue it
								MesMan.QueuePacket((Packet *)newPacket);
								continue;
							}
							// Else we're sending to all others than ourselves
							else
							{
								continue;
							}
						}

						// Don't send a copy to the sender either
						if(i == packet->sender)
							continue;

						// Save the target
						targetClients[targets++] = &clients[i];
					}
				}
				// Else we're sending to a single player
				else
				{
					// Don't send to inactive players
					if(clients[packetTarget].connectionStatus == CONNECTION_STATUS_NULL)
					{
						print("Packet's target player is invalid.");
					}
					// Check if we're sending to ourselves
					else if(Network.info.yourClientIndex == packet->target)
					{
						// Allocate memory for packet
						char *newPacket = (char *)malloc(packet->packetSize);
						// Copy all data
						memcpy(newPacket, packet, packet_size[packetType]);
						// Make it a Packet and queue it
						MesMan.QueuePacket((Packet *)newPacket);
					}
					// Otherwise, add the target client
					else
					{
						targetClients[targets++] = &clients[packetTarget];
					}
				}
			}
			else if(Network.userType == CLIENT)
			{
				// Don't send to inactive host
				if(clients[0].connectionStatus != CONNECTION_STATUS_CONNECTED)
					continue;

				// Send to the host if we're not only targeting ourselves
				if(Network.info.yourClientIndex != packetTarget)
					targetClients[targets++] = &clients[0];

				// Check if we're sending to all or ourselves
				if(packetTarget == PACKET_TARGET_ALL || Network.info.yourClientIndex == packetTarget)
				{
					// Allocate memory for packet
					char *newPacket = (char *)malloc(packet->packetSize);
					// Copy all data
					memcpy(newPacket, packet, packet_size[packetType]);
					// Make it a Packet and queue it
					MesMan.QueuePacket((Packet *)newPacket);
				}
			}


			// Send the packets
			if(targets != 0)
			{
				for(int i = 0; i < targets; ++i)
				{
					// If we're sending with TCP
					if(Network.connectionType == SOCK_STREAM)
					{
						// Don't send to invalid sockets
						if(!targetClients[i]->clientSock)
							continue;

						// Make sure we're sending all data!
						bytesSent = 0;
						do {
							int status = send(targetClients[i]->clientSock, (char *)packet + bytesSent, packetSize - bytesSent, NULL);
							// Check if we got error
							if(status == SOCKET_ERROR){
                                PrintNetworkError();
                                break;
							}
							bytesSent += status;
						} while(bytesSent != packetSize);
					}
					// Else we're sending with UDP
					else
					{
						//bytesSent = sendto(Network.sockfd, (char *)packet, packetSize, NULL, (SOCKADDR *)&targetClients[i]->clientAddr, targetClients[i]->clientAddrSize);
					}

					// Check for error
					if(bytesSent == SOCKET_ERROR)
					{
						print("Failed to send packet of type: " << int(packetType) << " to target: " << packetTarget);
						PrintNetworkError();
					}
					else if(bytesSent != packetSize)
					{
						print("Packet size: " << packetSize << " mismatch send size: " << bytesSent);
					}
					else
					{
						//print("Packet ID: " << (int)packet->packetType << " sent to: " << (int)packet->target << " - " << ((packet->target != -1)?Network.clients[packet->target].name : ""));
					}
				}
			}

			// Delete the packet
			delete packet;
		}
		else
		{
		}
	}
	Network.packetListenerThread = NULL;
}
*/
#endif
