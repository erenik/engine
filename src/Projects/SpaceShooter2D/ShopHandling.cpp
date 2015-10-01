/// Emil Hedemalm
/// 2015-10-01
/// Handling of shop-purchase-selling code

#include "SpaceShooter2D.h"

// For shop/UI-interaction.
Vector2i WeaponTypeLevelFromString(String str)
{
	String weaponString = str - "Weapon";
	int weapon = weaponString.ParseInt();
	String levelString = str.Tokenize("_")[1] - "Level";
	int level = levelString.ParseInt();
	return Vector2i(weapon, level);
} 
String WeaponTypeLevelToString(int type, int level)
{
	return "Weapon"+String(type)+"_Level"+String(level);
}

int DiffCost(String toUpgrade)
{
	bool selling = false;
	bool buying = false;
	// Check weapon type and level.
	Vector2i typeLevel = WeaponTypeLevelFromString(toUpgrade);
	int type = typeLevel.x, level = typeLevel.y;
	int playerWeapCurrLevel = playerShip->weapons[type]->level;
	if (level > playerWeapCurrLevel)
		buying = true;
	else if (level < playerWeapCurrLevel)
		selling = true;
	Weapon targetWeapon;
	int totalCost = 0;
	int targetLevelReached = playerWeapCurrLevel;
	// Check the request level is available?
	if (buying)
	{
		// Check the request level is available?
		for (int i = playerWeapCurrLevel + 1; i <= level; ++i)
		{
			Weapon * weapon = Weapon::Get(type, i);
			if (!weapon)
				break;
			targetWeapon = *weapon;
			totalCost += targetWeapon.cost;
			targetLevelReached = i;
		}
	}
	else if (selling)
	{
		// Check the request level is available?
		for (int i = playerWeapCurrLevel; i > level; --i)
		{
			Weapon * weapon = Weapon::Get(type, i);
			if (!weapon)
				break;
			targetWeapon = *weapon;
			totalCost -= targetWeapon.cost;
			targetLevelReached = i - 1;
		}	
	}
	return totalCost;	
}

void SpaceShooter2D::BuySellToUpgrade(String upgrade)
{
	Vector2i typeLevel = WeaponTypeLevelFromString(upgrade);
	int type = typeLevel.x, level = typeLevel.y;
	// Calculate diffs to buy/sell.
	int totalCost = 0;
	int playerWeapCurrLevel = playerShip->weapons[type]->level;
	int targetLevelReached = playerWeapCurrLevel;
	totalCost = DiffCost(upgrade);
	if (totalCost > money->GetInt())
	{
		/// Unable to buy, error message?
		return;
	}
	money->SetInt(money->GetInt() - totalCost);
	// add/subtract munnies
	UpdateUpgradesMoney();
	// set new levels
	Weapon * newWeapon = Weapon::Get(type, level);
	if (newWeapon)
	{
		*playerShip->weapons[type] = *newWeapon;
	}
	else 
	{
		playerShip->weapons[type]->name = "";
		playerShip->weapons[type]->level = 0;
		playerShip->weapons[type]->cooldown = Time(TimeType::MILLISECONDS_NO_CALENDER, 50000000);
	}
	// update ui.
	UpdateUpgradeStatesInList();

}
