/// Emil Hedemalm
/// 2014-08-22
/// Multi-purpose basic function class.

#ifndef FUNCTION_H
#define FUNCTION_H

#include "MathLib.h"
#include "String/AEString.h"
#include "Variable.h"
#include "Expression.h"

#define Equation Function

class Function 
{
public:
	Function();
	/// Returns false if no symbols are presented or if there are semantic errors.
	bool Good();
	
	/** Loads functions to be used from target file. Each row is considered one function usually.
		Returns amount of functions loaded.
	*/
	static int LoadFunctions(String fromFile);

	/** Functions from text. 
		A regular expression is expected in the form of "a = bx + c" or "f(x) = ax + m".

		Returns false if there exists any errors in the function, i.e. semantic errors such as lacking parenthesis.
		Loaded functions are stored within the loadedFunctions list.
	*/
	bool LoadFunction(String fromText);
	/// Fetches a function by name. The name is defined as the part to the left of the equals-sign.
	static Function GetFunctionByName(String name);

	/// Evaluates the function with no additional arguments.
	ExpressionResult Evaluate();
	/// Evaluates the function with given variable arguments.
	ExpressionResult Evaluate(List<Variable> args);

	/// Static simple functions such as Max, Min and Random
	static ExpressionResult Evaluate(String functionName, List<Symbol> arguments, List<Variable> knownVariables);

	/// E.g. "f(x)" in the case of "f(x) = ax + m".
	String returnName;

	/// List of loaded functions.
	static List<Function> loadedFunctions;
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
