/// Emil Hedemalm
/// 2015-01-06
/// Our version of Tetris

#include "AppStates/AppState.h"

#define TetrisBlock TetrisBrick
class TetrisBrick 
{
public:
	/** Types correspond to the following shapes.
		I,	S,	Z,	L,	J,	O,	T

		#			#	 #
		#   ##	##	#	 #	##	#
		#  ##	 ##	##	##	## ###
		#
	*/
	enum {
		I, S, Z, L, J, O, T, MAX_TYPES
	};
	TetrisBrick();
	virtual ~TetrisBrick();
	/// Returns part locations for this brick in local space. This should then be offset by the actual position to get the correct placement.
	List<Vector2f> GetLocalPartLocations();
	List<Vector2i> GetAbsPartLocations();

	/// 0 - default, 1 - 90 degrees, 2 = 180 degrees, 3 = 270 degrees, 4 = 0
	int rotation;

	List<Entity*> parts;
	int type;
	Vector2f position;
	int millisecondsAlive;
};

class Tetris : public AppState 
{
public:
	Tetris();
	/// Virtual destructor to discard everything appropriately.
	virtual ~Tetris();

	/// Function when entering this state, providing a pointer to the previous StateMan.
	virtual void OnEnter(AppState * previousState);
	/// Main processing function, using provided time since last frame.
	virtual void Process(int timeInMs);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	virtual void OnExit(AppState * nextState);

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();
	
private:

	void CreateField();
	void SetupCamera();

	
	void EnsureBrickWithinBoundaries(TetrisBrick * brick);
	void CreateParts(TetrisBrick * forBrick);

	/// Unlinks block parts from the field, used before movements.
	void Unlink(List<Entity*> blockParts);
	/// Checks if the block can be placed in the field properly. Only Y+ may go beyond the field's limits.
	bool CanPlace(TetrisBlock * block);
	/// Places. Calls CanPlace automatically unless alreadyChecked is true.
	bool Place(TetrisBlock * block, bool alreadyChecked = false);

	/// End current game. Display stuff.
	void GameOver();

	/// Returns false if it failed due to obstruction.
	bool MoveBrick(TetrisBrick * brick, Vector2f distance);
	/// Returns false if it failed due to obstruction.
	bool RotateBrick(TetrisBrick * brick, int turnsClockwise);

	// Evaluate rows if any was completed.
	void EvaluateRows();


	int millisecondsPassed;
	Entity *** field;
	
	// All brick-parts on the field!
	List<Entity*> brickParts;

	TetrisBrick * movingBrick;
	List<Entity*> movingBrickParts;

	int rowsCompletedTotal;
	int score;
	int timeStepMs;
};

