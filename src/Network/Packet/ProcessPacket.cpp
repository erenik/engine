/// Emil Hedemalm
/// 2013-10-20
/// Processor function for all general connection packets (and some related, like FTP et cetera).
/// Also includes the definitions of the functions it uses to process the individual packets.

#include "../NetworkSettings.h"

#ifdef USE_NETWORK
/*

#include "../NetworkManager.h"
#include "../NetworkClient.h"
#include "Packets.h"
#include "../Packet.h"
#include "Message/MessageManager.h"
#include "Network/NetworkMessage.h"

#define print(a) // a 

	/// Establishes clients connection to the host, if not established, client will be DC:d
	void EstablishConnection(PacketEstablishConnection *pkt);
	/// Applies information about a client, then tell all clients about the new info
//	void ApplyClientInfo(PacketUpdateMyClientInfo *pkt);
	/// Updates all clients with information given from server
//	void UpdateClientsInfo(PacketBroadcastClientsInfo *pkt);

/// Handles networks managed by the network manager. Returns true if it processed the packet.
void NetworkManager::ProcessPacket(Packet * packet){
	switch(packet->packetType){
		case PACKET_CHAT:
			std::cout<<"\nChat packet intercepted. Do more with it?";
			break;
		case PACKET_VERSION_CHECK: {
			std::cout<<"\nVersion check packet received.. check it?";
			PacketVersionCheck * pvc = (PacketVersionCheck *)packet;
			float myGameEngineVersion = (float)GAME_ENGINE_VERSION;
			float myGameVersion = (float)GAME_VERSION;
			/// If host,..
			if(networkManager->GetUserType() == HOST){
				/// ..see if the client's version is correct or not.
				NetworkClient * client = networkManager->GetClient(pvc->sender);
				// If we have already made version check..
				if(client->connectionStatus == CONNECTION_STATUS_CONNECTING || client->connectionStatus == CONNECTION_STATUS_CONNECTED)
					break;
				// If we have not yet made version check
				else if(client->connectionStatus == CONNECTION_STATUS_PENDING)
					client->connectionStatus = CONNECTION_STATUS_CONNECTING; // Give client connecting status so we only verify him once

				// Do validation!
				bool valid = true;
				if(pvc->gameEngineVersion != myGameEngineVersion ||
					pvc->gameVersion != myGameVersion)
					valid = false;
				
				// If the client had different information/version
				// Reject his connection, and give him required version information
				if(!valid) {
					print("NetworkClient[" << pvc->clientName << "] failed to establish connection!");
					PacketVersionCheck * responsePacket = new PacketVersionCheck();
					responsePacket->gameEngineVersion = myGameEngineVersion;
					responsePacket->gameVersion = myGameVersion;
					networkManager->QueuePacket(responsePacket, pvc->sender);
					/// Notify the game/states of the new client.
					MesMan.QueueMessage(new OnClientConnected(client));
				}
				// Accept his connection
				else {
					strncpy(client->name, pvc->clientName, NAME_LIMIT);
					client->connectionStatus = CONNECTION_STATUS_CONNECTED;
					client->versionCheck = true;
					print("NetworkClient[" << pkt->clientName << "] successfully established connection!");
					PacketEstablishConnection * estPkt = new PacketEstablishConnection(pvc->sender);
					networkManager->QueuePacket(estPkt, pvc->sender);
				}
			}
			/// If client...
			else {
				/// .. receiving this message means we have an incorrect version. Compare it with the received info.
				std::stringstream ss;
				ss<<"Incorrect version.";
				ss<<"\nHost version: "<<pvc->gameEngineVersion<<", "<<pvc->gameVersion;
				ss<<"\nMy version:   "<<myGameEngineVersion<<", "<<myGameVersion;
				String errorString = String(ss.str().c_str());
				MesMan.QueueMessage(new OnConnectionFailed(errorString));
				// Call for version mismatch
				Network.info.connectionStatus = CONNECTION_STATUS_DISCONNECTING;
			}
			break;
		}
		case PACKET_ESTABLISH_CONNECTION: {
			/// Sent to client if version is OK.
			PacketEstablishConnection * pec = (PacketEstablishConnection *) packet;
			// Only clients should do establishing
			if(Network.GetUserType() == CLIENT)
			{
				Network.info.yourClientIndex = pec->clientIndex;
				Network.info.connectionStatus = CONNECTION_STATUS_CONNECTED;
				// Connection is established, fill in your client info
				NetworkClient *client = Network.GetClient(pec->clientIndex);
				client->versionCheck = true;
				client->connectionStatus = CONNECTION_STATUS_CONNECTED;
				strncpy(client->name, Network.info.name, NAME_LIMIT);
				// Also make sure that the host is version-checked
				Network.GetClient(0)->versionCheck = true;
				/// Notify the manager's that we're connected and can begin synchronizing game-specific data.
				MesMan.QueueMessage(new OnConnectionEstablished());
			}
			break;
		}
		case PACKET_UPDATE_MY_CLIENT_INFO: {
			PacketUpdateMyClientInfo * pumci = (PacketUpdateMyClientInfo*) packet; 
			// Only host should update a clients information and brodcast it
			if(Network.GetUserType() == HOST)
			{
				NetworkClient *client = Network.GetClient(pumci->sender);
				// Apply clients new information
				strncpy(client->name, pumci->name, NAME_LIMIT);
				strncpy(client->IP, pumci->IP, NAME_LIMIT);
				client->connectionStatus = pumci->connectionStatus;
				client->versionCheck = pumci->versionCheck;
				// Then update all clients about the new information
				Network.QueuePacket(new PacketBroadcastClientsInfo(), PACKET_TARGET_OTHERS);
			}
			break;
		}
		case PACKET_BROADCAST_CLIENTS_INFO: {
			PacketBroadcastClientsInfo * pbci = (PacketBroadcastClientsInfo*) packet;
			// Only clients should update their clients information, since the host is always right!
			if(Network.GetUserType() == CLIENT)
			{
				// Go through all clients and update them
				for(int i = 0; i < MAX_CLIENTS; ++i)
				{
					NetworkClient *client = Network.GetClient(i);
					// Update information
					strncpy(client->name, pbci->name[i], NAME_LIMIT);
					strncpy(client->IP, pbci->IP[i], NAME_LIMIT);
					client->connectionStatus = pbci->connectionStatus[i];
					client->versionCheck = pbci->versionCheck[i];
				}
			}
			break;
		}
		case PACKET_PING: case PACKET_PING_REPLY:
		case PACKET_REQUEST_CLIENT_INFO:
		case PACKET_MAKE_FTP_REQUEST:
		case PACKET_ANSWER_FTP_REQUEST:
		case PACKET_FTP_DATA:
		case PACKET_FTP_DATA_REPLY:
		case PACKET_FTP_FINISHED:
			assert(false && "Unhandled packets ;_;");
			break;		
	};
}
*/

#endif // USE_NETWORK