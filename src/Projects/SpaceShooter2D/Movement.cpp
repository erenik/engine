/// Emil Hedemalm
/// 2015-01-24
/// Movement for AIs.

#include "Movement.h"
#include "SpaceShooter2D.h"

#include "File/LogFile.h"



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
		case LOOK_AT:	return "LookAt";
	}
	return String();
}

#define SET_DIRECTION(d) 	Physics.QueueMessage(new PMSetEntity(shipEntity, PT_VELOCITY, d * ship->speed))


// Upon entering this movement pattern.
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
		case Movement::LOOK_AT:
			// Flag ship to travel in the direction it is facing, let the integrator solve all problems within.
			PhysicsMan.QueueMessage(new PMSetEntity(shipEntity, PT_VELOCITY, Vector3f()));
			PhysicsMan.QueueMessage(new PMSetEntity(shipEntity, PT_RELATIVE_VELOCITY, Vector3f(0,0,-ship->speed)));
			break;
		default:
			assert(false && "Implement. Ignore until emil implements.");
	}
}


// Called every frame.
void Movement::OnFrame(int timeInMs)
{
	switch(type)
	{
		// No updates per frame.
		case Movement::NONE:
		case Movement::STRAIGHT:
		case Movement::MOVE_DIR:
		case Movement::LOOK_AT: // No updates per-frame.
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
	
// Upon exiting this movement pattern.
void Movement::OnEnd()
{
	switch(type)
	{
		// No special update upon leaving this movement pattern.
		case Movement::NONE:
		case Movement::STRAIGHT:
		case Movement::MOVE_DIR:
		case Movement::UP_N_DOWN:
		case Movement::CIRCLE:
		case Movement::MOVE_TO:
			break;
		case Movement::LOOK_AT:
			// Flag ship to travel in the direction it is facing, let the integrator solve all problems within.
			PhysicsMan.QueueMessage(new PMSetEntity(shipEntity, PT_RELATIVE_VELOCITY, Vector3f()));
			break;
		default:
			assert(false && "Implement. Poke Emil to implement.");
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
