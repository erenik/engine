/// Emil Hedemalm
/// 2015-01-24
/// Movement for AIs.

#include "Movement.h"
#include "SpaceShooter2D/SpaceShooter2D.h"

#include "File/LogFile.h"

String Movement::ToString()
{
	return Name(type);
}

List<Movement> Movement::ParseFrom(String fromString)
{	
	List<Movement> movements;
	/// Tokenize.
	List<String> parts = TokenizeIgnore(fromString, ",", "()");
	for (int i = 0; i < parts.Size(); ++i)
	{
		String part = parts[i];
		List<String> broken = part.Tokenize("()");
		if (broken.Size() == 0)
		{
			LogMain("Empty movement pattern when parsing string '"+fromString+"'.", INFO);
			continue;
		}
		String partPreParenthesis = broken[0];
		partPreParenthesis.RemoveSurroundingWhitespaces();
		Movement move;
		move.type = -1;
		for (int j = 0; j < Movement::TYPES; ++j)
		{
			String name = Movement::Name(j);
			if (name == partPreParenthesis)
			{
				move.type = j;
				break;
			}
			if (name == "n/a")
				move.type = Movement::NONE;
		}
		if (move.type == -1)
		{
			String typesString;
			for (int j = 0; j < Movement::TYPES; ++j)
			{
				typesString += "\n\t"+Movement::Name(j);
			}
			LogMain("Unrecognized movement pattern \'"+partPreParenthesis+"\' when parsing ship \'"+fromString+"\'. Available types are as follows: "+typesString, INFO);
			continue;
		}
		// Add it and continue.
		if (move.type == Movement::NONE)
		{
			movements.Add(move);
			continue;
		}
		// Demand arguments depending on type?
		if (broken.Size() < 2)
		{
			LogMain("Lacking arguments for movement pattern \'"+partPreParenthesis+"\' when parsing data for ship \'"+fromString+"\'.", INFO);
			continue;
		}
		List<String> args = broken[1].Tokenize(",");
#define DEMAND_ARGS(a) if (args.Size() < a){ \
	LogMain("Expected "+String(a)+" arguments for movement type \'"+\
	Movement::Name(move.type)+"\', encountered "+String(args.Size())+".", INFO); \
	continue;}
#define GET_DURATION(a) if (args[a] == "inf") move.durationMs = -1; else move.durationMs = args[a].ParseInt();
		switch(move.type)
		{
			case Movement::STRAIGHT:
				DEMAND_ARGS(1);
				GET_DURATION(0);
				break;	
			case Movement::ZAG:
				DEMAND_ARGS(3);
				move.vec.ParseFrom(args[0]);
				move.zagTimeMs = args[1].ParseInt();
				GET_DURATION(2);
				break;
			case Movement::MOVE_TO:
			{
				DEMAND_ARGS(2);
				// Optionally parse some string for where to go.
				String arg = args[0];
				arg.RemoveSurroundingWhitespaces();
				arg.SetComparisonMode(String::NOT_CASE_SENSITIVE);
				if (arg == "upper edge")
				{
					move.location = Location::UPPER_EDGE; 	
				}
				else if (arg == "lower edge")
				{
					move.location = Location::LOWER_EDGE;
				}
				else if (arg == "center")
				{
					move.location = Location::CENTER;
				}
				else if (arg == "player")
				{
					move.location = Location::PLAYER;
				}
				else 
				{
					move.vec.ParseFrom(args[0]);
					move.location = Location::VECTOR;
				}
				GET_DURATION(1);
				break;
			}
			case Movement::MOVE_DIR:
			{
				DEMAND_ARGS(2);
				move.vec.ParseFrom(args[0]);
				GET_DURATION(1);
				break;
			}
			case Movement::CIRCLE:
			{
				DEMAND_ARGS(4);
				move.target = args[0];
				move.radius = args[1].ParseFloat();
				move.clockwise = args[2].ParseBool();
				GET_DURATION(3);
				break;
			}
			case Movement::UP_N_DOWN:
			{
				DEMAND_ARGS(2);
				move.distance = args[0].ParseFloat();
				GET_DURATION(1);
				break;
			}
		}
		move.vec.Normalize();
		movements.Add(move);
	}
	return movements;
};

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
	timeInCurrentMovement = 0;
	type = Movement::STRAIGHT;
	durationMs = -1;
	zagTimeMs = 0;
	radius = 1.f;
	distance = 1.f;
	state = 0;
	timeSinceLastUpdate = 0;
	ship = NULL;
}

