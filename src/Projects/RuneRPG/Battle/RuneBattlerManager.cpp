// Emil Hedemalm
// 2013-07-09

#include "RuneBattlerManager.h"
#include <cmath>
#include <fstream>
#include <cstring>
#include "Battle/BattleAction.h"
#include "File/FileUtil.h"
#include "File/File.h"

#include "BattleStats.h"
;

void AddActionToBattler(RuneBattler * battler, String actionName)
{
	assert(false);
}

RuneBattlerManager * RuneBattlerManager::runeBattlerManager = NULL;
String RuneBattlerManager::rootBattlerDir = "data/Battlers/";

RuneBattlerManager::RuneBattlerManager(){}
RuneBattlerManager::~RuneBattlerManager()
{
	battlers.ClearAndDelete();
}

RuneBattlerManager * RuneBattlerManager::Instance(){
	assert(runeBattlerManager);
	return runeBattlerManager;
}

void RuneBattlerManager::Allocate(){
	assert(runeBattlerManager == NULL);
	runeBattlerManager = new RuneBattlerManager();
}
void RuneBattlerManager::Deallocate(){
	assert(runeBattlerManager);
	delete runeBattlerManager;
	runeBattlerManager = NULL;
}

/// Reloads all battlers by their respective source file.
void RuneBattlerManager::ReloadBattles()
{
	for (int i = 0; i < runeBattles.Size(); ++i)
	{
		RuneBattle * battle = runeBattles[i];
		battle->Load(battle->source);
	}
}
/// Reloads all battlers by their respective source file.
void RuneBattlerManager::ReloadBattlers()
{
	assert(false);
	/*
	for (int i = 0; i < battlers.Size(); ++i)
	{
		RuneBattler * rb = battlers[i];
		rb->Load(rb->source);
	}
	*/
}


/// Looks for a "Battlers.list" which should then specify the battler-files to load.
bool RuneBattlerManager::LoadFromDirectory(String dir)
{
	assert(false);
	/*
    if (BALib.TotalsActions() == 0){
        std::cout<<"\nBattle action library is empty, consider re-loading it first, yo.";
        return false;
    }

	std::cout<<"\nRuneBattlerManager::LoadFromDirectory called.";
	
	List<String> files;
	bool result = GetFilesInDirectory(dir, files);
	if (!result)
		return false;

	for (int i = 0; i < files.Size(); ++i){
		RuneBattler * battler = new RuneBattler();
		String battlerSource = dir + files[i];
		if (!battlerSource.Contains(".b"))
			battlerSource += ".b";
		bool result = battler->Load(battlerSource);
		if (!result)
			delete battler;
		else {
			for (int b = 0; b < battlers.Size(); ++b){
				String name = battlers[b]->name;
				if (name == battler->name){
					std::cout<<"\nWARNING: Battler with name "<<name<<" already exists! Deleting previous entry.";
					battlers.Remove(battlers[b]);
				}
			}

			/// Bind the actions, yaow.
			for (int i = 0; i < battler->actionNames.Size(); ++i){
				battler->AddActions(BALib.GetBattleActions(battler->actionNames));
		//	AddActionToBattler(battler, battler->actionNames[i]);
            }
            /// Give them "Attack" if nothing was specified.
            if (battler->actions.Size() == 0)
                AddActionToBattler(battler, "Attack");

            battlers.Add(battler);
		}
	}
	std::cout<<"\nBattlerTypes now "<<battlers.Size();
	*/
	return true;
}

bool RuneBattlerManager::LoadBattles(String fromDirectory)
{
	assert(false);
	/*
	List<String> files;
	bool result = GetFilesInDirectory(fromDirectory, files);
	if (!result)
		return false;
	for (int i = 0; i < files.Size(); ++i){
		String fullPath = fromDirectory;
		if (fullPath.At(fullPath.Length()-1) != '/')
			fullPath += "/";
		fullPath += files[i];
		LoadBattle(fullPath);
	}
	*/
	return true;
}

const RuneBattler * RuneBattlerManager::LoadBattler(String fromSource)
{
	RuneBattler * battler = new RuneBattler();
	if (!fromSource.Contains(rootBattlerDir))
		fromSource = rootBattlerDir + fromSource;
	if (battler->Load(fromSource))
	{
		this->battlers.Add(battler);
		return battler;
	}
	return NULL;
}

