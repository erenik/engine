/// Emil Hedemalm
/// 2014-08-22
/// An expression resulting in some constant or other simplified expression.
/// http://en.wikipedia.org/wiki/Expression_%28mathematics%29

#include "Expression.h"
#include "Constants.h"
#include <cmath>

// Bad typ defualt constructor.
ExpressionResult::ExpressionResult(int type)
	: type(type)
{
	expressionResult = NULL;
}

/// Returns the result as a float.
float ExpressionResult::GetFloat()
{
	switch(type)
	{
		case DataType::FLOAT:
			return fResult;
		case DataType::INTEGER:
			return iResult;
	}
	return 0;
}


Expression::Expression()
{
	resultSymbol = NULL;
}

/// Creates and parses the expression as based on the provided text, making it ready to call Evaluate straight away.
Expression::Expression(String text)
{
	ParseExpression(text);
}
	
/// For creating parenthesis-based sub-expressions.
Expression::Expression(List<Symbol> symbols)
	: symbols(symbols)
{

}	

Expression::~Expression()
{
}

List<Variable> Expression::constantVariables;

/// Creates the constant symbols, such as PI, to be readily available for use in any expression.
void Expression::InitializeConstants()
{
	constantVariables.Add(Variable("PI", PI));
	constantVariables.Add(Variable("e", e));
}

/** Parses the given expression, creating Expression and Variable objects as needed in order to evaluate this function.
	If semantic errors are detected, it will return false.
*/
bool Expression::ParseExpression(String exp)
{
	/// Easiest parsing is probably a step-char-wise parse...
	String alpha, numeric;
	for (int i = 0; i < exp.Length(); ++i)
	{
		char c = exp.c_str()[i];
		
		/// Alphabetics.
		if (isalpha(c))
		{
			alpha += c;
		}
		else if (alpha.Length())
		{
			// It should be a variable then!
			symbols.Add(Symbol(alpha, Symbol::VARIABLE));
			// Reset string
			alpha = String();
		}
		/// Numerics
		if (isdigit(c) || c == '.' || c == ',')
		{
			numeric += c;
		}
		else if (numeric.Length())
		{
			symbols.Add(Symbol(numeric, Symbol::CONSTANT));
			numeric = String();
		}
		/// Single-character stuffs.
		switch(c)
		{
			case '-':
			case '/':
			case '*':
			case '+':
			case '^':
			case '%':
				symbols.Add(Symbol(c, Symbol::OPERATOR));
				break;
			case '(':
				symbols.Add(Symbol(c, Symbol::BEGIN_PARENTHESIS));
				break;
			case ')':
				symbols.Add(Symbol(c, Symbol::END_PARENTHESIS));
				break;
		};
	}
	/// Add final stuff if any was not added.
	if (alpha.Length())
	{
		// It should be a variable then!
		symbols.Add(Symbol(alpha, Symbol::VARIABLE));
		alpha = String();
	}
	if (numeric.Length())
	{
		symbols.Add(Symbol(numeric, Symbol::CONSTANT));
		numeric = String();
	}

	// Check for inconsistencies? ..

	return true;
}

/// Evaluating without any arguments
ExpressionResult Expression::Evaluate()
{
	TryEvaluate();
	return result;
}

/** Evaluating when variable states are required to be known. 
	Variables required may be queried with RequiredVariables().
*/
ExpressionResult Expression::Evaluate(List<Variable> variables)
{
	knownVariables = variables;
	TryEvaluate();
	return result;
}
	
/** Returns a list of names of all variables which are required to evaluate the expression.
	No distinction is made as to what type the variables may have. This should be up to the user.
*/
List<String> Expression::RequiredVariables()
{
	List<String> varNames;
	for (int i = 0; i < symbols.Size(); ++i)
	{
		Symbol & symbol = symbols[i];
		if (symbol.type == Symbol::VARIABLE)
		{
			if (!varNames.Exists(symbol.name))
				varNames.Add(symbol.name);
		}
	}
	return varNames;
}

