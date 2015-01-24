/// Emil Hedemalm
/// 2015-01-24
/// Movement for AIs.

#ifndef MOVEMENT_H
#define MOVEMENT_H

class Ship;
class Entity;

#include "String/AEString.h"
#include "MathLib.h"


namespace Location {
	enum {
		VECTOR,
		UPPER_EDGE,
		LOWER_EDGE,
		CENTER,
		PLAYER,
	};
};

class Movement 
{
public:
	Movement();
	Movement(int type);
	void Nullify();

	void OnEnter(Ship * ship);
	void OnFrame(int timeInMs);

	static String Name(int type);

	enum {
		NONE, // Staying still.
		STRAIGHT,
		ZAG,
		MOVE_TO,
		MOVE_DIR,
		CIRCLE,
		UP_N_DOWN,
		TYPES
	};
	// See enum above.
	int type;
	/// See enum above.
	int location;
	/// Duration in milliseconds.
	int durationMs;
	String target; // Name, used for circling.
	Vector2f vec;
	int zagTimeMs;
	float radius;
	float distance;
	int state; // Custom, used for zag.
	bool clockwise; // Used for Circle

	/// Used for zagging, among others.
	int timeSinceLastUpdate;

	Vector3f startPos;

private:

	void MoveToLocation();
	void Circle();

	Ship * ship;
	Entity * shipEntity;
};

#endif
