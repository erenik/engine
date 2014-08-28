/// Emil Hedemalm
/// 2014-08-22
/// A variable in an expression or function.

#ifndef VARIABLE_H
#define VARIABLE_H

#include "String/AEString.h"

class Variable
{
public:
	Variable();
	/// As the variable's type may not be explicitly defined upon loading, let it remain undefined until it has to be defined.
	Variable(String name);
	Variable(String name, int iValue);
	Variable(String name, float fValue);
	
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

};



#endif

