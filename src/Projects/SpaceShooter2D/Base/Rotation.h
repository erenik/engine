/// Emil Hedemalm
/// 2015-01-27
/// Rotation for AIs.

#ifndef ROTATION_H
#define ROTATION_H

class Ship;
class Entity;

#include "String/AEString.h"
#include "MathLib.h"

class Rotation
{
public:
	enum {
		NONE, // o.o
		MOVE_DIR,
		ROTATE_TO_FACE,
		SPINNING,
		WEAPON_TARGET,
	};
	Rotation();
	Rotation(int type);
	void Nullify();

	String ToString();
	static String Name(int type);
	static List<Rotation> ParseFrom(String);

	void OnEnter(Ship * ship);
	void OnFrame(int timeInMs);


	// See enum above.
	int type;
	int durationMs;
	// For RotateToFace
	String target;
	float spinSpeed;

private:
	void MoveDir();	
	void RotateToFace(const Vector3f & position);
	void RotateToFaceDirection(const Vector3f & direction);
	/// Radian-direction last iteration. Used together with ships maxRadiansPerSecond and the time passed to dictate new direction.
	Angle lastFacingAngle;
	Ship * ship;
	Entity * entity;
//	Vector3f targetPosition;
};

#endif