RuneBattle * RuneBattlerManager::LoadBattle(String source)
{
	RuneBattle * b = new RuneBattle();
	bool result = b->Load(source);
	if (!result)
	{
		delete b;
		return NULL;
	}
	// Remove runeBattles with the same name, enabling re-loading!
	for (int i = 0; i < runeBattles.Size(); ++i){
		if (runeBattles[i]->name == b->name){
			std::cout<<"\nWARNING: RuneBattle with name "<<b->name<<" already exists! Deleting previous entry.";
			runeBattles.Remove(runeBattles[i]);
		}
	}
	runeBattles.Add(b);
	return b;
}

RuneBattle RuneBattlerManager::GetBattleBySource(String source)
{
	for (int i = 0; i < runeBattles.Size(); ++i)
		if (runeBattles[i]->source == source)
			return *runeBattles[i];
	RuneBattle * loaded = this->LoadBattle(source);
	if (loaded)
		return RuneBattle(*loaded);
	return RuneBattle();
}

/// The default directory and file ending will be added automatically as needed. 
const RuneBattler * RuneBattlerManager::GetBattlerBySource(String bySource)
{
	if (!bySource.Contains(rootBattlerDir)){
		bySource = rootBattlerDir + bySource;
	}
	if (!bySource.Contains(".b"))
		bySource += ".b";

	for (int i = 0; i < battlers.Size(); ++i){
		String source = battlers[i]->Source();
		if (source == bySource)
			return battlers[i];
	}
	// Could not find it? try loading it?
	return this->LoadBattler(bySource);
}

RuneBattler RuneBattlerManager::GetBattlerType(String byName)
{
	assert(battlers.Size() > 0);

	std::cout<<"\nGetBattlerType: by name "<<byName<<" out of "<<battlers.Size()<<" types";
	for (int i = 0; i < battlers.Size(); ++i){
		String battlerName = battlers[i]->name;
		std::cout<<"\nBattler "<<i<<": "<<battlerName;
		if (battlers[i]->name == byName){
			return *battlers[i];
		}
	}
	std::cout<<"\nERROR: There is no RuneBattler with name \""<<byName<<"\"!";
	assert(false && "Undefined RuneBattler type!");
	return RuneBattler();
}


/// Loads battlers from target CSV file.
bool RuneBattlerManager::LoadBattlersFromCSV(String fileName)
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

	int battlersLoaded = 0;
	// For each line after the first one, parse a spell.
	for (int j = 1; j < lines.Size(); ++j)
	{
		String & line = lines[j];
		// Keep empty strings or all will break.
		List<String> values = line.Tokenize(separator, true);
		// If not, now loop through the words, parsing them according to the column name.
		// First create the new spell to load the data into!
		RuneBattler * newBattler = new RuneBattler();
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
				newBattler->name = value;

			int stat = GetStatByString(column);
			if (stat != RStat::BAD_STAT)
			{
				newBattler->baseStats[stat].iValue = value.ParseInt();
				continue;
			}
			else if (column == "AP")
				newBattler->maxActionPoints = value.ParseInt();
			else if (column == "Armour total")
				newBattler->baseStats[RStat::ARMOR_RATING].iValue = value.ParseInt();
			else if (column == "Magic Armour total")
				newBattler->baseStats[RStat::MAGIC_ARMOR_RATING].iValue = value.ParseInt();
			else if (column == "Block")
				newBattler->canBlock = value.ParseBool();
			else if (column == "Parry")
				newBattler->canParry = value.ParseBool();
			else if (column == "Dodge")
				newBattler->canDodge = value.ParseBool();
			else if (column == "Abilities")
				newBattler->actionNames = value.Tokenize(",");

			if (error)
			{
				std::cout<<"\n .. when parsing line \'"<<line<<"\'";
			}
		}
		// Remove older versions of the same battler
		for (int i = 0; i < battlers.Size(); ++i)
		{
			RuneBattler * b = battlers[i];
			/// Identified by having the same name.
			if (b->name == newBattler->name)
			{
				battlers.Remove(b);
				delete b;
			}
		}
		// And add the new one.
		battlers.Add(newBattler);
		++battlersLoaded;
	}
	return battlersLoaded;
}
