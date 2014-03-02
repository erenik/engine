/// Emil Hedemalm
/// 2014-02-02
/// Middle-class that provides general functionality like retrieving active players via the game session, etc.

#include "SpaceRaceGameState.h"
#include "Network/NetworkManager.h"
#include "Network/Session/SessionTypes.h"
#include "Network/Session/GameSessionTypes.h"

/// Retrieves the active gaming session (this assumes only one is active at a time).
SRSession * SpaceRaceGameState::GetSession()
{
	/// Gets session from network-manager. If no session exists, it will create one.
	Session * s = NetworkMan.GetSession(SessionType::GAME, GameSessionType::SPACE_RACE);
	assert(s);
	return (SRSession*)s;
}

/// 
SRPlayer* SpaceRaceGameState::GetPlayer(int byIndex)
{
	SRSession * s = GetSession();
	assert(s);
	return s->GetPlayer(byIndex);
}

/// Gets index for target player.
int SpaceRaceGameState::GetPlayerIndex(SRPlayer * player)
{
	List<SRPlayer*> players = GetPlayers();
	for (int i = 0; i <  players.Size(); ++i){
		if (players[i] == player)
			return i;
	}
	return -1;
}

/// Retrieves a list of active players.
List<SRPlayer*> SpaceRaceGameState::GetPlayers()
{
	SRSession * s = GetSession();
	assert(s && "No valid session to fetch players from D:");
	return s->GetPlayers();	
}

/// Returns list of all local players.
List<SRPlayer*> SpaceRaceGameState::GetLocalPlayers()
{
	SRSession * s = GetSession();
	assert(s && "No valid session to fetch players from D:");
	return s->GetLocalPlayers();	
}