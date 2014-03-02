// Emil Hedemalm & Aron Wåhlberg
// 2013-07-08


#ifdef USE_NETWORK

#include "NetworkSettings.h"
#include <cstring>

#include "Packet/Packets.h"
#include "Packet.h"
#include "NetworkManager.h"
#include "NetworkClient.h"
#include <ctime>

extern int packet_size[PACKET_TYPES];

Packet::Packet(char type, char target)
{
	this->packetType = type;
	this->packetSize = packet_size[type];
	this->sender = Network.info.yourClientIndex;
	this->target = target;
	this->timeCreated = Timer::GetCurrentTimeMs();
}

//////////////////////////////////////////////////////////////////////////
// Packets

PacketChat::PacketChat( const char *msg ) : Packet(PACKET_CHAT)
{
	strncpy(message, msg, MAX_MESSAGE_LENGTH);
}

PacketVersionCheck::PacketVersionCheck() : Packet(PACKET_VERSION_CHECK)
{
	strncpy(clientName, Network.info.name, NAME_LIMIT);
	gameEngineVersion = (float)GAME_ENGINE_VERSION;
	gameVersion = (float)GAME_VERSION;
}
PacketEstablishConnection::PacketEstablishConnection( int clientIndex ) : Packet(PACKET_ESTABLISH_CONNECTION)
{
	this->clientIndex = clientIndex;
}
PacketUpdateMyClientInfo::PacketUpdateMyClientInfo() : Packet(PACKET_UPDATE_MY_CLIENT_INFO)
{
	NetworkClient *myClient = Network.GetClient(Network.info.yourClientIndex);
	strncpy(name, myClient->name, NAME_LIMIT);
	strncpy(IP, myClient->IP, NAME_LIMIT);
	connectionStatus = myClient->connectionStatus;
	versionCheck = myClient->versionCheck;
}
PacketBroadcastClientsInfo::PacketBroadcastClientsInfo() : Packet(PACKET_BROADCAST_CLIENTS_INFO)
{
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		NetworkClient *client = Network.GetClient(i);
		strncpy(name[i], client->name, NAME_LIMIT);
		strncpy(IP[i], client->IP, NAME_LIMIT);
		connectionStatus[i] = client->connectionStatus;
		versionCheck[i] = client->versionCheck;
	}
}

/// FTP Packets
#ifdef USE_FTP
PacketMakeFtpRequest::PacketMakeFtpRequest(FtpRequest *request) : Packet(PACKET_MAKE_FTP_REQUEST)
{
	this->files = 0;
	for(int i = 0; i < MAX_FTP_FILES; ++i)
	{
		if(request->files[i] == NULL){
			this->filesizes[i] = 0;
			this->packets[i] = 0;
			continue;
		}

		strncpy(this->filenames[i], request->files[i]->filename, MAX_FILENAME_LENGTH);
		this->filesizes[i] = request->files[i]->filesize;
		this->packets[i] = request->files[i]->packets;
		++this->files;
	}

	this->ftpID = request->id;
}

PacketAnswerFtpRequest::PacketAnswerFtpRequest(int ftpID, bool answer) : Packet(PACKET_ANSWER_FTP_REQUEST)
{
	this->ftpID = ftpID;
	this->answer = answer;
}

PacketFtpData::PacketFtpData( int ftpID, int fileIndex, int packetNumber, char data[MAX_FTP_DATA], unsigned long sizeToRead ) : Packet(PACKET_FTP_DATA)
{
	this->ftpID = ftpID;
	this->fileIndex = fileIndex;
	this->packetNumber = packetNumber;
	memcpy(this->data, data, MAX_FTP_DATA);
	this->sizeToRead = sizeToRead;
}

PacketFtpDataReply::PacketFtpDataReply( int ftpID, int fileIndex, int packetNumber ) : Packet(PACKET_FTP_DATA_REPLY)
{
	this->ftpID = ftpID;
	this->fileIndex = fileIndex;
	this->packetNumber = packetNumber;
}

PacketFtpFinished::PacketFtpFinished( int ftpID ) : Packet(PACKET_FTP_FINISHED)
{
	this->ftpID = ftpID;
}
#endif // USE_FTP

#endif
