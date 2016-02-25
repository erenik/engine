/// Emil Hedemalm
/// 2015-01-24
/// Movement for AIs.

#ifndef MOVEMENT_H
#define MOVEMENT_H

class Ship;
class Entity;

#include "String/AEString.h"
#include "MathLib.h"
#include "MathLib/Variable.h"

namespace Location {
	enum {
		VECTOR,
		LEFT_EDGE,
		RIGHT_EDGE,
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
	/// For scripting.
	static List<Variable> GetTypesAsVariables();
	// Upon entering this movement pattern.
	void OnEnter(Ship * ship);
	/// Called on scripted updates or otherwise when adjusted.
	void OnSpeedUpdated();
	// Called every frame.
	void OnFrame(int timeInMs);
	// Upon exiting this movement pattern.
	void OnEnd();

	String ToString();

	static List<Movement> ParseFrom(String);
	static String Name(int type);

	/// Adds level speed and sets ship speed accordingly.
	void SetWindowSpeed(Vector2f desiredAppearedSpeed);
	enum {
		NONE = -1, // Staying still.
		STRAIGHT, // Plain X-
		ZAG,
		MOVE_TO,
		MOVE_DIR,
		CIRCLE,
		UP_N_DOWN,
		LOOK_AT, // Follows entity's current forward-direction.
		TYPES
	};
	// See enum above.
	int type;
	/// See enum above.
	int location;
	/// Duration in milliseconds.
	int durationMs;
	/// Reset when entering it!
	int timeInCurrentMovement;
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
	/// Sets movement speed to target normalized direction.
	void SetDirection(Vector2f dir);
	void MoveToLocation();
	void Circle();

	Ship * ship;
	Entity * shipEntity;
};

#endif
