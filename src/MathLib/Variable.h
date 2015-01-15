/// Emil Hedemalm
/// 2014-08-22
/// A variable in an expression or function.

#ifndef VARIABLE_H
#define VARIABLE_H

#include "String/AEString.h"
#include "MathLib.h"

class Variable
{
public:
	Variable();
	/// As the variable's type may not be explicitly defined upon loading, let it remain undefined until it has to be defined.
	Variable(String name);
	Variable(String name, int iValue);
	Variable(String name, float fValue);
	Variable(String name, Vector3f vec3f);
	Variable(String name, Vector4f vec4f);
	
	int Type() const { return type; };
	/// Prints string. Returns type-string too.
	String PrintType();	
	/// 
	String name;
	
	/// See DataTypes.h
	int type;

	/// Some common variable data types.
	float fValue;
	int iValue;
	Vector3f vec3fValue;
	Vector4f vec4fValue;

};

/// o.o prepends the names of all variables in the list.
void PrependVariables(List<Variable> & list, String withString);


#endif

