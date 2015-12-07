/// Emil Hedemalm
/// 2013-06-29 - rev 0
/// 2015-02-06 - rev 1, Read/Write added
/// Manager for most variables used within e.g. a game. May be used for other purposes. These vars are usually saved on a per game-save basis.


#ifndef GAME_VARIABLES_H
#define GAME_VARIABLES_H

#include "MathLib/Variable.h"
#include "GameVariable.h"

#define GameVars (*GameVariableManager::Instance())

/// Manager for global game variables!
class GameVariableManager{
	GameVariableManager() {};
	~GameVariableManager();
	static GameVariableManager * gameVarMan;
public:
	static void Allocate() {
		assert(gameVarMan == NULL);
		gameVarMan = new GameVariableManager();
	}
	static GameVariableManager * Instance(){
		assert(gameVarMan);
		return gameVarMan;
	}
	static void Deallocate(){
		assert(gameVarMan);
		delete gameVarMan;
		gameVarMan = NULL;
	}

	/// Deletes all current game variables. Make sure you save your file before calling this! Should be called upon New game, as all scripts use (or should use) these game vars.
	void Clear();
	
	List<GameVar*> All();
	List<Variable> GetAllExpressionVariables();

	/// General getter for any variable.
	GameVar * Get(String name);
	/// Gette for strings
	GameVar * GetString(String stringName);
	/// Specific getters.
	int GetInt(String name);
	
	// SEttetrrrrr
	void SetInt(String name, int intValue);

	/// Creators, returns the varible. If it exists, the existing variable will be returned.
	GameVar * CreateInt(String name, int initialValue = 0);
	GameVar * CreateInt64(String name, int64 initialValue = 0);
	GameVar * CreateString(String name, String initialValue = "");
	GameVar * CreateTime(String name, Time initialValue = Time::Now());
	
	/// Saves all current game variables to target file-stream.
	bool WriteTo(std::fstream & stream);
	bool ReadFrom(std::fstream & stream);

private:
	// Main
	List<GameVar*> gameVariables, 
		// And all sub-parts.
		integers, int64s,
		floats, strings,
		vectors, timeVars;
};



#endif