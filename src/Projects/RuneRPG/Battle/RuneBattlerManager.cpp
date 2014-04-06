// Emil Hedemalm
// 2013-07-09

#include "RuneBattlerManager.h"
#include <cmath>
#include <fstream>
#include <cstring>
#include "Battle/BattleAction.h"
#include "File/FileUtil.h"
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

bool RuneBattlerManager::LoadBattle(String source){
	std::fstream file;
	file.open(source.c_str(), std::ios_base::in);
	if (!file.is_open()){
		std::cout<<"\nERROR: Unable to open file stream to "<<source;
		file.close();
		return false;
	}
	int start  = (int) file.tellg();
	file.seekg( 0, std::ios::end );
	int fileSize = (int) file.tellg();
	char * data = new char [fileSize];
	memset(data, 0, fileSize);
	file.seekg( 0, std::ios::beg);
	file.read((char*) data, fileSize);
	file.close();
	String fileContents(data);
	delete[] data; data = NULL;
	int loadingType = 0;
	List<String> lines = fileContents.GetLines();
	Battle * b = new Battle();
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
		else if (line.Contains("players"))
			loadingType = 0;
		else if (line.Contains("enemies"))
			loadingType = 1;
		else if (loadingType == 0)
			b->playerNames.Add(line);
		else if (loadingType == 1)
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
	return true;
}

Battle RuneBattlerManager::GetBattleBySource(String source){
	for (int i = 0; i < battles.Size(); ++i)
		if (battles[i]->source == source)
			return *battles[i];
	return Battle();
}

/// The default directory and file ending will be added automatically as needed. 
RuneBattler RuneBattlerManager::GetBattlerBySource(String bySource){
	if (!bySource.Contains("data/")){
		bySource = "data/Battlers/" + bySource;
	}
	if (!bySource.Contains(".b"))
		bySource += ".b";

	for (int i = 0; i < battlerTypes.Size(); ++i){
		String source = battlerTypes[i]->Source();
		if (source == bySource)
			return *battlerTypes[i];
	}
	return RuneBattler();
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
