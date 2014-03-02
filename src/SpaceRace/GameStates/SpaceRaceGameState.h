/// Emil Hedemalm
/// 2014-02-02
/// Middle-class that provides general functionality like retrieving active players via the game session, etc.

#ifndef SPACE_RACE_GAME_STATE_H
#define SPACE_RACE_GAME_STATE_H

#include "GameStates/GameState.h"
#include "GameStates/GameStates.h"
#include "Game/GameConstants.h"
#include "SpaceRace/Network/SRSession.h"
#include "SpaceRace/SRPlayer.h"

class SpaceRaceGameState : public GameState
{
public:
	/// Retrieves the active gaming session (this assumes only one is active at a time).
	SRSession * GetSession();
	/// Returns player for target index. Returns NULL for invalid indices.
	SRPlayer* GetPlayer(int byIndex);
	/// Gets index for target player. Returns -1 for invalid players.
	int GetPlayerIndex(SRPlayer * player);
	/// Retrieves a list of active players.
	List<SRPlayer*> GetPlayers();
	/// Returns list of all local players.
	List<SRPlayer*> GetLocalPlayers();
};

#endif