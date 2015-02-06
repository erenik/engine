/// Emil Hedemalm
/// 2015-02-06
/// o.o

#include "Gear.h"
#include "File/File.h"
#include "String/StringUtil.h"

/// Available to buy!
List<Gear> Gear::availableGear;

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
			else if (column == "Max Shield")
				gear.maxShield = value.ParseInt();
			else if (column == "Shield Regen")
				gear.shieldRegen = value.ParseInt();
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


