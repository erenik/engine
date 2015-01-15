/// Emil Hedemalm
/// 2015-01-15
/// Packets used for the MORPG project.

#include "MPackets.h"
#include "Network/Packet/PacketTypes.h"

MPacket::MPacket() 
: Packet(PacketType::MORPG_PACKET)
{
	mType = MPacket::BAD_TYPE;
}
MPacket::~MPacket()
{
}

/// Extracts data.
void MPacket::ExtractData()
{
	String textData;
	bool ok = data.GetDataAsString(textData);
	assert(ok);
	// Extract first row.
	String firstRow ;
	
	// Assume text-packet.

}

/// Builds the data stream, based on packet-type and provided data.
void MPacket::BuildDataStream()
{
	/// Add header
	String firstRow = "MPacket: " + String::ToString(mType);
	data.PushLine(firstRow);
	switch(mType)
	{
		case REQ_SERV_INFO:
		{
			data.PushLine("PacketType: Requesting server info");
			break;	
		}
	}

	/// Build first version of the data stream.

	/// Encrypt all other packages?
	

}






