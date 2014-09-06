/// Emil Hedemalm
/// 2014-04-10
/// Item class for the RuneRPG project.

#include "RuneItem.h"
#include "ItemTypes.h"
#include "File/File.h"
#include "RuneRPG/Battle/TargetFilters.h"
#include "RuneRPG/Battle/BattleStats.h"

List<RuneItem> RuneItem::allWeapons, RuneItem::allArmor, RuneItem::allConsumables, RuneItem::allKeyItems;

int GetSlot(String byName)
{
	for (int i = 0; i < Slot::SLOTS; ++i)
	{
		String name = GetSlotName(i);
		if (name == byName)
			return i;
	}
	return -1;
}
String GetSlotName(int slot)
{
	switch(slot)
	{
		case Slot::HEAD:
			return "Head";
		case Slot::TORSO:
			return "Torso";
		case Slot::HANDS:
			return "Hands";
		case Slot::FEET:
			return "Feet";
		case Slot::OFF_HAND:
			return "Off-hand";
	}
	assert(false);
	return String();
}


bool RuneItem::LoadWeaponsFromCSV(String fileName)
{
	List<RuneItem> loaded = LoadFromCSV(fileName);
	if (loaded.Size() == 0)
		return false;
	for (int i = 0; i < loaded.Size(); ++i)
	{
		loaded[i].type = ItemType::WEAPON;
	}
	allWeapons = loaded;
	return true;
}
bool RuneItem::LoadArmourFromCSV(String fileName)
{
	List<RuneItem> loaded = LoadFromCSV(fileName);
	if (loaded.Size() == 0)
		return false;
	for (int i = 0; i < loaded.Size(); ++i)
	{
		loaded[i].type = ItemType::GEAR;
	}
	allArmor = loaded;
	return true;
}
bool RuneItem::LoadConsumablesFromCSV(String fileName)
{
	List<RuneItem> loaded = LoadFromCSV(fileName);
	if (loaded.Size() == 0)
		return false;
	for (int i = 0; i < loaded.Size(); ++i)
	{
		loaded[i].type = ItemType::CONSUMABLE;
	}
	allConsumables = loaded;
	return true;
}

List<RuneItem> RuneItem::LoadFromCSV(String fileName)
{
	String contents = File::GetContents(fileName);
	List<String> lines = contents.GetLines();

	int tempLineNumber;

	String separator;
	int numCommas = contents.Count(',');
	int numSemisColons = contents.Count(';');
	if (numSemisColons > numCommas)
		separator = ";";
	else
		separator = ",";

	/// Column-names. Parse from the first line.
	List<String> columns;
	String firstLine = lines[0];
	// Keep empty strings or all will break.
	columns = firstLine.Tokenize(separator, true);

	List<RuneItem> itemsLoaded;

	// For each line after the first one, parse a spell.
	for (int j = 1; j < lines.Size(); ++j)
	{
		String & line = lines[j];
		// Keep empty strings or all will break.
		List<String> values = line.Tokenize(separator, true);
		// If not, now loop through the words, parsing them according to the column name.
		// First create the new spell to load the data into!
		RuneItem newItem = RuneItem();
		for (int k = 0; k < values.Size(); ++k)
		{
			String column;
			bool error = false;
			/// In-case extra data is added beyond the columns defined above..?
			if (columns.Size() > k)
				column = columns[k];
			String value = values[k];
			column.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (column == "Name")
				newItem.name = value;
			else if (column == "Slot")
			{
				newItem.slot = GetSlot(value);
			}
			else if (column == "Armour Rating")
			{
				newItem.equipEffects += BattleEffect::ConstantStatIncrease(RStat::ARMOR_RATING, value.ParseInt());
				newItem.armorRating = value.ParseInt();
			}
			else if (column == "Magic Armour Rating")
			{
				newItem.equipEffects += BattleEffect::ConstantStatIncrease(RStat::MAGIC_ARMOR_RATING, value.ParseInt());
				newItem.magicArmorRating = value.ParseInt();
			}
			else if (column == "Block modifier")
			{
				newItem.equipEffects += BattleEffect::ConstantStatIncrease(RStat::SHIELD_BLOCK_RATING, value.ParseInt());
				newItem.blockModifier = value.ParseInt();
			}
			else if (column == "Weight")
			{
				newItem.equipEffects += BattleEffect::ConstantStatIncrease(RStat::BATTLE_GEAR_WEIGHT, value.ParseInt());
				newItem.weight = value.ParseInt();
			}
			else if (column == "Weapon modifier")
			{
				newItem.equipEffects += BattleEffect::ConstantStatIncrease(RStat::WEAPON_MODIFIER, value.ParseInt());
				newItem.weaponModifier = value.ParseInt();
			}
			else if (column == "Action cost")
			{
				newItem.actionCost = value.ParseInt();
			}
			else if (column == "Parry Modifier")
			{
				newItem.equipEffects += BattleEffect::ConstantStatIncrease(RStat::PARRY_RATING, value.ParseInt());
				newItem.parryModifier = value.ParseInt();
			}
			else if (column == "Other Bonuses")
			{
				newItem.equipEffects += BattleEffect::ParseEffects(value);				
			}
			else if (column.Contains("Targeting"))
				// As the spell may have multiple targets, parse it as well.
				newItem.targetFilter = ParseTargetFilter(value);
			
			else if (column.Contains("Freeze time"))
				newItem.freezeTimeInSeconds = value.ParseFloat();
			else if (column.Contains("Cast time"))
				newItem.castTimeInSeconds = value.ParseFloat();
			else 
			{
		//		std::cout<<"\nUnknown column D:";
			}
			if (error)
			{
				std::cout<<"\n .. when parsing line \'"<<line<<"\'";
			}
		}
		itemsLoaded.Add(newItem);
	}
	return itemsLoaded;
}

