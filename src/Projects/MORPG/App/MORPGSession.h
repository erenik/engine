/// Emil Hedemalm
/// 2014-08-01
/** The network-session responsible for handling the game.
	Client handling and some game logic may also be placed here, to some extent.
*/

#include "Network/Session/GameSession.h"

class Character;

class MORPGSession : public GameSession
{
public:
	MORPGSession();
	virtual ~MORPGSession();

	/// Reads packets, creating them and returning them for processing. Note that some packets will already be handled to some extent within the session (for exampling many SIP messages).
	virtual List<Packet*> ReadPackets();

	/// Tries to login to the server (which should have been chosen before-hand).
	void Login(String userName, String passWord);


	/// Our character we choose to login as! Should contain starting position, etc.
	Character * self;
	/// Our owned characters!
	List<Character*> characters;

protected:

};