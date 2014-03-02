// Emil Hedemalm
// 2013-07-10

#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

class Player;
#include <String/AEString.h>

#define PlayerMan (*PlayerManager::Instance())

/** All maximii should be set upon entering your game's global-state!
	Default is 4 local, 12 network, 16 max.
*/
class PlayerManager {
	PlayerManager();
	~PlayerManager();
	static PlayerManager * playerManager;
public:
	static bool IsAllocated();
	static void Allocate();
	static void Deallocate();
	static PlayerManager * Instance();
	int NumPlayers() { return players.Size(); };
	Player * Get(int playerIndex);
	/// Check if a player with given name exists.
	bool Exists(String playerWithName);
	/// Returns the player's index (0 and up) if successful, -1 upon failure! Adding a player will automatically assign it a vacant input device.
	int AddPlayer(Player * p);
	/// True if successful
	bool RemovePlayer(Player * p);
	/// Wosh.
	void RemoveAllNetworkPlayers();

    /// Returns you! The 0-indexed local-player of this machine, or the static-host player if there are no local players.
    Player * Me();

	void SetMaxPlayers(int n);

	int  MaxPlayers() const { return maxPlayers; };
	int MaxLocalPlayers() { return maxLocalPlayers;};
	void SetMaxLocalPlayers(int n);

	Player * GetPlayer(int index);
	Player * GetPlayerByID(int ID);
	Player * GetPlayerByClientIndex(int clientIndex);
	List<Player*> GetPlayers(){ return players; };
	List<Player*> GetLocalPlayers(){ return localPlayers; };
	List<Player*> GetAIPlayers() {return aiPlayers;};

    /** Sets player input device! Works with negative indices and over maximum as they will be recycled to valid numbers.
        Returns the player's new device ID.
    */
	int SetPlayerInputDevice(Player * player, int deviceEnum);
	Player * GetPlayerByActiveInputDevice(int inputDeviceEnum);
	/// Prints the player's input-devices.
	void PrintPlayerInputDevices();
	/// Returns the ID of the first input-device that is not already claimed by another local player
	int GetFirstVacantInputDevice();
private:
	/// Meeee
    Player * me;

	List<Player*> players;
	List<Player*> aiPlayers;
	List<Player*> localPlayers;
	List<Player*> networkPlayers;
	int maxLocalPlayers;
	int maxNetworkPlayers;
	int maxPlayers;
};

#endif