/// Tries to evaluate the expression.
bool Expression::TryEvaluate()
{
	/// Make a copy of the original expression, as we will modify it.
	List<Symbol> evaluationSymbols = symbols;
	List<Expression> parenthesisExpressions;
	
	/// Search for parenthesis.
	int parenthesisStart = -1;
	int parenthesis = 0;
	for (int i = 0; i < evaluationSymbols.Size(); ++i)
	{
		Symbol & evalSymbol = evaluationSymbols[i];
		switch(evalSymbol.type)
		{
			case Symbol::BEGIN_PARENTHESIS:
				++parenthesis;
				// Store start of the parenthesis.
				if (parenthesis == 1)
					parenthesisStart = i;
				break;
			case Symbol::END_PARENTHESIS:
				--parenthesis;
				// If at "ground"-level, store the symbols within as a parenthesis expression to be evaluated first.
				if (parenthesis == 0)
				{
					Expression pe(evaluationSymbols.Part(parenthesisStart + 1, i - 1));
					// Transfer the known variables too.
					pe.knownVariables = knownVariables;
					bool ok = pe.TryEvaluate();
					if (!ok)
						return false;

					// Remove all the evaluation symbols that were part of the parenthesis and replace them with the result.
					evaluationSymbols.RemovePart(parenthesisStart, i);
					
					// Insert the new parenthesis-result-symbol. where the parenthesis was.
					Symbol sym(pe.result.text, Symbol::CONSTANT);
					evaluationSymbols.Insert(sym, parenthesisStart);
				}
				break;
		}
	}

	// All parenthesis should have been evaluated recursively above.

	// Now do regular evaluation..
	Symbol var1, var2;
	// Short for operator, since that is a keyword in C++ 
	Symbol op1, 
		// Special case with e.g. a * - b ?
		* op2;

	/// Go from right to left, to make it easier to handle + and - signs?
	for (int i = evaluationSymbols.Size() - 1; i >= 0; --i)
	{
		Symbol & sym = evaluationSymbols[i];
		switch(sym.type)
		{
			case Symbol::OPERATOR:
				if (op1.type == Symbol::BAD_TYPE)
					op1 = sym;
				break;

			// Variables handle exactly as constants, except that the values have to be extracted before-hand.
			case Symbol::VARIABLE:
			{
				// Extract the variable via its name!
				Variable * var = GetVariableForName(sym.text);
				if (!var)
					return false;
				switch(var->type)
				{
					case DataType::FLOAT:	sym.text = String(var->fValue);	break;
					case DataType::INTEGER:	sym.text = String(var->iValue);	break;
					default:
						assert(false);
						return false;
				}
				// Should have been converted to constant.
				sym.type = Symbol::CONSTANT;
			}
			case Symbol::CONSTANT:
				// Since we're reading backwards, set var2 first.
				if (var2.type == Symbol::BAD_TYPE)
					var2 = sym;
				else 
				{
					var1 = sym;
					// Evaluate the expression part using the given operator.
					Symbol symbol = Evaluate(var1, var2, op1);
					// Save the symbol as var2 for the next operation.
					var2 = symbol;
					// Reset var1 and op
					var1 = op1 = Symbol();
				}
				break;
		}
	}
	// Store text from the latest var2 as the final answer in the expression!
	result.text = var2.text;
	switch(var2.type)
	{
		case Symbol::CONSTANT:
			if (var2.text.Contains("."))
			{
				result.type = DataType::FLOAT;
				result.fResult = var2.text.ParseFloat();
			}
			else {
				result.type = DataType::INTEGER;
				result.iResult = var2.text.ParseInt();
			}
			break;
	}
	return true;
}

/// Evaluates a binary expression-part of the following format:  sym1 op sym2
Symbol Expression::Evaluate(Symbol sym1, Symbol sym2, Symbol op)
{
	char cop = op.text.c_str()[0];
	
	assert(sym1.type == Symbol::CONSTANT && sym2.type == Symbol::CONSTANT);

	// Use whatever format is best for them.
	String totalOperands = sym1.text + " " + sym2.text;
	/// Assume Int as lowest type?
	int typeNeeded = DataType::INTEGER;
	if (totalOperands.Contains("."))
		typeNeeded = DataType::FLOAT; // Could use double too.

	float f1, f2, fres;
	int i1, i2, ires;
	switch(typeNeeded)
	{
		case DataType::FLOAT:
			f1 = sym1.text.ParseFloat();
			f2 = sym2.text.ParseFloat();
			switch(cop)
			{
				case '+':	fres = f1 + f2;	break;
				case '-':	fres = f1 - f2;	break;
				case '/':	fres = f1 / f2; break;
				case '*':	fres = f1 * f2;	break;
				case '^':	fres = pow(f1, f2); break;
				default:
					assert(false);
			}
			break;
		case DataType::INTEGER:
			i1 = sym1.text.ParseInt();
			i2 = sym2.text.ParseInt();
			switch(cop)
			{
				case '+':	ires = i1 + i2;	break;
				case '-':	ires = i1 - i2;	break;
				case '/':	ires = i1 / i2; break;
				case '*':	ires = i1 * i2;	break;
				case '^':	ires = pow((float)i1, (float)i2); break;
				case '%':	ires = i1 % i2; break;
				default:
					assert(false);
			}
			break;
		default:
			assert(false);
	}

	// Depending on type, convert it back to a string.
	Symbol resultSym;
	switch(typeNeeded)
	{
		case DataType::FLOAT:
			resultSym.text = String(fres);
			break;
		case DataType::INTEGER:
			resultSym.text = String(ires);
			break;
	}
	resultSym.type = Symbol::CONSTANT;
	return resultSym;
}

/// Works with the knownVariables list.
Variable * Expression::GetVariableForName(String name)
{
	for (int i = 0; i < knownVariables.Size(); ++i)
	{
		Variable & var = knownVariables[i];
		var.name.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (var.name == name)
			return &var;
	}
	for (int i = 0; i < constantVariables.Size(); ++i)
	{
		Variable & var = constantVariables[i];
		var.name.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (var.name == name)
			return &var;
	}
	return NULL;
}



