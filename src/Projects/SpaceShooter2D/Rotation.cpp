/// Emil Hedemalm
/// 2015-01-27
/// Rotation for AIs.

#include "Rotation.h"

#include "SpaceShooter2D.h"
#include "File/LogFile.h"


Rotation::Rotation()
{
	Nullify();
}
Rotation::Rotation(int rotType)
{
	Nullify();
	this->type = rotType;
}

void Rotation::Nullify()
{
	durationMs = -1;
	type = -1;
	spinSpeed = 0.5f;
}


String Rotation::Name(int type)
{
	switch(type)
	{
		case Rotation::NONE: return "None";
		case Rotation::MOVE_DIR: return "MoveDir";
		case Rotation::ROTATE_TO_FACE: return "RotationTo";
		case Rotation::SPINNING: return "Spinning";
		default:
			assert(false);
	}
	return String();
}

void Rotation::OnEnter(Ship * forShip)
{
	this->ship = forShip;
	this->entity = ship->entity;
	switch(type)
	{
		case Rotation::NONE:		
		case Rotation::ROTATE_TO_FACE: // Nothing first frame.
		case Rotation::WEAPON_TARGET:
		{
			// Fetch current radians.
			Vector3f lookAt = entity->rotationMatrix * Vector4f(0,0,-1,0);
			/// o.o
			lastFacingAngle = Angle(lookAt);
			// Default is pointing right up, meaning PI/2, so deduct it to get it correct?
			lastFacingAngle -= Angle(PI/2);
			break;
		}
		case Rotation::MOVE_DIR:
			MoveDir();
			break;
		case Rotation::SPINNING:
			QueuePhysics(new PMSetEntity(entity, PT_ROTATIONAL_VELOCITY, Vector3f(0,spinSpeed,0)));
			break;
	}
}

void Rotation::OnFrame(int timeInMs)
{
	Entity * targetEntity = NULL;
	switch(type)
	{
		case Rotation::NONE:
			return;
		case Rotation::MOVE_DIR:
			MoveDir();
			break;
		case Rotation::ROTATE_TO_FACE:
		{
			target.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (target == "Player")
			{
				targetEntity = playerShip.entity;
				if (targetEntity)
					RotateToFace(targetEntity->position);
			}
			else
				return;
			break;
		}
		case Rotation::WEAPON_TARGET:
			RotateToFaceDirection(ship->WeaponTargetDir());
			break;
	}
}

void Rotation::MoveDir()
{
	if (!entity->physics)
	{
		// Log it.
		LogToFile("rotationErrors", "Entity "+entity->name+" lacking physics property, thus probably stationary, why does it have a Rotation pattern then?", WARNING);
		return;
	}
	Vector3f dir = entity->Velocity();
	// Grab angle of current movement dir.
	dir.Normalize();
	Angle angleToLook(dir);
	// Default is pointing right up, meaning PI/2, so deduct it to get it correct?
	Angle correctedAngle = angleToLook - Angle(PI/2);
	// Rotate to it.
	Vector3f rot(PI/2,correctedAngle.Radians(),0);
	QueuePhysics(new PMSetEntity(entity, PT_SET_ROTATION, rot));
}	

void Rotation::RotateToFace(const Vector3f & position)
{
	Vector3f toTarget = position - entity->position;
	// Grab angle of current movement dir.
	RotateToFaceDirection(toTarget);
}

void Rotation::RotateToFaceDirection(const Vector3f & direction)
{
	// Current dir?
	Vector3f lookAt = ship->entity->rotationMatrix * Vector3f(0,0,-1);
	Angle lookAtAngle(lookAt);
	Vector3f dir = direction.NormalizedCopy();
	Angle targetDir(dir);
	Angle diff = targetDir - lookAtAngle;
	float radiansDiff = diff.Radians();
	float radialVel = radiansDiff * 10.f;
	float minMax = ship->maxRadiansPerSecond;
	ClampFloat(radialVel, -minMax, minMax);
	// Set rotational velocity, yo. lol.
	Vector3f rot(0,radialVel,0);
	QueuePhysics(new PMSetEntity(entity, PT_ROTATIONAL_VELOCITY, rot));		
}


