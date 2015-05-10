/// Emil Hedemalm
/// 2015-02-06
/// Any variable within e.g. a game. May be used for other purposes. These vars are usually saved on a per game-save basis.

#ifndef GAME_VAR_H
#define GAME_VAR_H

#include "MathLib.h"
#include <Util/Util.h>
#include "System/DataTypes.h"
// Short-name.
#define GameVar GameVariable

class GameVariable
{
	friend class GameVariableManager;
public:
	GameVariable(): type(NULL_TYPE){};
	GameVariable(String name, int type) : type(type), name(name) {};
	virtual ~GameVariable();
	enum types {
		NULL_TYPE,
		INTEGER,
		INT64,
		FLOAT,
		STRING,
		VECTOR,
		VECTOR_3F,
		VECTOR_4F,
		TIME,
		CUSTOM,
	};
	int Type() const { return type; };
	void PrintType();
	String ToString();
	/// Converts to int as needed.
	int GetInt();
	/// Sets value using integer input.
	void SetInt(int iValue);
	bool WriteTo(std::fstream & stream);
	bool ReadFrom(std::fstream & stream);
	// Data.
	int iValue;
	int64 i64Value;
	float fValue;
	Vector3f vec3fValue;
	Vector4f vec4fValue;
	String strValue;
	Time timeValue;
private:
	// Name and type may not be changed once created.
	String name;
	int type;
};

#endif

