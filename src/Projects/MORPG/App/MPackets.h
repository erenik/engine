/// Emil Hedemalm
/// 2015-01-15
/// Packets used for the MORPG project.

#include "Network/Packet/Packet.h"


/**	Default Packets will feature a header as follows:
		MPacket: <PacketType>, e.g. MPacket: 4 for CLIENT_INFO
	Optionally the second row will feature the packet type in text for debugging purposes:
		PacketType: Requesting client info
*/

class MPacket : public Packet
{
public:
	MPacket();
	virtual ~MPacket();
	enum 
	{
		BAD_TYPE,
		
		// Base info.
		REQ_SERV_INFO,
		SERV_INFO,
		REQ_CLIENT_INFO,
		CLIENT_INFO,
		
		// Authentication
		LOGIN,
		LOGIN_RESULT,
		LOGOUT,
		LOGOUT_RESULT,

		// Notices, warnings, errors.
		SERV_MSG,
		SERV_WARN,
		SERV_ERR,

		// Player's character list.
		REQ_CHARACTERS,
		CHARACTERS,
		REQ_CREATE_CHARACTER, // Creating a new character
		CREATE_CHARACTER, // Result of char creation.
		SELECT_CHARACTER, // Choosing a character to enter the world with

		// Loading map data.
		REQ_MAP, // Request map to load. 
		MAP, // Map to load, including file-size and version. REQ_POSITION should be sent straight after receiving this to place the player and camera accordingly.
		REQ_MAP_PCS, // Requests map PCs surrounding the player.
		MAP_PCS, // Returns active PCs close enough to the character. This should include a set of IDs.
		REQ_PC, // Requests comprehensive data of target PC. This should include name, model, texture, transform, etc.?
		PC, // Contains all above-mentioned data of a target PC.
		REQ_TRACK_PC, // Request to track movement and status of target PC, including all animation.
		TRACK_PC, // Contains track data for target PC. Can also contain approval/failure reports of a request.
		
		// Moving self
		REQ_POSITION,
		POSITION, // Correcting position if player trying to cheat or lagging too much. Can also be an approval of a REQ.
		REQ_EMOTE, // Request to perform an emote animation/text display. 
		
		// Communication.
		EVENT, // This can be any "story-telling", or custom player emotes.
		SAY,
		WHISPER,
		YELL,
		
		// Character info.
		REQ_CHAR_INFO,
		CHAR_INFO,
		REQ_EQUIP, // Equipped gear
		EQUIP,
		REQ_INV, // Inventory
		INV,

		// 

	};

	/// What type is it (see above), from a MORPG perspective?
	int mType;

	/// Extracts data.
	void ExtractData();
	/// Builds the data stream, based on packet-type and provided data.
	void BuildDataStream();

};





