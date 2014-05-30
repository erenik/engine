// Emil Hedemalm
// 2013-06-29

#ifndef GAME_VARIABLES_H
#define GAME_VARIABLES_H

#include "MathLib.h"
#include <Util/Util.h>
#include "System/DataTypes.h"

#define GameVars (*GameVariableManager::Instance())

class GameVariable{
	friend class GameVariableManager;
	GameVariable(const GameVariable & ref);
public:
	GameVariable(String name, int type) : type(type), name(name) {};
	enum types {
		NULL_TYPE,
		INTEGER,
		INT64,
		FLOAT,
		STRING,
		VECTOR,
		CUSTOM,
	};
	int Type() const { return type; };
	void PrintType() {
		switch(type){
			case INTEGER: std::cout<<"Integer"; break;
			case FLOAT: std::cout<<"Float"; break;
			case STRING: std::cout<<"String"; break;
			case VECTOR: std::cout<<"Vector"; break;
			case CUSTOM: std::cout<<"Custom"; break;
			default: std::cout<<"Bad/no type"; break;
		}
	};
private:
	String name;
	int type;
};

class GameVariablei : public GameVariable{
	friend class GameVariableManager;
	GameVariablei(String name, int value = 0) : GameVariable(name, INTEGER), value(value){};
public:
	int Get() const { return value; };
	void Set(int v) { value = v; };
private:
	int value;
};

class GameVariablei64 : public GameVariable{
	friend class GameVariableManager;
	GameVariablei64(String name, int64 value = 0) : GameVariable(name, INT64), value(value){};
public:
	int64 Get() const { return value; };
	void Set(int64 v) { value = v; };
private:
	int64 value;
};

class GameVariablef : public GameVariable{
	friend class GameVariableManager;
	GameVariablef(String name, float value = 0) : GameVariable(name, FLOAT), value(value){};
public:
	float Get() const { return value; };
	void Set(float v) { value = v; };
private:
	float value;
};

class GameVariables : public GameVariable{
	friend class GameVariableManager;
	GameVariables(String name, String value = "") : GameVariable(name, STRING), value(value){};
public:
	String Get() const { return value; };
	void Set(String v) { value = v; };
private:
	String value;
};

class GameVariablefv : public GameVariable{
	friend class GameVariableManager;
	GameVariablefv(String name, Vector4f value = Vector4f(0,0,0,1)) : GameVariable(name, VECTOR), value(value){};
public:
	Vector4f Get() const { return value; };
	void Set(Vector4f v) { value = v; };
private:
	Vector4f value;
};

/// Manager for global game variables!
class GameVariableManager{
	GameVariableManager() {};
	~GameVariableManager() {};
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

	/// General getter for any variable.
	GameVariable * Get(String name);
	/// Gette for strings
	GameVariables * GetString(String stringName);
	/// Specific getters.
	int GetInt(String name);
	
	// SEttetrrrrr
	void SetInt(String name, int intValue);

	/// Creators, returns the varible. If it exists, the existing variable will be returned.
	GameVariablei * CreateInt(String name, int initialValue = 0);
	GameVariablei64 * CreateInt64(String name, long long initialValue = 0);

private:
	List<GameVariable*> gameVariables;
	List<GameVariablei*> integers;
	List<GameVariablei64*> int64s;
	List<GameVariablef*> floats;
	List<GameVariables*> strings;
	List<GameVariablefv*> vectors;
};



#endif