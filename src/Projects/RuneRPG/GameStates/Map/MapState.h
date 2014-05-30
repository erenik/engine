// Emil Hedemalm
// 2013-06-17

#ifndef MAP_STATE_H
#define MAP_STATE_H

#include "Game/GameConstants.h"
#include "../RRGameState.h"
#include "GameStates/GameStates.h"
#include "Selection.h"

class RuneEntity;
class RRPlayer;
class RREntityState;
class RuneShop;

namespace EnterMode {
enum enterModes{
	NULL_MODE,
	NEW_GAME,
	CONTINUE,
	LOAD_GAME,
	TEST_LEVEL, TESTING_MAP = TEST_LEVEL,
	MAX_ENTER_MODES,
}; };

class TileMap2D;

/// When walking around on the "maps"
class MapState : public RRGameState {
	friend class RuneEditor;
	friend class EditorState;
public:
	MapState();
	/// Virtual destructor so parent-class desctructor callback re-direction will work correctly.
	virtual ~MapState();
	void OnEnter(GameState * previousState);
	void Process(float time);
	void OnExit(GameState * nextState);
	/// Input functions for the various states
	void MouseClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	void MouseRightClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	virtual void MouseMove(int x, int y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL);
	void MouseWheel(float delta);
	/// Callback from the Input-manager, query it for additional information as needed.
	void KeyPressed(int keyCode, bool downBefore);
	void CreateDefaultBindings();
	void CreateUserInterface();
	void InputProcessor(int action, int inputDevice = 0);
	void ProcessMessage(Message * message);

	/// New game, load game, testing map, etc.
	void SetEnterMode(int mode);
	void SetCamera(Camera & reference);

private:
	/// o-o
	void SpawnNPC(String fromRef, int x, int y);
	/// Hides sub-menus in the main.. menu...
	void HideMenus();
	/// Load shop ui for player interaction.
	void LoadShop(RuneShop * shop);
	/// Macros to access the active map via the MapManager.
	TileMap2D * ActiveMap();
	void OpenMenu();
	void CloseMenu();
	/// Zone to map!
	void Zone(String mapName);
	/// Bind camera to ze playah.-
	void TrackPlayer();
	/// Place player on ze mappur ^3^
	bool PlacePlayer(Vector3i position);
	// To look at ze player?
	void ResetCamera();
	// For when testing..!
	void ReturnToEditor();
	// Disables movement for the player!
	void DisableMovement();
	void EnableMovement();

	/// Create entity using given reference.
	void CreateEntity(RuneEntity * entityReference);

	/// For chaining event-script-thingies!
	Entity * lastModifiedEntity;

	bool menuOpen;

	/// Active shop!
	RuneShop * activeShop;
	RRPlayer * player;
	Entity * playerEntity;
	RREntityState * playerState;
	// For when testing.
	TileMap2D * mapToLoad;
	int enterMode;

	/// The cameras for the 4 primary players and spectators...!
	Camera * camera;
	Vector3i cursorPosition;
};

#endif
