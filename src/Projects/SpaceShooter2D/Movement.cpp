/// Emil Hedemalm
/// 2015-01-24
/// Movement for AIs.

#include "Movement.h"
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
			return;
		case Rotation::MOVE_DIR:
			MoveDir();
			break;
		case Rotation::SPINNING:
			PhysicsMan.QueueMessage(new PMSetEntity(entity, PT_ROTATIONAL_VELOCITY, Vector3f(0,spinSpeed,0)));
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
				targetEntity = spaceShooter->playerShip.entity;
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
	float radians = GetAngler(dir[0], dir[1]);
	// Default is pointing right up, meaning PI/2, so deduct it to get it correct?
	float radiansToSet = radians - PI/2;
	// Rotate to it.
	Vector3f rot(PI/2,radiansToSet,0);
	PhysicsMan.QueueMessage(new PMSetEntity(entity, PT_SET_ROTATION, rot));
}	

void Rotation::RotateToFace(const Vector3f & position)
{
	Vector3f toTarget = position - entity->position;
	// Grab angle of current movement dir.
	RotateToFaceDirection(toTarget);
}

void Rotation::RotateToFaceDirection(const Vector3f & direction)
{
	Vector3f dir = direction.NormalizedCopy();
	float radians = GetAngler(dir[0], dir[1]);
	// Default is pointing right up, meaning PI/2, so deduct it to get it correct?
	float radiansToSet = radians - PI/2;
	// Rotate to it.
	Vector3f rot(PI/2,radiansToSet,0);
	PhysicsMan.QueueMessage(new PMSetEntity(entity, PT_SET_ROTATION, rot));		
}



Movement::Movement()
{
	Nullify();
}

Movement::Movement(int withType)
{	
	Nullify();
	type = withType;
}

void Movement::Nullify()
{
	type = Movement::STRAIGHT;
	durationMs = -1;
	zagTimeMs = 0;
	radius = 1.f;
	distance = 1.f;
	state = 0;
	timeSinceLastUpdate = 0;
	ship = NULL;
}


String Movement::Name(int type)
{
	switch(type)
	{
		case NONE: return "None";
		case STRAIGHT: return "Straight";
		case ZAG: return "Zag";
		case MOVE_TO: return "MoveTo";
		case MOVE_DIR: return "MoveDir";
		case CIRCLE: return "Circle";
		case UP_N_DOWN: return "Up-and-Down";
	}
	return String();
}

#define SET_DIRECTION(d) 	Physics.QueueMessage(new PMSetEntity(shipEntity, PT_VELOCITY, d * ship->speed))


void Movement::OnEnter(Ship * ship)
{
	// Reset stuff.
	state = 0;
	timeSinceLastUpdate = 0;

	// Grab pointers.
	this->ship = ship;
	shipEntity = ship->entity;
	switch(type)
	{
		case Movement::NONE:
			SET_DIRECTION(Vector3f());
			break;
		case Movement::STRAIGHT:
			// Begin forward!
			SET_DIRECTION(Vector3f(-1,0,0));
			break;
		case Movement::ZAG:
		{
			// Begin zag.
			SET_DIRECTION(vec);
			break;
		}
		case Movement::MOVE_TO:
			MoveToLocation();
			break;
		case Movement::MOVE_DIR:
			SET_DIRECTION(vec);
			break;
		case Movement::CIRCLE:
		{
			Circle();
			break;
		}
		case Movement::UP_N_DOWN:
			// Start up.
			SET_DIRECTION(Vector3f(0,1,0));
			startPos = shipEntity->position;
			break;
		default:
			assert(false && "Implement. Ignore until emil implements.");
	}
}


void Movement::OnFrame(int timeInMs)
{
	switch(type)
	{
		// No updates per frame.
		case Movement::NONE:
		case Movement::STRAIGHT:
		case Movement::MOVE_DIR:
			break;
		case Movement::ZAG:
		{
			timeSinceLastUpdate += timeInMs;
			if (timeSinceLastUpdate > zagTimeMs)
			{
				// Change direction.
				state = (state + 1) % 2;
				Vector3f dir = vec;
				if (state == 1)
					dir[1] *= -1;
				SET_DIRECTION(dir);
				timeSinceLastUpdate = 0;
			}
			break;
		}
		case Movement::MOVE_TO:
			MoveToLocation();
			break;
		case Movement::CIRCLE:
			Circle();
			break;
		case Movement::UP_N_DOWN:
		{
			if (shipEntity->position[1] > startPos[1] + distance)
				SET_DIRECTION(Vector3f(0,-1,0));
			else if (shipEntity->position[1] < startPos[1] - distance)
				SET_DIRECTION(Vector3f(0,1,0));
			break;
		}
		default:
			assert(false && "Implement. Ignore until emil implements.");
	}
}


void Movement::MoveToLocation()
{
	Vector3f pos;
	bool isPosition = false;
	/// Check if we reached our destination?
	switch(location)
	{
		case Location::VECTOR:
		{
			// Adjust movement vector?
			pos = spaceShooter->levelEntity->position + vec;
			isPosition = true;
			break;
		}
		case Location::UPPER_EDGE:
		{
			if (state == 0)
			{
				SET_DIRECTION(Vector3f(0,1,0));
				++state;
			}
			else if (shipEntity->position[1] > 20.f && state == 1)
			{
				PhysicsMan.QueueMessage(new PMSetEntity(shipEntity, PT_VELOCITY, Vector3f()));
				++state;
			}
			break;
		}
		case Location::LOWER_EDGE:
		{
			if (state == 0)
			{
				SET_DIRECTION(Vector3f(0,-1,0));
				++state;
			}
			if (shipEntity->position[1] < 0.f && state == 1)
			{
				PhysicsMan.QueueMessage(new PMSetEntity(shipEntity, PT_VELOCITY, Vector3f()));
				++state;
			}
			break;
		}
		case Location::CENTER: 
			pos = spaceShooter->levelEntity->position;
			isPosition = true;
			break;
		case Location::PLAYER:
			if (spaceShooter->playerShip.entity)
			{
				pos = spaceShooter->playerShip.entity->position;
				isPosition = true;
			}
			else {
				SET_DIRECTION(Vector3f());
				return;
			}
			break;
	}
	if (isPosition)
	{
		Vector3f toPos = pos - shipEntity->position;
		toPos.Normalize();
		SET_DIRECTION(toPos);
	}
}


void Movement::Circle()
{
	// Go towards target.
	Entity * target = spaceShooter->playerShip.entity;
	if (!target)
	{
		SET_DIRECTION(Vector3f());
		return;
	}
	Vector3f pos = target->position;
	Vector3f targetToShip = shipEntity->position - pos;
	targetToShip.Normalize();
	// Calculate our current angle in relation to the target.
	float radians = GetAngler(targetToShip[0], targetToShip[1]);
	// Start circling it straight away? Always trying to go clockwise or counter-clockwise?
	// Select spot a bit along the radian path.
	radians += clockwise? -0.10f : 0.10f;
	Vector2f targetDir(cos(radians), sin(radians));
	Vector3f position = target->position + targetDir * radius;
	Vector3f toPosForreal = position - shipEntity->position;
	toPosForreal.Normalize();
	SET_DIRECTION(toPosForreal);
}
