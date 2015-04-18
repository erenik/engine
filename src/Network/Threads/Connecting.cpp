/// Emil Hedemalm
/// 2013-10-20
/// Handles the connecting handshake from the client-side.

#include "../NetworkSettings.h"

#ifdef USE_NETWORK
/*

#include "../NetworkManager.h"
#include "Message/MessageManager.h"

#include "../NetworkMessage.h"
#include "../Packet.h"
#include "../Packet/Packets.h"

#include "OS/Sleep.h"

#define print(a) // a;


#ifdef WINDOWS
void NetworkManager::Connecting(void *vArgs)
#else
void * NetworkManager::Connecting(void *vArgs)
#endif
{
	print("Trying to establish connection..");
	// Tries before giving up
	int tries = 3;

	while(Network.connectingActive)
	{
		// Let the other threads start a little
		SleepThread(200);

		// Check if we connected
		if(Network.info.yourClientIndex != -1)
		{
			switch(Network.info.connectionStatus)
			{
				// TODO: fix here, WTF!?
				case CONNECTION_STATUS_NULL:
					print("CONNECTION_STATUS_NULL");
					break;
				case CONNECTION_STATUS_PENDING:
					print("CONNECTION_STATUS_PENDING");
					break;
				case CONNECTION_STATUS_CONNECTING:
					print("CONNECTION_STATUS_CONNECTING");
					break;
				case CONNECTION_STATUS_CONNECTED:
					print("Esablished connection");
					Network.QueuePacket(new PacketUpdateMyClientInfo(), PACKET_TARGET_HOST);
					break;
				case CONNECTION_STATUS_DISCONNECTING:
					print("Could not esablish connection.");
					Network.Shutdown();
					break;
				case CONNECTION_STATUS_ERROR:
					print("Error when esablishing connection.");
					break;
			}
			Network.connectingActive = false;
		}
		else
		{
			switch(Network.info.connectionStatus)
			{
				case CONNECTION_STATUS_DISCONNECTING:
					print("Could not esablish connection.");
					Network.Shutdown();
			//		ChatMan.AddMessage(new ChatMessage(NULL, "Could not establish connection."));
					break;
				case CONNECTION_STATUS_ERROR:
					print("Error when esablishing connection.");
			//		ChatMan.AddMessage(new ChatMessage(NULL, "Error when establishing connection."));
					Network.Shutdown();
					break;
			}
		}

		// Check if we didn't connect
		if(Network.info.yourClientIndex == -1 && Network.info.connectionStatus == CONNECTION_STATUS_CONNECTING)
		{
			if(tries-- == 0)
			{
				print("Could not esablish connection.");
				Network.info.connectionStatus = CONNECTION_STATUS_DISCONNECTING;
				/// Include some more error text in the message?
				MesMan.QueueMessage(new OnConnectionFailed("Could not esablish connection."));
			}
			else
                Network.info.connectionStatus = CONNECTION_STATUS_PENDING;
		}
		// Check if we need to send our information
		if(Network.info.yourClientIndex == -1 && Network.info.connectionStatus == CONNECTION_STATUS_PENDING)
		{
			Network.info.connectionStatus = CONNECTION_STATUS_CONNECTING;
			PacketVersionCheck *packet = new PacketVersionCheck();
			Network.QueuePacket(packet, PACKET_TARGET_HOST);
		}
	}
	Network.connectingThread = NULL;
}

*/

#endif // USE_NETWORK
