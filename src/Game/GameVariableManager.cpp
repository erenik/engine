/// Emil Hedemalm
/// 2015-02-06
/// Manager for most variables used within e.g. a game. May be used for other purposes. These vars are usually saved on a per game-save basis.

#include "GameVariableManager.h"
#include "File/LogFile.h"
#include <fstream>

GameVariableManager * GameVariableManager::gameVarMan = NULL;

GameVariableManager::~GameVariableManager()
{
	gameVariables.ClearAndDelete();
}


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

List<GameVar*> GameVariableManager::All()
{
	return gameVariables;
}

List<Variable> GameVariableManager::GetAllExpressionVariables()
{
	List<Variable> vars;
	Variable var;
	for (int i = 0; i < gameVariables.Size(); ++i)
	{
		GameVariable * gv = gameVariables[i];
		switch(gv->Type())
		{
			case GameVariable::INTEGER:
				var = Variable(gv->name, gv->GetInt());
				vars.AddItem(var);
				break;
			case GameVariable::FLOAT:
				var = Variable(gv->name, gv->fValue);
				vars.AddItem(var);
				break;
			case GameVariable::VECTOR:
			case GameVariable::VECTOR_4F:
			case GameVariable::VECTOR_3F:
			case GameVariable::STRING:
			case GameVariable::TIME:
				break;
			default:
				assert(false && "Fix for all data types..");
		}
	}
	return vars;
}


/// General getter for any variable.
GameVar * GameVariableManager::Get(String name){
	for (int i = 0; i < gameVariables.Size(); ++i){
		if (gameVariables[i]->name == name)
			return gameVariables[i];
	}
	return NULL;
}

/// Getter for strings
GameVar * GameVariableManager::GetString(String stringName){
	for (int i = 0; i < gameVariables.Size(); ++i){
		GameVar * var = gameVariables[i];
		if (var->type != GameVariable::STRING)
			continue;
		if (var->name == stringName)
			return var;
	}
	return NULL;
}

/// Specific getters.
GameVar * GameVariableManager::GetInt(String name)
{
	for (int i = 0; i < integers.Size(); ++i){
		if (integers[i]->name == name)
		{
			return integers[i];
//			return integers[i]->iValue;
		}
	}
	LogMain("ERROR: No such integer with given name!", ERROR);
	return nullptr;
}
GameVar * GameVariableManager::GetFloat(String name)
{
	for (int i = 0; i < floats.Size(); ++i)
	{
		if (floats[i]->name == name)
		{
			return floats[i];
		}
	}
	return 0;
}


GameVar * GameVariableManager::GetTime(String byName)
{
	for (int i = 0; i < timeVars.Size(); ++i)
	{
		GameVar * v = timeVars[i];
		if (v->name == byName)
			return v;
	}
	return 0;
}

int GameVariableManager::GetIntValue(String name, int defaultValue /*= 0*/) {
	GameVar * var = GetInt(name);
	if (var)
		return var->iValue;
	return defaultValue;
}


// SEttetrrrrr
void GameVariableManager::SetInt(String name, int intValue)
{
	for (int i = 0; i < integers.Size(); ++i){
		name.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (integers[i]->name == name){
			integers[i]->iValue = intValue;
			return;
		}
	}
	LogMain("No variable by name " + name + ", creating it.", WARNING);
	CreateInt(name, intValue);
}

void GameVariableManager::SetString(String name, String strValue) {
	for (int i = 0; i < strings.Size(); ++i) {
		name.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (strings[i]->name == name) {
			strings[i]->strValue = strValue;
			return;
		}
	}
	LogMain("No variable by name " + name + ", creating it.", WARNING);
	CreateString(name, strValue);
}

/// Creators
GameVar * GameVariableManager::CreateInt(String name, int initialValue)
{
	GameVariable * exists = Get(name);
	if (exists)
	{
		if (exists->Type() == GameVariable::INTEGER)
			return exists;
		assert(false);
		return NULL;
	}
	GameVar * integer = new GameVar(name, GameVariable::INTEGER);
	integer->iValue = initialValue;
	integers.Add(integer);
	gameVariables.Add(integer);
	return integer;
}

GameVar * GameVariableManager::CreateFloat(String name, float initialValue /*= 0*/)
{
	GameVariable * exists = Get(name);
	if (exists)
	{
		if (exists->Type() == GameVariable::FLOAT)
			return exists;
		assert(false);
		return NULL;
	}
	GameVar * floatr = new GameVar(name, GameVariable::FLOAT);
	floatr->fValue = initialValue;
	floats.Add(floatr);
	gameVariables.Add(floatr);
	return floatr;
}	

GameVar * GameVariableManager::CreateInt64(String name, int64 initialValue /*= 0*/)
{
	GameVariable * exists = Get(name);
	if (exists)
	{
		if (exists->Type() == GameVariable::INT64)
			return exists;
		assert(false);
		return NULL;
	}
	GameVar * i64 = new GameVar(name, GameVariable::INT64);
	i64->i64Value = initialValue;
	int64s.Add(i64);
	gameVariables.Add(i64);
	return i64;
}

GameVar * GameVariableManager::CreateString(String name, String initialValue)
{
	GameVariable * exists = Get(name);
	if (exists)
	{
		if (exists->Type() == GameVariable::STRING)
			return exists;
		assert(false);
		return NULL;
	}
	GameVar * str = new GameVar(name, GameVariable::STRING);
	str->strValue = initialValue;
	this->strings.Add(str);
	gameVariables.Add(str);
	return str;
}

GameVar * GameVariableManager::CreateTime(String name, Time initialValue)
{
	GameVar * exists = Get(name);
	if (exists)
	{
		if (exists->Type() == GameVariable::TIME)
			return exists;
		assert(false);
		return NULL;
	}
	GameVar * timeVar = new GameVar(name, GameVariable::TIME);
	timeVar->timeValue = initialValue;
	timeVars.Add(timeVar);
	gameVariables.Add(timeVar);
	return timeVar;
}


/// Saves all current game variables to target file-stream.
bool GameVariableManager::WriteTo(std::fstream & stream)
{
	// Write version
	int version = 0;
	stream.write((char*)&version, sizeof(int));
	// Number of vars.
	int num = gameVariables.Size();
	stream.write((char*)&num, sizeof(int));
	for (int i = 0; i < gameVariables.Size(); ++i)
	{
		bool ok = gameVariables[i]->WriteTo(stream);
		if (!ok)
			return false;
	}
	return true;
}

bool GameVariableManager::ReadFrom(std::fstream & stream)
{
	// Read version.
	int version;
	stream.read((char*) &version, sizeof(int));
	if (version != 0)
		return false;
	int num;
	// Readn umber of vars.
	int sizeofInt = sizeof(int);
	assert(sizeofInt == 4);
	stream.read((char*)&num, sizeof(int));
	for (int i = 0; i < num; ++i)
	{
		GameVar * newGV = new GameVar();
		bool ok = newGV->ReadFrom(stream);
		if (!ok)
			return false;
		// Check if a duplicate exists, if so copy over the values.
		GameVar * exist = Get(newGV->name);
		if (exist)
		{
			*exist = *newGV;
			delete newGV;
		}
		else
		{
			gameVariables.Add(newGV);
		}
	}
	return true;
}


