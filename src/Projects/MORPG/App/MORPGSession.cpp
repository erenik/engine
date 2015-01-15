/// Emil Hedemalm
/// 2014-08-01
/// The network-session responsible for handling the game.

#include "MORPGSession.h"

#include "Message/MessageManager.h"

#include "MORPG/Character/Character.h"

#include "Network/Packet/Packet.h"

#include "MPackets.h"

#include "Globals.h"

MORPGSession::MORPGSession()
: GameSession("MORPGSessionX", "MORPG game", 0)
{
	self = NULL;
}

MORPGSession::~MORPGSession()
{
	SAFE_DELETE(self);
}

/// Reads packets, creating them and returning them for processing. Note that some packets will already be handled to some extent within the session (for exampling many SIP messages).
List<Packet*> MORPGSession::ReadPackets()
{
	// Eh... so what do I do here.
	List<Packet*> packetsReceived;
	const int packetBufferSize = 5000;
	char packetBuffer[packetBufferSize];

	/// Gather relevant sockets to check for received data.
	List<Socket*> socketsToCheck;
	if (isHost)
	{
		socketsToCheck = sockets;
	}
	else {
		if (hostSocket)
			socketsToCheck = hostSocket;
	}	

	/// Check all sockets now.
	for (int i = 0; i < socketsToCheck.Size(); ++i)
	{
		Socket * s = socketsToCheck[i];
		int bytesRead = s->Read(packetBuffer, packetBufferSize);
		// Read one packet each frame?
		if (bytesRead > 0)
		{
			MPacket * pack = new MPacket();
			pack->data.PushBytes((uchar*)packetBuffer, bytesRead);
			/// Assign socket it was sent from and extract peer if possible.
			pack->socket = s;
			/// Extract packet info?
			pack->ExtractData();
			packetsReceived.Add(pack);
		}
	}
	return packetsReceived;
};


/// Tries to login to the server (which should have been chosen before-hand).
void MORPGSession::Login(String userName, String passWord)
{
	// Success!
	MesMan.QueueMessages("LoginSuccessful");

	// Query our characters and character data from server?
	// ..

	// Select character!
	if (!self)
	{
		self = new Character();
	}
	self->name = "Me";
	/// Either set position as loaded from the server, or call some function to place our character somewhere else?
	self->position = Vector3f();

	// OK. We got info on what map we are located in, so load that map!
	String mapName = "Test";
	MesMan.QueueMessages("LoadMap:"+mapName);
}



