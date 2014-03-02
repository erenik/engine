// Emil Hedemalm
// 2013-06-28

#include "GameStates/GameState.h"
#include "GameStates/States.h"
#include "Selection.h"
#include "Graphics/Camera/Camera.h"

struct Waypoint;
struct Path;
class Ship;
class Map;

class StreamerTestState : public GameState{
public:
	StreamerTestState();
	/// Virtual destructor to discard everything appropriately.
	virtual ~StreamerTestState();

	enum editModes {
		NULL_MODE,
		ENTITIES,
		NAVMESH,
		SHIP_EDITOR,
		PATHS
	};

	void OnEnter(GameState * previousState);
	void Process(float time);
	void OnExit(GameState * nextState);
	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

	///
	virtual Selection GetActiveSelection();

	/// Called every time the current selection is updated.
	void OnSelectionUpdated();

	/// Input functions for the various states
	void MouseClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	void MouseRightClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	virtual void MouseMove(float x, float y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL);
	void MouseWheel(float delta);
	void CreateDefaultBindings();
	void CreateUserInterface();
	void InputProcessor(int action, int inputDevice = 0);

	/// And update UI and stuff!
	void SetMode(int mode);

	/// Handle drag-n-drop files.
	virtual void HandleDADFiles(List<String> & files);

private:

    /// Playing with forces.
    float forceMagnitude;
	bool multiplyForceByMass;

	/// Debug shit
	bool renderClickRays;

	void ReturnToLastActiveMap();

	int editMode;
	/// For managing path
	Path * activePath;

	/// For creating le checkpoints
	int checkPointWaypointInterval;
	float checkpointSize;
	String checkpointModelSource; // For custom models.

	/// For waypoint-pathy-pathing and updating what iz rendurrred o-o;
	void OnActivePathUpdated();
	/// Update what's rendered and stuff in UI too maybe?
	void OnWaypointSelectionUpdated();

	/// Attempts to generate a track path using only the selected waypoints and current navMesh as help.
	void GenerateTrackPath();
	/// Create checkpoints using the given track path!
	void CreateCheckpoints();
	/// Connects all selected waypoints :3
	void ConnectWaypoints();

	// Loads target map o-o
	void LoadMap(String fromFile);
	void TranslateActiveEntities(Vector3f distance);
	void SetScaleActiveEntities(Vector3f scale);
	void ScaleActiveEntities(Vector3f scale);
	void RotateActiveEntities(Vector3f rotation);

	// Ship Editor
	void NewShip();
	void SaveShip(String toFile);
	void LoadShip(String fromFile);

	/// Update UI with the new stats and names of the ship.
	void OnActiveShipUpdated();

	/// The editor camera! :D
	Camera editorCamera;
	/// Actively manipulated entities
	Selection editorSelection;
	List<Waypoint*> waypointSelection;

	Entity * activeShipEntity;
	Ship * activeShip;
	// To remember which map we were editing when switching state.s
	Map * lastActiveMap;

	/// Mouse states
	bool lButtonDown;
	bool rButtonDown;
	/// Starting positions for when dragging mouse for selection, etc.
	float startMouseX, startMouseY;
	float mouseX, mouseY;

	enum mouseCameraStates {
		NULL_STATE,
		ROTATING,
		PANNING
	};
	int mouseCameraState;
};

