/// Emil Hedemalm
/// 2015-10-04
/// Editable user-scripts

#ifndef WEAPON_SCRIPT_H
#define WEAPON_SCRIPT_H

#include "Ship.h"

class ScriptAction 
{
public:
	ScriptAction();
	ScriptAction(int type);
	void Nullify();
	void OnEnter(Ship * forShip);

	static ScriptAction SwitchWeapon(int toWeaponIndex, int durationToHoldMs);
	static String GetStringForType(int type);

	enum {
		SWITCH_TO_WEAPON,
		MAX_TYPES,
	};
	String name;
	int type;
	int weaponIndex;
	int durationMs;
	Time startTime;
};

extern List<WeaponScript*> weaponScripts;

class WeaponScript
{
public:
	WeaponScript();
	static void CreateDefault();
	void Process(Ship * forShip, int timeInMs);
	static WeaponScript * LastEdited();
	int timeInCurrentActionMs;
	String name;
	List<ScriptAction> actions;
	Time lastEdit;
private:
	int currentAction;
};

#endif
