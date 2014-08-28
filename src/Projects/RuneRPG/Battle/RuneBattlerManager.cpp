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

void AddActionToBattler(RuneBattler * battler, String actionName)
{
	assert(false);
}

RuneBattlerManager * RuneBattlerManager::runeBattlerManager = NULL;
String RuneBattlerManager::rootBattlerDir = "data/Battlers/";

RuneBattlerManager::RuneBattlerManager(){}
RuneBattlerManager::~RuneBattlerManager()
{
	for (int i = 0; i < runeBattlers.Size(); ++i)
		delete runeBattlers[i];
	runeBattlers.Clear();
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
	for (int i = 0; i < runeBattlers.Size(); ++i)
	{
		RuneBattler * rb = runeBattlers[i];
		rb->Load(rb->source);
	}
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
			for (int b = 0; b < runeBattlers.Size(); ++b){
				String name = runeBattlers[b]->name;
				if (name == battler->name){
					std::cout<<"\nWARNING: Battler with name "<<name<<" already exists! Deleting previous entry.";
					runeBattlers.Remove(runeBattlers[b]);
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

            runeBattlers.Add(battler);
		}
	}
	std::cout<<"\nBattlerTypes now "<<runeBattlers.Size();
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
		this->runeBattlers.Add(battler);
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

	for (int i = 0; i < runeBattlers.Size(); ++i){
		String source = runeBattlers[i]->Source();
		if (source == bySource)
			return runeBattlers[i];
	}
	// Could not find it? try loading it?
	return this->LoadBattler(bySource);
}

RuneBattler RuneBattlerManager::GetBattlerType(String byName){
	assert(runeBattlers.Size() > 0);

	std::cout<<"\nGetBattlerType: by name "<<byName<<" out of "<<runeBattlers.Size()<<" types";
	for (int i = 0; i < runeBattlers.Size(); ++i){
		String battlerName = runeBattlers[i]->name;
		std::cout<<"\nBattler "<<i<<": "<<battlerName;
		if (runeBattlers[i]->name == byName){
			return *runeBattlers[i];
		}
	}
	std::cout<<"\nERROR: There is no RuneBattler with name \""<<byName<<"\"!";
	assert(false && "Undefined RuneBattler type!");
	return RuneBattler();
}
