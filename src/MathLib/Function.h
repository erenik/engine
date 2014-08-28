/// Emil Hedemalm
/// 2014-08-22
/// Multi-purpose basic function class.

#ifndef FUNCTION_H
#define FUNCTION_H

#include "MathLib.h"
#include "String/AEString.h"
#include "Variable.h"
#include "Expression.h"

class Function 
{
public:
	Function();
	/** Functions from text. 
		A regular expression is expected in the form of "a = bx + c" or "f(x) = ax + m".

		Returns false if there exists any errors in the function, i.e. semantic errors such as lacking parenthesis.
	*/
	bool LoadFunction(String fromText);

	/// Evaluates the function with no additional arguments.
	ExpressionResult Evaluate();
	/// Evaluates the function with given variable arguments.
	ExpressionResult Evaluate(List<Variable> args);

	/// E.g. "f(x)" in the case of "f(x) = ax + m".
	String returnName;

private:
	
	/// If the expression is ok.
	bool ok;

	/// Names of all variables.
	List<String> variableNames;
	List<Variable> variables;

	/// Right-hand expression.
	Expression exp;
	// Right of equals-sign.
	String expression;

	/// String it is based upon.
	String text;
	/// If there is any error in the expressions of the function, this will be set here.
	String errorString;
};

#endif