List<Variable> Movement::GetTypesAsVariables()
{
	List<Variable> var;
	for (int i = 0; i < TYPES; ++i)
	{
		var.AddItem(Variable(Name(i), i));
	}
	return var;
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

// Upon entering this movement pattern.
void Movement::OnEnter(Ship * ship)
{
	timeInCurrentMovement = 0;
	// Reset stuff.
	state = 0;
	timeSinceLastUpdate = 0;

	// Grab pointers.
	this->ship = ship;
	shipEntity = ship->entity;
	switch(type)
	{
		case Movement::NONE:
			SetDirection(Vector3f());
			break;
		case Movement::STRAIGHT:
			// Begin forward!
			SetDirection(Vector2f(-1,0));
			break;
		case Movement::ZAG:
		{
			// Begin zag.
			SetDirection(vec);
			break;
		}
		case Movement::MOVE_TO:
			MoveToLocation();
			break;
		case Movement::MOVE_DIR:
			SetDirection(vec);
			break;
		case Movement::CIRCLE:
		{
			Circle();
			break;
		}
		case Movement::UP_N_DOWN:
			// Start up.
			SetDirection(Vector3f(0,1,0));
			startPos = shipEntity->worldPosition;
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

/// Called on scripted updates or otherwise when adjusted.
void Movement::OnSpeedUpdated()
{
	// Do stuff..
	// Just call Start for now?
	if (ship)
		OnEnter(ship);
	else
		assert(false);
}

// Called every frame.
void Movement::OnFrame(int timeInMs)
{
	timeInCurrentMovement += timeInMs;
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
				SetDirection(dir);
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
			if (shipEntity->worldPosition[1] > startPos[1] + distance)
				SetDirection(Vector3f(0,-1,0));
			else if (shipEntity->worldPosition[1] < startPos[1] - distance)
				SetDirection(Vector3f(0,1,0));
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

/// Sets movement speed to target normalized direction.
void Movement::SetDirection(Vector2f dir)
{
	// #define SET_DIRECTION(d) 	
	Vector2f speed = dir * ship->speed;
	SetWindowSpeed(speed);
}

void Movement::SetWindowSpeed(Vector2f desiredAppearedSpeed)
{
	Vector2f totalSpeed = desiredAppearedSpeed;
	if (totalSpeed.LengthSquared())
		totalSpeed += spaceShooter->level.BaseVelocity();
	QueuePhysics(new PMSetEntity(shipEntity, PT_VELOCITY, totalSpeed));
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
			pos = levelEntity->worldPosition + vec;
			isPosition = true;
			break;
		}
		case Location::LEFT_EDGE:
		{
			if (state == 0)
			{
				SetDirection(Vector3f(-1,0,0));
				++state;
			}
			else if (shipEntity->worldPosition.x < leftEdge && state == 1)
			{
				SetDirection(Vector2f());
				++state;
			}
			break;
		}
		case Location::RIGHT_EDGE:
			if (state == 0)
			{
				SetDirection(Vector3f(1,0,0));
				++state;
			}
			else if (shipEntity->worldPosition.x > rightEdge && state == 1)
			{
				SetDirection(Vector2f());
				++state;
			}
			break;
		case Location::UPPER_EDGE:
		{
			if (state == 0)
			{
				SetDirection(Vector3f(0,1,0));
				++state;
			}
			else if (shipEntity->worldPosition[1] > 20.f && state == 1)
			{
				SetDirection(Vector2f());
				++state;
			}
			break;
		}
		case Location::LOWER_EDGE:
		{
			if (state == 0)
			{
				SetDirection(Vector3f(0,-1,0));
				++state;
			}
			if (shipEntity->worldPosition[1] < 0.f && state == 1)
			{
				SetDirection(Vector2f());
				++state;
			}
			break;
		}
		case Location::CENTER: 
			pos = levelEntity->worldPosition;
			isPosition = true;
			break;
		case Location::PLAYER:
			if (playerShip->entity)
			{
				pos = playerShip->entity->worldPosition;
				isPosition = true;
			}
			else {
				SetDirection(Vector3f());
				return;
			}
			break;
		default:
			assert(false && "Movement unidentified");
	}
	if (isPosition)
	{
		Vector3f toPos = pos - shipEntity->worldPosition;
		if (toPos.LengthSquared() > 1)
			toPos.Normalize();
		SetDirection(toPos);
	}
}


void Movement::Circle()
{
	// Go towards target.
	Entity * target = playerShip->entity;
	if (!target)
	{
		SetDirection(Vector3f());
		return;
	}
	Vector3f pos = target->worldPosition;
	Vector3f targetToShip = shipEntity->worldPosition - pos;
	targetToShip.Normalize();
	// Calculate our current angle in relation to the target.
	float radians = GetAngler(targetToShip[0], targetToShip[1]);
	// Start circling it straight away? Always trying to go clockwise or counter-clockwise?
	// Select spot a bit along the radian path.
	radians += clockwise? -0.10f : 0.10f;
	Vector2f targetDir(cos(radians), sin(radians));
	Vector3f position = target->worldPosition + targetDir * radius;
	Vector3f toPosForreal = position - shipEntity->worldPosition;
	toPosForreal.Normalize();
	SetDirection(toPosForreal);
}
