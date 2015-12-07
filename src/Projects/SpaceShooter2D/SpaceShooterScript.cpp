/// Emil Hedemalm
/// 2015-12-07
/// Space-Shooter specific scripting, containing bindings between classes here and functions in the script.

#include "SpaceShooterScript.h"
#include "SpaceShooter2D.h"
#include "Entity/Entity.h"
#include "Base/Ship.h"

SpaceShooterScript::SpaceShooterScript()
{
	sssState = 0;
}

void SpaceShooterScript::EvaluateLine(String & line)
{
	/// Default line processed once?
	lineProcessed = true;
	if (line.StartsWith("Charge"))
	{
		lineProcessed = false; // Repeat this until we're done.
		List<String> args = line.Tokenize(",()");
		// Do charge until done.
		String target = args[1];
		Entity * targetEntity = 0;
		if (target == "self")
			targetEntity = this->entity;
		lineFinished = false;
		if (targetEntity == 0)
			return;
		
		/// Update speed based on progression.
		float distance = (targetEntity->worldPosition.x - (levelEntity->worldPosition.x - playingFieldHalfSize.x)) / playingFieldSize.x;
	//	std::cout<<"\nDistance "<<distance;
		ClampFloat(distance, -1.f, 1.f);
		Ship * ship = spaceShooter->GetShip(targetEntity);
		int minSpeed = args[2].ParseFloat(), maxSpeed = args[3].ParseFloat();
		ship->speed = minSpeed + (1 - distance) * (maxSpeed - minSpeed);
		/// Set initial speed.
		if (sssState != 1)
		{
			/// Enter charging movement if not already there.
			Movement move(Movement::MOVE_TO);
			move.location = Location::LEFT_EDGE; 
			ship->SetMovement(move);
			sssState = 1;
		}
		/// Update speed.
		else
			ship->OnSpeedUpdated();
		
		// Mark as complete when we are at the left side.
		if (distance < 0)
		{
			lineProcessed = true;
			lineFinished = true;
			sssState = 0;
			ship->SetSpeed(0);
		}
	}
	else if (line.StartsWith("Return"))
	{
		lineProcessed = false; // Repeat this until we're done.
		List<String> args = line.Tokenize(",()");
		// Do charge until done.
		String target = args[1];
		Entity * targetEntity = 0;
		if (target == "self")
			targetEntity = this->entity;
		lineFinished = false;
		if (targetEntity == 0)
			return;
		
		/// Update speed based on progression.
		float distance = ((levelEntity->worldPosition.x + playingFieldHalfSize.x) - targetEntity->worldPosition.x) / playingFieldSize.x;
	//	std::cout<<"\nDistance "<<distance;
//		ClampFloat(distance, -1.f, 1.f);
		Ship * ship = spaceShooter->GetShip(targetEntity);
		/// Set initial speed.
		if (sssState != 1)
		{
			ship->speed = args[2].ParseFloat();
			/// Enter charging movement if not already there.
			Movement move(Movement::MOVE_TO);
			move.location = Location::RIGHT_EDGE; 
			ship->SetMovement(move);
			sssState = 1;
		}		
		// Mark as complete when we are at the left side.
		if (distance < 0)
		{
			lineProcessed = true;
			lineFinished = true;
			sssState = 0;
			ship->SetSpeed(0);
		}
	}
	else if (line.StartsWith("SetMovementPattern"))
	{
		String loin = line - "SetMovementPattern");
		loin = loin.Part(1, loin.Length() - 1);
		/// FUCK YEAH
		List<String> args = line.Tokenize(",");
		String target = args[1];
		String pattern = args[2];
		String dir = args[3]; // e.g. x y 0 Rand(-1,1) Dir will be normalized
//		Entity * target = ;
	}
	else if (line.StartsWith("DisableWeapon"))
	{
		List<String> args = line.Tokenize(",()");
		// Do charge until done.
		String target = args[1];
		Entity * targetEntity = 0;
		if (target == "self")
			targetEntity = this->entity;
		if (targetEntity == 0)
			return;		
		String weaponName = args[2];
		Ship * ship = spaceShooter->GetShip(targetEntity);
		ship->DisableWeapon(weaponName);
		lineProcessed = lineFinished = true;
	}
	else 
	{
		Script::EvaluateLine(line);
	}
}
void SpaceShooterScript::EvaluateFunction(String function, List<String> arguments)
{

}

SpaceShooterEvaluator spaceShooterEvaluator;
bool SpaceShooterEvaluator::EvaluateFunction(String byName, List<String> arguments, ExpressionResult & result)
{
	String name = byName;
	if (name == "ArrivedAtDestination")
	{
		// Get ship by ID
		int id = arguments[0].ParseInt();
		Ship * ship = spaceShooter->GetShipByID(id);
		result = ExpressionResult::Boolean(ship->ArrivedAtDestination());
		return true;
	}
	else if (name == "TimeInCurrentMovement")
	{
		int id = arguments[0].ParseInt();
		Ship * ship = spaceShooter->GetShipByID(id);	
		result = ExpressionResult::Integral(ship->movementPatterns[ship->currentMovement].timeInCurrentMovement);
		return true;
	}
//	exp.text = "Unable to find function";
	return false;
}

bool SpaceShooterEvaluator::IsFunction(String name)
{
	if (name == "ArrivedAtDestination" || name == "TimeInCurrentMovement")
		return true;
	return false;
}


