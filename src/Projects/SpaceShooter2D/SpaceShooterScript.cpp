/// Emil Hedemalm
/// 2015-12-07
/// Space-Shooter specific scripting, containing bindings between classes here and functions in the script.

#include "SpaceShooterScript.h"
#include "SpaceShooter2D.h"
#include "Entity/Entity.h"
#include "Base/Ship.h"
#include "File/LogFile.h"
#include "Game/GameVariableManager.h"
#include "Game/GameVariable.h"

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
		float targetX = playingFieldHalfSize.x - 5.f;
		float distance = ((levelEntity->worldPosition.x + targetX) - targetEntity->worldPosition.x) / playingFieldSize.x;
	//	std::cout<<"\nDistance "<<distance;
//		ClampFloat(distance, -1.f, 1.f);
		Ship * ship = spaceShooter->GetShip(targetEntity);
		/// Set initial speed.
		if (sssState != 1)
		{
			ship->speed = args[2].ParseFloat();
			/// Enter charging movement if not already there.
			Movement move(Movement::MOVE_TO);
			move.location = Location::VECTOR;
			move.vec = Vector2f(targetX, 0); // Compared to center 0,0 of screen.
			ship->SetMovement(move);
			sssState = 1;
		}		
		// Mark as complete when we are at the left side.
		if (distance < 0.05f)
		{
			lineProcessed = true;
			lineFinished = true;
			sssState = 0;
			ship->SetSpeed(0);
		}
	}
	else if (line.StartsWith("SetSpeed"))
	{
		/// FUCK YEAH
		List<String> args = line.Tokenize(",()");
		String target = args[1]; target.RemoveSurroundingWhitespaces();
		int id = target.ParseInt();
		Ship * ship = spaceShooter->GetShipByID(id);
		if (target == "self")
			ship = spaceShooter->GetShip(this->entity);
		float speed = args[2].ParseFloat();
		ship->SetSpeed(speed);
		lineFinished = true;
	}
