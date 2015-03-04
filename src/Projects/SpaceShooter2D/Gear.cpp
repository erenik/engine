/// Emil Hedemalm
/// 2015-02-06
/// o.o

#include "Gear.h"
#include "File/File.h"
#include "String/StringUtil.h"

/// Available to buy!
List<Gear> Gear::availableGear;

Gear::Gear()
{
	toughness = 10;
	reactivity = 0;
	price = -1;
	maxHP = -1;
	maxShield = -1;
	reloadTime = Time(TimeType::MILLISECONDS_NO_CALENDER, 0);
}

/// o.o
bool Gear::Load(String fromFile)
{
	List<String> lines = File::GetLines(fromFile);
	if (lines.Size() == 0)
		return false;
	String separator;
	/// Column-names. Parse from the first line.
	List<String> columns;
	String firstLine = lines[0];
	int commas = firstLine.Count(',');
	int semiColons = firstLine.Count(';');
	int delimiter = semiColons > commas? ';' : ',';
	columns = TokenizeCSV(firstLine, delimiter);
	for (int j = 1; j < lines.Size(); ++j)
	{
		String & line = lines[j];
		Gear gear;
		if (fromFile.Contains("Shield"))
			gear.type = SHIELD_GENERATOR;
		else if (fromFile.Contains("Armor"))
			gear.type = ARMOR;
		else 
			gear.type = WEAPON;
		List<String> values = TokenizeCSV(line, delimiter);
		for (int k = 0; k < values.Size(); ++k)
		{
			String column;
			bool error = false;
			/// In-case extra data is added beyond the columns defined above..?
			if (columns.Size() > k)
				column = columns[k];
			String value = values[k];
			if (value == "n/a")
				continue;
			column.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (column == "Name")
			{
				value.RemoveSurroundingWhitespaces();
				gear.name = value;
			}
			else if (column == "Type")
				gear.type = value.ParseInt();
			else if (column == "Price")
				gear.price = value.ParseInt();
			else if (column == "Damage")
				gear.damage = value.ParseInt();
			else if (column == "Reload time")
				gear.reloadTime.intervals = value.ParseInt();
			else if (column == "Max Shield")
				gear.maxShield = value.ParseInt();
			else if (column == "Shield Regen")
				gear.shieldRegen = value.ParseInt();
			else if (column == "Max HP")
				gear.maxHP = value.ParseInt();
			else if (column == "Toughness")
				gear.toughness = value.ParseInt();
			else if (column == "Reactivity")
				gear.reactivity = value.ParseInt();
			else if (column == "Info")
				gear.description = value;
		}
		// Remove copies or old data.
		for (int i = 0; i < availableGear.Size(); ++i)
		{
			if (availableGear[i].name == gear.name)
			{
				availableGear.RemoveIndex(i);
				--i;
			}
		}
		availableGear.Add(gear);
	}
}

List<Gear> Gear::GetType(int type)
{
	List<Gear> list;
	for (int i = 0; i < availableGear.Size(); ++i)
	{
		if (availableGear[i].type == type)
			list.Add(availableGear[i]);
	}
	return list;
}

Gear Gear::Get(String byName)
{
	for (int i = 0; i < availableGear.Size(); ++i)
	{
		if (availableGear[i].name == byName)
			return availableGear[i];
	}
	assert(false);
	return Gear();	
}

Gear Gear::StartingWeapon()
{
	List<Gear> weapons = GetType(WEAPON);
	for (int i = 0; i < weapons.Size(); ++i)
	{
		Gear & weapon = weapons[i];
		if (weapon.price == 0)
			return weapon;
	}
	assert(false);
	return Gear();
}
Gear Gear::StartingArmor()
{
	List<Gear> armors = GetType(ARMOR);
	for (int i = 0; i < armors.Size(); ++i)
	{
		Gear & armor = armors[i];
		if (armor.price == 0)
			return armor;
	}
	assert(false);
	return Gear();
}
Gear Gear::StartingShield()
{
	List<Gear> shields = GetType(SHIELD_GENERATOR);
	for (int i = 0; i < shields.Size(); ++i)
	{
		Gear & shield = shields[i];
		if (shield.price == 0)
			return shield;
	}
	assert(false);
	return Gear();
}


