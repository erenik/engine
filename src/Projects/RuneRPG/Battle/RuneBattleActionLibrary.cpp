/// Emil Hedemalm
/// 2014-06-20
/// Library for managing all actions available for battlers in the RuneRPG game

#include "RuneBattleActionLibrary.h"
#include "Elements.h"
#include "File/File.h"

RuneBattleActionLibrary * RuneBattleActionLibrary::runeBattleActionLibrary = NULL;

RuneBattleActionLibrary::RuneBattleActionLibrary()
{
}
RuneBattleActionLibrary::~RuneBattleActionLibrary()
{
	AllActions().ClearAndDelete();
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
	List<RuneBattleAction*> allActions = AllActions();
	for (int i = 0; i < allActions.Size(); ++i)
	{
		RuneBattleAction * rba = allActions[i];
		if (rba->name == byNameOrSource)
			return rba;
	}
	return NULL;
}

/// Fetches battle action by source file.
const RuneBattleAction * RuneBattleActionLibrary::GetBattleActionBySource(String bySource)
{
	assert(false);
	/*
#define ACTION_DIR "data/BattleActions/"
	if (!bySource.Contains(ACTION_DIR))
		bySource = ACTION_DIR + bySource;
	if (!bySource.Contains(".ba"))
		bySource = bySource + ".ba";
	for (int i = 0; i < AllActions().Size(); ++i)
	{
		RuneBattleAction * rab = (RuneBattleAction *) AllActions()[i];
		if (rab->source == bySource)
			return rab;
	}
	return LoadBattleAction(bySource);
*/
	return NULL;
}
	

/// Reloads all battle actions.
void RuneBattleActionLibrary::Reload()
{
	assert(false);
	/*
	for (int i = 0; i < runeBattleActions.Size(); ++i)
	{
		RuneBattleAction * rab = runeBattleActions[i];
		rab->Load(rab->source);
	}*/
}

/// Creates Attack, Flee and.. Use Item?
void RuneBattleActionLibrary::CreateDefaultActions()
{
	assert(mundaneActions.Size() == 0);
	RuneBattleAction * attack = new RuneBattleAction();
	attack->name = "Attack";
	attack->type = RuneBattleAction::MUNDANE_ACTION;
	attack->targetFilter = TargetFilter::ENEMY;

	BattleEffect damage;
	damage.type = BattleEffect::DAMAGE;
	damage.equation = "Base damage";
	damage.element = Element::PHYSICAL; 
//	damage.isPhysical = true;
	attack->effects.Add(damage);

	attack->actionCost = 20;
	attack->freezeTimeInSeconds = 0.5f;
	attack->castTimeInSeconds = 0.5f;

	mundaneActions.Add(attack);
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

/// Returns all spells, skills and mundane actions together.
List<RuneBattleAction*> RuneBattleActionLibrary::AllActions()
{
	return spells + skills + mundaneActions;
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
	List<RuneBattleAction*> actionsLoaded = LoadBattleActionsFromCSV(fileName);
	for (int i = 0; i < actionsLoaded.Size(); ++i)
	{
		RuneBattleAction * rba = actionsLoaded[i];
		rba->type = RuneBattleAction::MAGIC_SPELL;
	}
	spells = actionsLoaded;
	return actionsLoaded.Size() > 0;
}

/// Load from a CSV file (Comma-separated values).
bool RuneBattleActionLibrary::LoadSkillsFromCSV(String fileName)
{
	List<RuneBattleAction*> actionsLoaded = LoadBattleActionsFromCSV(fileName);
	for (int i = 0; i < actionsLoaded.Size(); ++i)
	{
		RuneBattleAction * rba = actionsLoaded[i];
		rba->type = RuneBattleAction::MAGIC_SKILL;
	}
	skills = actionsLoaded;
	return actionsLoaded.Size() > 0;
}

/// Load from a CSV file (Comma-separated values).
bool RuneBattleActionLibrary::LoadMundaneAbilitiesFromCSV(String fileName)
{
	List<RuneBattleAction*> actionsLoaded = LoadBattleActionsFromCSV(fileName);
	for (int i = 0; i < actionsLoaded.Size(); ++i)
	{
		RuneBattleAction * rba = actionsLoaded[i];
		rba->type = RuneBattleAction::MUNDANE_ACTION;
	}
	this->mundaneActions = actionsLoaded;
	return actionsLoaded.Size() > 0;
}

/// Loads battle actions from target CSV file (actions, spell, mundane skills, w/e).
List<RuneBattleAction*> RuneBattleActionLibrary::LoadBattleActionsFromCSV(String fileName)
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

	List<RuneBattleAction*> actionsLoaded;

	// For each line after the first one, parse a spell.
	for (int j = 1; j < lines.Size(); ++j)
	{
		String & line = lines[j];
		// Keep empty strings or all will break.
		List<String> values = line.Tokenize(separator, true);
		// If not, now loop through the words, parsing them according to the column name.
		// First create the new spell to load the data into!
		RuneBattleAction * newAction = new RuneBattleAction();
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
				newAction->name = value;
			else if (column.Contains("ID"))
				newAction->id = value;
			else if (column.Contains("Description"))
				newAction->description = value;
			else if (column.Contains("Keywords"))
				newAction->keywords = value;
			else if (column.Contains("Mana cost"))
				newAction->manaCost = value.ParseInt();
			else if (column.Contains("Action cost"))
				newAction->actionCost = value.ParseInt();
			else if (column.Contains("Other costs"))
				newAction->ParseOtherCosts(value);
			else if (column.Contains("Power"))
				newAction->spellPower = value.ParseInt();
			else if (column.Contains("Target"))
				// As the spell may have multiple targets, parse it as well.
				newAction->ParseTargets(value);
			else if (column.Contains("Location"))
				;
	//				spell->originatingLocation = GetOriginatingLocationByString(word);
			else if (column.Contains("Element"))
				newAction->ParseElements(value);
			else if (column.Contains("Effect"))
				newAction->ParseEffects(value);
			else if (column.Contains("Freeze time"))
				newAction->freezeTimeInSeconds = value.ParseFloat();
			else if (column.Contains("Cast time"))
				newAction->castTimeInSeconds = value.ParseFloat();
			else if (column.Contains("Duration"))
			{
				error = !newAction->ParseDurations(value);
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
		if (!newAction->IsValid())
			delete newAction;
		else 
		{
			actionsLoaded.Add(newAction);
		}
	}
	return actionsLoaded;
}



