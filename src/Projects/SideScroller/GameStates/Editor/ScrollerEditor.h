// Emil Hedemalm
// 2013-06-28

#include "Game/GameType.h"

#include "Maps/Map.h"
#include "../SideScrollerGameState.h"
#include "GameStates/GameStates.h"
#include "Selection.h"

class GridObject;
class GridObjectType;
class Camera;
class TileMap2D;
class Event;
struct TileType;
struct GraphicsState;

namespace EditMode {
enum editModes{
	NULL_MODE,
	SIZE, 
	TILES, 
	TERRAIN, 
	OBJECTS, 
	EVENTS,
	LIGHTING,
	EDIT_MODES,
};};
	

class ScrollerEditor : public SideScrollerGameState {
public:
	ScrollerEditor();
	/// Virtual destructor to discard everything appropriately.
	virtual ~ScrollerEditor();
	void OnEnter(GameState * previousState);
	void Process(float time);
	void OnExit(GameState * nextState);

	/// For key-bindings.
	void CreateDefaultBindings();
	/// For key-bindings.
	void InputProcessor(int action, int inputDevice = 0);


	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

	///
	virtual Selection GetActiveSelection() { return editorSelection; };

	/// Called every time the current selection is updated.
	void OnSelectionUpdated();

	/// Input functions for the various states
	void MouseClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	void MouseRightClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	void MouseMove(float x, float y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL);
	void MouseWheel(float delta);
	/// Callback from the Input-manager, query it for additional information as needed.
	void KeyPressed(int keyCode, bool downBefore);

	void CreateUserInterface();

	void Render(GraphicsState & graphicsState);

private:

	// Used to keep track of saving for when query-dialogues are necessary.
	String mapFilePath;
	String rootMapDir;
	/// Saves the map to set mapFilePath. Assumes any file-checks have been done beforehand. Pauses input and physics while saving.
	bool SaveMap();
	/// Attempts to load target map
	bool LoadMap(String name);

	void SetMapSize(int x, int y);

	/// Active map.
	Map * map;
	Event * selectedEvent;
	
	/// In 3D-world
	Vector3f cursorPosition, previousCursorPosition;
	/// Float values for painting smoothly!
	Vector3f cursorPositionPreRounding;

	/// Transitions to the level test state!
	void Playtest();
	/// Attempts to create an object on current mouse location. Returns object if it succeeded.
	GridObject * CreateObject();
	/// SElect evvveettttnt, using given mouse coordinates. Returns false if none was close enough.
	bool SelectEvent();
	/// Create an event on the map! :)
	void CreateEvent();
	/// Attempts to delete an event at given location!
	void DeleteEvent();


	void TranslateActiveEntities(Vector3f distance);
	void SetScaleActiveEntities(Vector3f scale);
	void ScaleActiveEntities(Vector3f scale);
	void RotateActiveEntities(Vector3f rotation);


	/// Set defaults!
	void ResetCamera();
	/// The ScrollerEditor camera! :D
	Camera * scrollerEditorCamera;
	/// Actively manipulated entities
	Selection editorSelection;

	/// Mouse states
	bool lButtonDown;
	bool rButtonDown;
	/// Starting positions for when dragging mouse for selection, etc.
	float startMouseX, startMouseY;
	float mouseX, mouseY;
	/// Used for moving entities in-game.
	Vector3f moveStartPosition;

	enum mouseCameraStates {
		NULL_STATE,
		ROTATING,
		PANNING
	};
	int mouseCameraState;
};
