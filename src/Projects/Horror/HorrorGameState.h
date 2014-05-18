// Emil Hedemalm
// 2015-05-17

#include "GameStates/GameState.h"


class HorrorGameState : public GameState 
{
public:
	HorrorGameState();
	void OnEnter(GameState *);
	void Process(float);
	void OnExit(GameState *);
	void ProcessMessage(Message * message);
	void CreateDefaultBindings();
private:
	// Clear world. Spawn character. 
	void NewGame();
	// New room! o-o
#define CreateRoom SpawnRoom
	void SpawnRoom(Vector2i index, Vector3f atLocation);

	enum cameraTypes{
		GLOBAL,
		TRACK_PLAYER,
		CAMERA_TYPES
	};

	bool forward, backward, left, right;

	// Updates camera depending on type.
	void OnCameraUpdated();

#define ROOM_NAME "Horror/4Doors.obj"
	// Global or tracking an entity?
	int cameraType;
	Entity * player;
	// Based on the model?
	float roomSize;
	float scale;
};

