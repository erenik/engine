/// Emil Hedemalm
/// 2014-06-20
/// Library for managing all actions available for battlers in the RuneRPG game

#include "RuneBattleActionLibrary.h"

#include "File/File.h"

RuneBattleActionLibrary * RuneBattleActionLibrary::runeBattleActionLibrary = NULL;

RuneBattleActionLibrary::RuneBattleActionLibrary()
{
}
RuneBattleActionLibrary::~RuneBattleActionLibrary()
{
	runeBattleActions.ClearAndDelete();
}
void RuneBattleActionLibrary::Allocate()
{
	assert(runeBattleActionLibrary == NULL);
	runeBattleActionLibrary = new RuneBattleActionLibrary();
}
void RuneBattleActionLibrary::Deallocate()
{
	assert(runeBattleActionLibrary);
	delete runeBattleActionLibrary;
	runeBattleActionLibrary = NULL;
}
RuneBattleActionLibrary * RuneBattleActionLibrary::Instance()
{
	assert(runeBattleActionLibrary);
	return runeBattleActionLibrary;
}

/// Fetches battle action by name or source file.
const RuneBattleAction * RuneBattleActionLibrary::GetBattleAction(String byNameOrSource)
{
//	RuneBattleAction * rba = 
	return GetBattleActionBySource(byNameOrSource);
}

/// Fetches battle action by source file.
const RuneBattleAction * RuneBattleActionLibrary::GetBattleActionBySource(String bySource)
{
#define ACTION_DIR "data/BattleActions/"
	if (!bySource.Contains(ACTION_DIR))
		bySource = ACTION_DIR + bySource;
	if (!bySource.Contains(".ba"))
		bySource = bySource + ".ba";
	for (int i = 0; i < runeBattleActions.Size(); ++i)
	{
		RuneBattleAction * rab = (RuneBattleAction *) runeBattleActions[i];
		if (rab->source == bySource)
			return rab;
	}
	return LoadBattleAction(bySource);

}
	

/// Reloads all battle actions.
void RuneBattleActionLibrary::Reload()
{
	for (int i = 0; i < runeBattleActions.Size(); ++i)
	{
		RuneBattleAction * rab = runeBattleActions[i];
		rab->Load(rab->source);
	}
}



const RuneBattleAction * RuneBattleActionLibrary::LoadBattleAction(String bySource)
{
	RuneBattleAction * newRab = new RuneBattleAction();
	// Doesn't exist? Too bad.
	return NULL;
	/*
	bool result = newRab->Load(bySource);
	if (!result)
	{
		delete newRab;
		return NULL;
	}
	runeBattleActions.Add(newRab);
	return newRab;
	*/
}

/// Returns list of all spells, as battle-action objects. The object will be created as usual via the BattleActionLibrary.
List<RuneBattleAction*> RuneBattleActionLibrary::GetSpells()
{
	return spells;
}

/// Returns list of all spells, as battle-action objects. The object will be created as usual via the BattleActionLibrary.
List<RuneBattleAction*> RuneBattleActionLibrary::GetSkills()
{
	return skills;
}

/// Load from a CSV file (Comma-separated values).
bool RuneBattleActionLibrary::LoadSpellsFromCSV(String fileName)
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

	// For each line after the first one, parse a spell.
	for (int j = 1; j < lines.Size(); ++j)
	{
		String & line = lines[j];
		// Keep empty strings or all will break.
		List<String> values = line.Tokenize(separator, true);
		// If not, now loop through the words, parsing them according to the column name.
		// First create the new spell to load the data into!
		RuneBattleAction * spell = new RuneBattleAction();
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
				spell->name = value;
			else if (column.Contains("ID"))
				spell->id = value;
			else if (column.Contains("Description"))
				spell->description = value;
			else if (column.Contains("Keywords"))
				spell->keywords = value;
			else if (column.Contains("Mana cost"))
				spell->manaCost = value.ParseInt();
			else if (column.Contains("Action cost"))
				spell->actionCost = value.ParseInt();
			else if (column.Contains("Other costs"))
				spell->ParseOtherCosts(value);
			else if (column.Contains("Power"))
				spell->spellPower = value.ParseInt();
			else if (column.Contains("Target"))
				// As the spell may have multiple targets, parse it as well.
				spell->ParseTargets(value);
			else if (column.Contains("Location"))
				;
	//				spell->originatingLocation = GetOriginatingLocationByString(word);
			else if (column.Contains("Element"))
				spell->ParseElements(value);
			else if (column.Contains("Effect"))
				spell->ParseEffects(value);
			else if (column.Contains("Freeze time"))
				spell->freezeTimeInSeconds = value.ParseFloat();
			else if (column.Contains("Cast time"))
				spell->castTimeInSeconds = value.ParseFloat();
			else if (column.Contains("Duration"))
			{
				error = !spell->ParseDurations(value);
			}
			else 
			{
		//		std::cout<<"\nUnknown column D:";
			}
			if (error)
			{
				std::cout<<"\n .. when parsing line \'"<<line<<"\'";
			}
		}
		if (!spell->IsValid())
			delete spell;
		else
			spells.Add(spell);
	}
	return true;
}



