
/// Emil Hedemalm
/// 2015-10-04
/// Editable user-scripts

#include "WeaponScript.h"

List<WeaponScript*> weaponScripts;

String ScriptAction::GetStringForType(int type)
{
	switch(type)
	{
		case SWITCH_TO_WEAPON:
			return "Switch to weapon";
		default:
			return "Bad type";
	}
}

ScriptAction ScriptAction::SwitchWeapon(int toWeaponIndex, int durationToHoldMs)
{
	ScriptAction sa;
	sa.type = SWITCH_TO_WEAPON;
	sa.name = ScriptAction::GetStringForType(sa.type);
	sa.weaponIndex = toWeaponIndex;
	sa.durationMs = durationToHoldMs;
	return sa;
}

ScriptAction::ScriptAction()
{
	Nullify();
}

ScriptAction::ScriptAction(int in_type)
{
	Nullify();
	this->type = in_type;
	name = ScriptAction::GetStringForType(type);
}
void ScriptAction::Nullify()
{
	type = -1;
	weaponIndex = 0;
	durationMs = 0;
}


void ScriptAction::OnEnter(Ship * forShip)
{
	if (type == SWITCH_TO_WEAPON)
	{
		forShip->activeWeapon = forShip->weapons[weaponIndex];
		forShip->shoot = true;
	}
}

WeaponScript * lastEdited = 0;

WeaponScript::WeaponScript()
{
	timeInCurrentActionMs = 0;
	currentAction = 0;
	static int numScripts = 0;
	name = "Weapon script "+String(numScripts++);
	lastEdited = this;
}

void WeaponScript::CreateDefault()
{
	WeaponScript * weaponScript = new WeaponScript();
	weaponScript->actions.AddItem(ScriptAction::SwitchWeapon(0, 1000));
	weaponScript->actions.AddItem(ScriptAction::SwitchWeapon(1, 1000));
	weaponScript->actions.AddItem(ScriptAction::SwitchWeapon(2, 100));
	weaponScripts.AddItem(weaponScript);
}

void WeaponScript::Process(Ship * forShip, int timeInMs)
{
	assert(actions.Size());
	timeInCurrentActionMs += timeInMs;
	ScriptAction & current = actions[currentAction];
	if (timeInCurrentActionMs > current.durationMs)
	{
		currentAction = (currentAction + 1) % actions.Size();
		// When entering a new one, do stuff.
		ScriptAction & newOne = actions[currentAction];
		newOne.OnEnter(forShip);
		timeInCurrentActionMs = 0;
	}
}

WeaponScript * WeaponScript::LastEdited()
{
	return lastEdited;
}
