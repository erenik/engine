/// Emil Hedemalm
/// 2014-08-22
/// Multi-purpose basic function class.

#include "Function.h"

Function::Function()
{

}

/** Functions from text. 
	A regular expression is expected in the form of "a = bx + c" or "f(x) = ax + m".

	Returns false if there exists any errors in the function, i.e. semantic errors such as lacking parenthesis.
*/
bool Function::LoadFunction(String fromText)
{
	// Assume a single line with one equals-sign for now.
	List<String> parts = fromText.Tokenize("=");
	// Left one will be return type.
	if (parts.Size() < 2)
	{
		std::cout<<"Lacking components on each side of =";
		return false;
	}

	returnName = parts[0];

	/// The second part will be the expression to be evaluated.
	String expressionString = parts[1];

	/// Parse the expression for symbols.
	ok = exp.ParseExpression(expressionString);
	/// Return if the parsing failed or not!
	return ok;
}


/// Evaluates the function with no additional arguments.
ExpressionResult Function::Evaluate()
{
	if (!ok)
		return -1;
	return exp.Evaluate();
}

/// Evaluates the function with given variable arguments.
ExpressionResult Function::Evaluate(List<Variable> args)
{
	if (!ok)
		return -1;
	return exp.Evaluate(args);
}