/// Returns a list of gear as corresponding to the demanded string of comma-separated names.
List<RuneItem> RuneItem::GetGearByString(String str)
{
	List<RuneItem> gear;
	List<String> names = str.Tokenize(",");
	for (int i = 0; i < names.Size(); ++i)
	{
		String name = names[i];
		/// Remove unneccessary whitespaces at the start and end.
		name.RemoveInitialWhitespaces();
		name.RemoveTrailingWhitespaces();

		const RuneItem * gearPiece = NULL;
		gearPiece = GetWeapon(name);
		if (gearPiece)
			/// Add a copy of the original.
			gear.Add(RuneItem(*gearPiece));
		gearPiece = GetArmor(name);
		if (gearPiece)
			/// Add a copy of the original.
			gear.Add(RuneItem(*gearPiece));
	}
	return gear;
}

/// Returns reference gear by name.
const RuneItem * RuneItem::GetWeapon(String byName)
{
	for (int i = 0; i < allWeapons.Size(); ++i)
	{
		RuneItem * weapon = &allWeapons[i];
		if (weapon->name == byName)
			return weapon;
	}
	return NULL;
}
const RuneItem * RuneItem::GetArmor(String byName)
{
	for (int i = 0; i < allArmor.Size(); ++i)
	{
		RuneItem * armor = &allArmor[i];
		if (armor->name == byName)
			return armor;
	}
	return NULL;
}

/// Default weapon-slot: Unarmed
RuneItem RuneItem::DefaultWeapon()
{
	const RuneItem * unarmed = GetWeapon("Unarmed");
	if (unarmed)
		return *unarmed;
	RuneItem unarmedItem;
	unarmedItem.name = "Default unarmed weapon";
	unarmedItem.type = ItemType::WEAPON;
	return unarmedItem;
}

/// Default 1 item/gear in each slot and 1 weapon.
List<RuneItem> RuneItem::DefaultGear()
{
	List<RuneItem> gear;
	gear.Add(DefaultWeapon());
	for (int i = 0; i < Slot::SLOTS; ++i)
	{
		RuneItem piece;
		piece.name = "None";
		piece.slot = i;
		piece.type = ItemType::GEAR;
		gear.Add(piece);
	}
	return gear;
}


RuneItem::RuneItem()
{
	armorRating = magicArmorRating = blockModifier = weight = 0;
	weaponModifier = parryModifier = actionCost = 0;

}
	
RuneItem::~RuneItem()
{
}
