
#include "GameVariables.h"

GameVariableManager * GameVariableManager::gameVarMan = NULL;

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
GameVariablei * GameVariableManager::CreateInt(String name, int initialValue){
	GameVariable * exists = Get(name);
	if (exists){
		return (GameVariablei*)exists;
	}
	GameVariablei * integer = new GameVariablei(name, initialValue);
	integers.Add(integer);
	gameVariables.Add(integer);
	return integer;
}