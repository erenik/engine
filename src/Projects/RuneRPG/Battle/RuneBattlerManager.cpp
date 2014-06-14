// Emil Hedemalm
// 2013-07-09

#include "RuneBattlerManager.h"
#include <cmath>
#include <fstream>
#include <cstring>
#include "Battle/BattleAction.h"
#include "File/FileUtil.h"
#include "File/File.h"
;

void AddActionToBattler(RuneBattler * battler, String actionName){
    BattleAction * ba = BALib.Get(actionName);
    if (ba){
        battler->actions.Add(ba);
		assert(ba->category);
		if (!ba->category){
			std::cout<<"\nWARNING: Action "<<actionName<<" lacking category. Fix this.";
			return;
		}
        if (!battler->actionCategories.Exists(ba->category))
            battler->actionCategories.Add(ba->category);
    }
}

RuneBattlerManager * RuneBattlerManager::runeBattlerManager = NULL;
String RuneBattlerManager::rootBattlerDir = "data/Battlers/";

RuneBattlerManager::RuneBattlerManager(){}
RuneBattlerManager::~RuneBattlerManager(){
	for (int i = 0; i < battlerTypes.Size(); ++i)
		delete battlerTypes[i];
	battlerTypes.Clear();
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

/// Looks for a "Battlers.list" which should then specify the battler-files to load.
bool RuneBattlerManager::LoadFromDirectory(String dir){
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
		bool result = battler->LoadFromFile(battlerSource);
		if (!result)
			delete battler;
		else {
			for (int b = 0; b < battlerTypes.Size(); ++b){
				String name = battlerTypes[b]->name;
				if (name == battler->name){
					std::cout<<"\nWARNING: Battler with name "<<name<<" already exists! Deleting previous entry.";
					battlerTypes.Remove(battlerTypes[b]);
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

            battlerTypes.Add(battler);
		}
	}
	std::cout<<"\nBattlerTypes now "<<battlerTypes.Size();
	return true;
}

bool RuneBattlerManager::LoadBattles(String fromDirectory){
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
	return true;
}

const RuneBattler * RuneBattlerManager::LoadBattler(String fromSource)
{
	RuneBattler * battler = new RuneBattler();
	if (!fromSource.Contains(rootBattlerDir))
		fromSource = rootBattlerDir + fromSource;
	if (battler->LoadFromFile(fromSource))
	{
		this->battlerTypes.Add(battler);
		return battler;
	}
	return NULL;
}

Battle * RuneBattlerManager::LoadBattle(String source)
{
	List<String> lines = File::GetLines(source);
	Battle * b = new Battle();
	// Default to loading all active players too.
	b->addCurrentPlayers = true;
	enum {
		NONE,
		LOAD_PLAYERS,
		LOAD_ENEMIES,
	};
	int loadingState = LOAD_ENEMIES;
	for (int i = 0; i < lines.Size(); ++i){
		String & line = lines[i];
		// Try load the battler from the relative directory.
		if (line.Contains("//"))
			continue;
		line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (line.Contains("name")){
			line.Remove("name");
			if (line.Contains("\"")){
				b->name = line.Tokenize("\"")[1];
			}
			else {
				line.RemoveInitialWhitespaces();
				b->name = line;
			}
		}
		else if (line.Length() < 3)
			continue;
		else if (loadingState == LOAD_PLAYERS)
			b->playerNames.Add(line);
		else if (loadingState == LOAD_ENEMIES)
			b->enemyNames.Add(line);
	}
	b->source = source;
	// Remove battles with the same name, enabling re-loading!
	for (int i = 0; i < battles.Size(); ++i){
		if (battles[i]->name == b->name){
			std::cout<<"\nWARNING: Battle with name "<<b->name<<" already exists! Deleting previous entry.";
			battles.Remove(battles[i]);
		}
	}
	battles.Add(b);
	return b;
}

Battle RuneBattlerManager::GetBattleBySource(String source)
{
	for (int i = 0; i < battles.Size(); ++i)
		if (battles[i]->source == source)
			return *battles[i];
	Battle * loaded = this->LoadBattle(source);
	if (loaded)
		return Battle(*loaded);
	return Battle();
}

/// The default directory and file ending will be added automatically as needed. 
const RuneBattler * RuneBattlerManager::GetBattlerBySource(String bySource)
{
	if (!bySource.Contains(rootBattlerDir)){
		bySource = rootBattlerDir + bySource;
	}
	if (!bySource.Contains(".b"))
		bySource += ".b";

	for (int i = 0; i < battlerTypes.Size(); ++i){
		String source = battlerTypes[i]->Source();
		if (source == bySource)
			return battlerTypes[i];
	}
	// Could not find it? try loading it?
	return this->LoadBattler(bySource);
}

RuneBattler RuneBattlerManager::GetBattlerType(String byName){
	assert(battlerTypes.Size() > 0);

	std::cout<<"\nGetBattlerType: by name "<<byName<<" out of "<<battlerTypes.Size()<<" types";
	for (int i = 0; i < battlerTypes.Size(); ++i){
		String battlerName = battlerTypes[i]->name;
		std::cout<<"\nBattler "<<i<<": "<<battlerName;
		if (battlerTypes[i]->name == byName){
			return *battlerTypes[i];
		}
	}
	std::cout<<"\nERROR: There is no RuneBattler with name \""<<byName<<"\"!";
	assert(false && "Undefined RuneBattler type!");
	return RuneBattler();
}