/*	else if (line.StartsWith("DisableWeapon"))
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
	*/
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
	#define GRAB_SHIP int id = arguments[0].ParseInt();\
		Ship * ship = spaceShooter->GetShipByID(id);\
		if (!ship){\
			result = ExpressionResult::Integral(0);\
			result.text = "Unable to find ship with id "+String(id);\
			return true;\
		if (ship->entity == 0)\
			return false;\
		}
	if (name == "ArrivedAtDestination")
	{
		GRAB_SHIP
		result = ExpressionResult::Boolean(ship->ArrivedAtDestination());
		return true;
	}
	else if (name == "TimeInCurrentMovement")
	{
		GRAB_SHIP
		result = ExpressionResult::Integral(ship->movements[ship->currentMovement].timeInCurrentMovement);
		return true;
	}
	else if (name == "PositionY")
	{
		GRAB_SHIP
		if (ship->entity == 0)
			return false;
		if (!ship)
			result = ExpressionResult::Error("Bad ship id");
		else
			result = ExpressionResult::Integral(ship->entity->worldPosition.y - levelEntity->worldPosition.y);
		return true;
	}
	else if (name == "PositionX")
	{
		GRAB_SHIP
		if (!ship)
			result = ExpressionResult::Error("Bad ship id");
		else
			result = ExpressionResult::Integral(ship->entity->worldPosition.x - levelEntity->worldPosition.x);
		return true;
	}
	else if (name == "DisableWeapon")
	{
		GRAB_SHIP
		ship->DisableWeaponsByID(arguments[1].ParseInt());
		result = ExpressionResult::Boolean(true);
		return true;
	}
	else if (name == "DisableAllWeapons")
	{
		GRAB_SHIP
		ship->DisableAllWeapons();
		result = ExpressionResult::Boolean(true);
		return true;
	}
	else if (name == "EnableWeapon")
	{
		GRAB_SHIP
		ship->EnableWeaponsByID(arguments[1].ParseInt());
		result = ExpressionResult::Boolean(true);
		return true;
	}
	else if (name == "SetMovementPattern")
	{
		GRAB_SHIP
		int pattern = arguments[1].ParseInt();
		Movement move(pattern);
		move.vec = Vector2f(arguments[2].ParseFloat(), arguments[3].ParseFloat());
		move.vec.Normalize();
		ship->SetMovement(move);
		result = ExpressionResult::Boolean(true);
		return true;
	}
	else if (name == "HPPercent")
	{
		GRAB_SHIP
		float percent = ship->hp / (float)ship->maxHP * 100;
//		std::cout<<"\nperc "<<percent;
		result = ExpressionResult::Integral(percent);
		return true;	
	}
	else if (name == "PartsDestroyed")
	{
		GRAB_SHIP
		result = ExpressionResult::Integral(ship->childrenDestroyed);
		return true;
	}
	else if (name == "SetProjectileSpeedBonus")
	{
		GRAB_SHIP;
		ship->SetProjectileSpeedBonus(arguments[1].ParseFloat()); 
		result = ExpressionResult::Boolean(true);
		return true;
	}
	else if (name == "SetWeaponCooldownBonus")
	{
		GRAB_SHIP;
		ship->SetWeaponCooldownBonus(arguments[1].ParseFloat()); 
		result = ExpressionResult::Boolean(true);
		return true;
	}
	else if (name == "SetWeaponCooldown")
	{
		GRAB_SHIP;
		ship->SetWeaponCooldownByID(arguments[1].ParseInt(), arguments[2].ParseFloat());
		result = ExpressionResult::Boolean(true);
		return true;
	}
	else if (name == "CreateTimer")
	{
		// Store current TimeMs as a variable with that name?
		GameVars.CreateTime(arguments[0], levelTime);
		result = ExpressionResult::Boolean(true);
		return true;
	}
	else if (name == "TimeElapsedMs")
	{
		GameVar * var = GameVars.GetTime(arguments[0]);
		if (!var)
			result = ExpressionResult::Boolean(false);
		result = ExpressionResult::Integral((levelTime - var->timeValue).Milliseconds());
		return true;
	}
	else if (name == "ResetTimer")
	{
		GameVar * var = GameVars.GetTime(arguments[0]);
		if (!var)
			result = ExpressionResult::Boolean(false);
		var->timeValue = levelTime;
		result = ExpressionResult::Boolean(true);
		return true;		
	}
	else if (name == "Log")
	{
		String catenated;
		for (int i = 0; i < arguments.Size(); ++i)
		{
			catenated += arguments[i];
		}
		std::cout<<"\n"<<catenated;
//		LogMain(arguments[0], INFO);
		result = ExpressionResult::Boolean(true);
		return true;
	}
	else if (name == "CreateFloat")
	{
		GameVars.CreateFloat(arguments[0], arguments[1].ParseFloat());
		result = ExpressionResult::Boolean(true);
		return true;
	}
	else if (name == "SetFloat")
	{
		GameVar * var = GameVars.GetFloat(arguments[0]);
		var->fValue = arguments[1].ParseFloat();
		result = ExpressionResult::Boolean(true);
		return true;
	}
	else if (name == "CreateInt")
	{
		GameVars.CreateInt(arguments[0], arguments[1].ParseFloat());
		result = ExpressionResult::Boolean(true);
		return true;
	}
	else if (name == "SetInt")
	{
		GameVar * var = GameVars.GetInt(arguments[0]);
		var->iValue = arguments[1].ParseInt();
		result = ExpressionResult::Boolean(true);
		return true;
	}
//	exp.text = "Unable to find function";
	return false;
}

bool SpaceShooterEvaluator::IsFunction(String name)
{
	if (name == "ArrivedAtDestination" || name == "TimeInCurrentMovement" ||
		name == "PositionY" || name == "PositionX" || name == "DisableWeapon" || name == "SetMovementPattern" ||
		name == "HPPercent" || name == "PartsDestroyed" || name == "SetProjectileSpeedBonus" ||
		name == "CreateTimer" || name == "TimeElapsedMs" || name == "ResetTimer" || name == "Log" ||
		name == "CreateInt" || name == "SetInt" || name == "CreateFloat" || name == "SetFloat" ||
		name == "SetWeaponCooldownBonus" || name == "EnableWeapon" || name == "DisableAllWeapons" ||
		name == "SetWeaponCooldown")
		return true;
	return false;
}


