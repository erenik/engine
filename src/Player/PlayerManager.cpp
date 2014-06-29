// Emil Hedemalm
// 2013-07-10

#include "Player.h"
#include "PlayerManager.h"
#include "List/List.h"
#include "Input/InputDevices.h"
#include "Network/NetworkManager.h"

PlayerManager * PlayerManager::playerManager = NULL;

using std::cout;

PlayerManager::PlayerManager(){
	maxLocalPlayers = 4;
	maxNetworkPlayers = 12;
	maxPlayers = 16;
	me = NULL;
}

PlayerManager::~PlayerManager(){
	std::cout<<"\nDeleting player manager.";
	CLEAR_AND_DELETE(players);
	if (me)
        delete me;
    me = NULL;
}

bool PlayerManager::IsAllocated(){
	return playerManager? true : false;
}
void PlayerManager::Allocate(){
	assert(playerManager == NULL);
	playerManager = new PlayerManager();
}
void PlayerManager::Deallocate(){
	assert(playerManager);
	delete playerManager;
	playerManager = NULL;
}
PlayerManager * PlayerManager::Instance(){
	assert(playerManager);
	return playerManager;
}

Player * PlayerManager::Me() {
    if (localPlayers.Size())
        return localPlayers[0];
    if (me)
        return me;
    me = new Player();
//    localMachine->name = Network.MyIP();
	return me;
}

Player * PlayerManager::Get(int playerIndex){
	assert(playerManager);
	assert(players.Size());
	return players[playerIndex];
}

/// Check if a player with given name exists.
bool PlayerManager::Exists(String playerWithName){
	for (int i = 0; i < players.Size(); ++i){
		Player * p = players[i];
		if (p->name == playerWithName)
			return true;
	}
	return false;
}

Player * PlayerManager::GetPlayerByClientIndex(int clientIndex){
    for (int i = 0; i < players.Size(); ++i){
        Player * p = players[i];
        if (p->clientIndex == clientIndex)
            return p;
    }
    return NULL;
}

/// Returns the player's index (0 and up) if successful, -1 upon failure!
int PlayerManager::AddPlayer(Player * p){
	assert(players.Size() < maxPlayers);
	if (players.Size() >= maxPlayers)
		return -1;
	
	/// Assign the player a unique ID before continuing!
	/// Also, screw IDs if not in multiplayer mode.
	/*
	if (NetworkMan.IsActive()){
		int newID = rand();
		bool idGood = false;
		while (!idGood){
			idGood = true;
			for (int i = 0; i < players.Size(); ++i){
				Player * p = players[i];
				if (p->id == newID){
					idGood = false;
					break;
				}
			}
			if (!idGood)
				newID = rand();
		};
		p->id = newID;
	}
	*/
	if (p->isAI){
		aiPlayers.Add(p);
	}
	else if (p->isLocal){
		// If local player, assign an input device!
		p->inputDevice = GetFirstVacantInputDevice();
		assert(localPlayers.Size() < maxLocalPlayers);
		if (localPlayers.Size() >= maxLocalPlayers)
			return -1;
		localPlayers.Add(p);
	}
	else {
	    assert(networkPlayers.Size() < maxNetworkPlayers);
		if (networkPlayers.Size() >= maxNetworkPlayers)
			return -1;
		networkPlayers.Add(p);
	}
	players.Add(p);
	return players.GetIndexOf(p);
}

/// True if successful
bool PlayerManager::RemovePlayer(Player * p){
	assert(players.Exists(p));
	if (p->isAI){
		assert(aiPlayers.Exists(p));
		aiPlayers.Remove(p);
	}
	else if (p->isLocal){
		assert(localPlayers.Exists(p));
		localPlayers.Remove(p);
	}
	else {
		assert(networkPlayers.Exists(p));
		networkPlayers.Remove(p);
	}
	players.Remove(p);
	delete p;
	return true;
}

/// Wosh.
void PlayerManager::RemoveAllNetworkPlayers(){
	assert(playerManager);
    while (networkPlayers.Size()){
        RemovePlayer(networkPlayers[0]);
    }
}

void PlayerManager::SetMaxPlayers(int n){
	if (n < players.Size()){
		std::cout<<"\nERROR: Remove a few players in the slots before reducing the maximum!";
		assert(false && "Handle appropriately!");
		return;
	}
	maxPlayers = n;
}

Player * PlayerManager::GetPlayerByID(int ID){
	for (int i = 0; i < players.Size(); ++i){
		Player * p = players[i];
		if (p->id == ID)
			return p;
	}
	return NULL;
}

Player * PlayerManager::GetPlayer(int index){
	assert(index >= 0 && index < players.Size());
	return players[index];
}

/** Sets player input device! Works with negative indices and over maximum as they will be recycled to valid numbers.
    Returns the player's new device ID.
*/
int PlayerManager::SetPlayerInputDevice(Player * player, int deviceEnum){
    while (deviceEnum >= InputDevice::MAX_INPUT_DEVICES){
        deviceEnum -= InputDevice::MAX_INPUT_DEVICES;
    }
    while(deviceEnum < 0) {
        deviceEnum += InputDevice::MAX_INPUT_DEVICES;
    }
    player->inputDevice = deviceEnum;
    std::cout<<"\nPlayer "<<player->name<<" input device set to "<<deviceEnum;
    std::cout<<"\nDevice name: "<<GetInputDeviceName(deviceEnum);
    return deviceEnum;
}

/// Look at InputManager for this one...
Player * PlayerManager::GetPlayerByActiveInputDevice(int inputDeviceEnum){
	for (int i = 0; i < localPlayers.Size(); ++i){
		if (localPlayers[i]->InputDevice() == inputDeviceEnum)
			return localPlayers[i];
	}
	return NULL;
}

/// Prints the player's input-devices.
void PlayerManager::PrintPlayerInputDevices(){
	for (int i = 0; i < localPlayers.Size(); ++i){
	    Player * p = localPlayers[i];
		std::cout<<"\nPlayer "<<i+1<<" input device: "<<GetInputDeviceName(p->inputDevice);
	}
}

/// Returns the ID of the first input-device that is not already claimed by another local player, or -1 if none is vacant.
int PlayerManager::GetFirstVacantInputDevice(){
	for (int i = FIRST_VALID_INPUT_DEVICE; i < InputDevice::MAX_INPUT_DEVICES; ++i){
		bool claimed = false;
		for (int p = 0; p < localPlayers.Size(); ++p){
			if (localPlayers[p]->InputDevice() == i)
				claimed = true;
		}
		if (claimed == false)
			return i;
	}
	return -1;
}
