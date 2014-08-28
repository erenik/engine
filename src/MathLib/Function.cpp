/// Emil Hedemalm
/// 2014-08-22
/// Multi-purpose basic function class.

#include "Function.h"
#include "File/File.h"

List<Function> Function::loadedFunctions;

Function::Function()
{

}

/// Returns false if no symbols are presented or if there are semantic errors.
bool Function::Good()
{
	return exp.symbols.Size();
}

/** Loads functions to be used from target file. Each row is considered one function usually.
	Returns amount of functions loaded.
*/
int Function::LoadFunctions(String fromFile)
{
	List<String> lines = File::GetLines(fromFile);
	assert(lines.Size());
	int numLoaded = 0;
	for (int i = 0; i < lines.Size(); ++i)
	{
		Function func;
		String line = lines[i];
		// Assume a single line with one equals-sign for now.
		List<String> parts = line.Tokenize("=");
		// Left one will be return type.
		if (parts.Size() < 2)
		{
			std::cout<<"Lacking components on each side of =";
			continue;
		}

		func.returnName = parts[0];

		/// The second part will be the expression to be evaluated.
		String expressionString = parts[1];

		/// Parse the expression for symbols.
		func.ok = func.exp.ParseExpression(expressionString);
		if (func.ok)
		{
			/// Check if the name co-insides.
			for (int j = 0; j < loadedFunctions.Size(); ++j)
			{
				Function & func2 = loadedFunctions[j];
				if (func2.returnName == func.returnName)
				{
					loadedFunctions.RemoveIndex(j);
					--j;
				}
			}
			/// Add it to the list of loaded functions for future use.
			loadedFunctions.Add(func);
			++numLoaded;
		}
	}
	/// Return if the parsing failed or not!
	return numLoaded;
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
/// Fetches a function by name. The name is defined as the part to the left of the equals-sign.
Function Function::GetFunctionByName(String name)
{
	for (int i = 0; i < loadedFunctions.Size(); ++i)
	{
		Function & f = loadedFunctions[i];
		if (f.returnName.Contains(name))
			return f;
	}
	return Function();
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




