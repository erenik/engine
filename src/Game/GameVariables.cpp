
#include "GameVariables.h"

GameVariableManager * GameVariableManager::gameVarMan = NULL;

/// Deletes all current game variables. Make sure you save your file before calling this! Should be called upon New game, as all scripts use (or should use) these game vars.
void GameVariableManager::Clear()
{
	gameVariables.ClearAndDelete();
	integers.Clear();
	int64s.Clear();
	floats.Clear();
	strings.Clear();
	vectors.Clear();
}

/// General getter for any variable.
GameVariable * GameVariableManager::Get(String name){
	for (int i = 0; i < gameVariables.Size(); ++i){
		if (gameVariables[i]->name == name)
			return gameVariables[i];
	}
	return NULL;
}

/// Getter for strings
GameVariables * GameVariableManager::GetString(String stringName){
	for (int i = 0; i < gameVariables.Size(); ++i){
		GameVariable * var = gameVariables[i];
		if (var->type != GameVariable::STRING)
			continue;
		if (var->name == stringName)
			return (GameVariables*)var;
	}
	return NULL;
}

/// Specific getters.
int GameVariableManager::GetInt(String name){
	for (int i = 0; i < integers.Size(); ++i){
		if (integers[i]->name == name){
			return integers[i]->value;
		}
	}
	std::cout<<"\nERROR: No such integer with given name!";
	assert(false && "No such integer with given name!");
	return -1;
}
// SEttetrrrrr
void GameVariableManager::SetInt(String name, int intValue){
	for (int i = 0; i < integers.Size(); ++i){
		if (integers[i]->name == name){
			integers[i]->Set(intValue);
			return;
		}
	}
	std::cout<<"\nERROR: No such integer with given name!";
	assert(false && "No such integer with given name!");
	return;
}


/// Creators
GameVariablei * GameVariableManager::CreateInt(String name, int initialValue)
{
	GameVariable * exists = Get(name);
	if (exists)
	{
		if (exists->Type() == GameVariable::INTEGER)
			return (GameVariablei*)exists;
		assert(false);
		return NULL;
	}
	GameVariablei * integer = new GameVariablei(name, initialValue);
	integers.Add(integer);
	gameVariables.Add(integer);
	return integer;
}

GameVariablei64 * GameVariableManager::CreateInt64(String name, int64 initialValue /*= 0*/)
{
	GameVariable * exists = Get(name);
	if (exists)
	{
		if (exists->Type() == GameVariable::INT64)
			return (GameVariablei64*)exists;
		assert(false);
		return NULL;
	}
	GameVariablei64 * integer = new GameVariablei64(name, initialValue);
	int64s.Add(integer);
	gameVariables.Add(integer);
	return integer;
}