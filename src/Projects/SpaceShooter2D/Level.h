/// Emil Hedemalm
/// 2015-01-21
/// Level.

#include "Ship.h"

struct ShipColorCoding 
{
	String ship;
	Vector3i color;
};

extern Camera * levelCamera;

class Level 
{
public:
	bool Load(String fromSource);
	// Used for player and camera. Based on millisecondsPerPixel.
	Vector3f BaseVelocity();
	void AddPlayer(Ship * playerShip);
	void SetupCamera();
	String source;
	/// Ships within.
	List<Ship> ships;
	/// To determine when things spawn and the duration of the entire "track".
	int millisecondsPerPixel;
	/// Music source to play.
	String music;
	/// Goal position in X.
	int goalPosition;
};
