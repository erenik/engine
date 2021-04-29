/// Emil Hedemalm
/// 2014-08-22
/// An expression resulting in some constant or other simplified expression.
/// http://en.wikipedia.org/wiki/Expression_%28mathematics%29

#include "Expression.h"
#include "Constants.h"
#include <cmath>
#include "String/StringUtil.h"
#include "Function.h"
#include "FunctionEvaluator.h"

bool Expression::printExpressionSymbols = false;

// Bad typ defualt constructor.
ExpressionResult::ExpressionResult(int type)
	: type(type)
{
}

ExpressionResult ExpressionResult::Boolean(bool value)
{
	ExpressionResult exp(DataType::BOOLEAN);
	exp.iResult = value;
	exp.text = String(value);
	return exp;
}
ExpressionResult ExpressionResult::Error(String text)
{
	ExpressionResult exp(DataType::NO_TYPE);
	exp.text = text;
	exp.iResult = 0;
	return exp;
}
ExpressionResult ExpressionResult::Integral(int value)
{
	ExpressionResult exp(DataType::INTEGER);
	exp.iResult = value;
	exp.text = String(value);
	return exp;
}
ExpressionResult ExpressionResult::Float(float value)
{
	ExpressionResult exp(DataType::FLOAT);
	exp.fResult = value;
	exp.text = String(value);
	return exp;
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
/// Returns the result as a bool. Non-0 will be converted to true, 0 false, if non-boolean.
bool ExpressionResult::GetBool()
{
	return GetFloat() != 0;
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
	String alpha, numeric, quote;
	bool lastWasOperator = false;
	bool inQuote = false;
	for (int i = 0; i < exp.Length(); ++i)
	{
		char c = exp.c_str()[i];
		if (inQuote)
		{
			if (c == '\"')
			{
				symbols.AddItem(Symbol(quote, Symbol::STRING));
				quote = "";
				inQuote = false;
				continue;
			}
			quote += c;
			continue;
		}
		switch(c)
		{
			case ' ':
			case '\t':
				continue;
			case '\"':
				inQuote = true;
				continue;
		}

		char c2;
		if (i < exp.Length() - 1)
			c2 = exp.c_str()[i+1];
		String c12 = String(c) + c2;
		
		/// Alphabetics.
		if (isalpha(c) || c == '_')
		{
			alpha += c;
		}
		else if (alpha.Length())
		{
			// It should be a variable then!... or function name!
			symbols.Add(Symbol(alpha, IsFunction(alpha)? Symbol::FUNCTION_NAME : Symbol::VARIABLE));
			// Reset string
			alpha = String();
		}
		/// Numerics
		if (isdigit(c) || c == '.')
		{
			numeric += c;
		}
		else if (numeric.Length())
		{
			symbols.Add(Symbol(numeric, Symbol::CONSTANT));
			lastWasOperator = false;
			numeric = String();
		}
		/// Single-character stuffs.
		switch(c)
		{
			case '-':
			case '+':
				if (lastWasOperator && isdigit(c2))
				{
					// Catenate as if alphabetical
					numeric += c;
					break;
				}
			case '/':
			case '*':
			case '^':
			case '%':
				symbols.Add(Symbol(c, Symbol::OPERATOR));
				lastWasOperator = true;
				break;
			case '<':
			case '>':
				if (c2 == '=')
				{
					symbols.Add(Symbol(c12, Symbol::OPERATOR));
					lastWasOperator = true;
					++i;
				}
				else {
					symbols.Add(Symbol(c, Symbol::OPERATOR));
					lastWasOperator = true;
				}
				break;
			case '|':
				if (c2 == '|')
				{
					symbols.Add(Symbol(c12, Symbol::OPERATOR));
					lastWasOperator = true;
					++i;
				}
				break;
			case '&':
				if (c2 == '&')
				{
					symbols.AddItem(Symbol(c12, Symbol::OPERATOR));
					lastWasOperator = true;
					++i;
				}
			case '!':
			case '=':
				if (c2 == '=')
				{
					symbols.Add(Symbol(c12, Symbol::OPERATOR));
					lastWasOperator = true;
					++i;
				}
				break;
			case '(':
				symbols.Add(Symbol(c, Symbol::BEGIN_PARENTHESIS));
				lastWasOperator = true;
				break;
			case ')':
				symbols.Add(Symbol(c, Symbol::END_PARENTHESIS));
				break;
			case ',':
				symbols.Add(Symbol(c, Symbol::ARGUMENT_ENUMERATOR));
				lastWasOperator = true;
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

bool Expression::IsFunction(String symbolName)
{
	for (int i = 0; i < functionEvaluators.Size(); ++i)
	{
		FunctionEvaluator * fe = functionEvaluators[i];
		if (fe->IsFunction(symbolName))
			return true;
	}
	return false;
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

/// Evaluates target variable. Expects a variable name and write the value back in the same string. Returns false if it doesn't exist, setting the error string appropriately.
bool Expression::EvaluateVariable(String & inputOutputString)
{
	String varName = inputOutputString;
	Variable * var = GetVariableForName(varName);
	if (!var)
	{
		result.text = "Undefined variable \'"+varName+"\'";
		return false;
	}
	switch(var->type)
	{
		case DataType::FLOAT:	inputOutputString = String(var->fValue);	break;
		case DataType::INTEGER:	inputOutputString = String(var->iValue);	break;
		default:
		{
			result.text = "Undefined variable data type: "+var->type;
			assert(false);
			return false;
		}
	}
	return true;
}


/// Tries to evaluate the expression.
bool Expression::TryEvaluate()
{
	if (symbols.Size() == 0)
		return true;
	/// Make a copy of the original expression, as we will modify it.
	evaluationSymbols = symbols;
	List<Expression> parenthesisExpressions;
	
	/// Search for parenthesis.
	int parenthesisStart = -1;
	int parenthesis = 0;
	int argumentStart = -1;
	bool argumentEnumerating = false;

	if (printExpressionSymbols)
		PrintSymbolsInALine(evaluationSymbols);
//	PrintSymbols(evaluationSymbols);

	/// All arguments to current function being evaluated on parenthesis-level 0.
	List<List<Symbol>> argumentExpressions; // Each argument can be a list of symbol (e.g. 3 arguments all of which calling functions or having regular mathematical expressions).
	String functionName;
	bool inFunction = false;
	/// Evaluate functions and parenthesis recursively (checking all parenthesis and separating argument operators ',')
	for (int i = 0; i < evaluationSymbols.Size(); ++i)
	{
		Symbol & evalSymbol = evaluationSymbols[i];
		switch(evalSymbol.type)
		{
			case Symbol::FUNCTION_NAME:
			{
				/// Only check for function name on base-level.
				if (parenthesis == 0)
				{
					inFunction = true;
					functionName = evalSymbol.text;
				}
				break;
			}
			case Symbol::BEGIN_PARENTHESIS:
				++parenthesis;
				// Store start of the parenthesis.
				if (parenthesis == 1)
				{
					argumentExpressions.Clear(); // Delete old arguments.
					parenthesisStart = i;
					argumentStart = i;
				}
				break;
			case Symbol::ARGUMENT_ENUMERATOR:
			{
				// Demand a parenthesis!
				if (parenthesis <= 0)
				{	
					result.text = "Unexpected argument enumerator ',' outside a parenthesis";
					return false;
				}
				/// Only evaluate 1 level up? (inside the parenthesis, yes)
				if (parenthesis == 1)
				{
					/// Check that we are parsing function arguments (name should have been identified)
					if (!inFunction)
					{
						result.text = "Unexpected arguments outside function.";
						return false;
					}
					/// Add symbols in current argument to the list of arguments!
					argumentExpressions.AddItem(evaluationSymbols.Part(argumentStart + 1, i - 1));
					/// Store the current position as argument start again for the next argument's parse to work out well.
					argumentStart = i;
				}
				break;
			}
			case Symbol::END_PARENTHESIS:
				--parenthesis;
				if (parenthesis == 0)
				{
					/// Remove and store the parenthesis as an argument-list instead of evaluating it as an expression.
					if (inFunction)
					{
						// If at "ground"-level, store the symbols within as a parenthesis expression to be evaluated first.
						argumentExpressions.AddItem(evaluationSymbols.Part(argumentStart + 1, i - 1));
						/// Evaluate each argument (list of symbols) separately.
						List<String> finalArgs;
						for (int j = 0; j < argumentExpressions.Size(); ++j)
						{
							List<Symbol> argExp = argumentExpressions[j];
						//	PrintSymbols(argExp);
							// Evaluate the symbols.
							Expression argumentExp(argExp);
							argumentExp.functionEvaluators = functionEvaluators;
							argumentExp.knownVariables = knownVariables;
							result = argumentExp.Evaluate();
							if (result.type == DataType::NO_TYPE)
							{
								return false;
							}
							finalArgs.AddItem(result.text);
						}
						/// Call the function.
						result = EvaluateFunction(functionName, finalArgs);
						if (result.type == DataType::NO_TYPE)
						{ // Result already logged in function call if bad.
					//		result.text = "Bad data-type in result when calling function "+functionName;
							return false;
						}

						/// Already, function done, replace all symbols used thus far with the resulting constant.
						int symbolsRemoved = evaluationSymbols.RemovePart(parenthesisStart - 1, i);
						Symbol newSym(result);
						evaluationSymbols.Insert(newSym, parenthesisStart - 1);
					//	PrintSymbols(evaluationSymbols);

						/// Move back i so that parsing will work out as intended?
						i -= symbolsRemoved - 1;
						/// Store the current position as argument start again for the next argument's parse to work out well.
						argumentStart = i;
						inFunction = false;
						break;
					}
					/// Non-function evaluation of the contents of a parenthesis!
					else {
						Expression pe(evaluationSymbols.Part(parenthesisStart + 1, i - 1));
						pe.functionEvaluators = this->functionEvaluators;
						// Transfer the known variables too.
						pe.knownVariables = knownVariables;
						bool ok = pe.TryEvaluate();
						if (!ok)
						{
							this->result.text = pe.result.text;
							return false;
						}
						/// Evaluation good, now check if the text before the parenthesis was a function or not.
						Symbol beforeParenthesis;
						if (parenthesisStart > 0)
							beforeParenthesis = evaluationSymbols[parenthesisStart - 1];
						/// Not a function before, so just replace the parenthesis with the result then.
						// Remove all the evaluation symbols that were part of the parenthesis and replace them with the result.
						evaluationSymbols.RemovePart(parenthesisStart, i);
						// Insert the new parenthesis-result-symbol. where the parenthesis was.
						Symbol sym(pe.result.text, Symbol::CONSTANT);
						evaluationSymbols.Insert(sym, parenthesisStart);						
						// Move back i so that parsing will work out as intended for the next parenthesis.
						// This since we removed several symbols. We need to adjust i to remain in the same relative location to the parenthesis we just evaluated.
						int symbolsRemoved = i - parenthesisStart + 1;
						i -= symbolsRemoved;
					}
				}
				else if (argumentEnumerating)
				{
					std::cout<<"\ncpbrn";
				}
				break;
		}
	}
	// All parenthesis should have been evaluated recursively above.
	
	/// Now we should only have constants left...
	// Convert variables to constants.
	for (int i = 0; i < evaluationSymbols.Size(); ++i)
	{
		Symbol & sym = evaluationSymbols[i];
		switch(sym.type)
		{
			// Variables handle exactly as constants, except that the values have to be extracted before-hand.
			case Symbol::VARIABLE:
			{
				// Extract the variable via its name!
				if (!EvaluateVariable(sym.text))
					return false;
				// Should have been converted to constant.
				sym.type = Symbol::CONSTANT;
			}
		}
	}

	bool ok = true;
	if (evaluationSymbols.Size() > 1)
		ok = EvaluateOperation();
	
	/// Final evaluation to result.
	/// Ensure we end up with just 1 final symbol.
	assert(evaluationSymbols.Size() == 1);
	if (evaluationSymbols.Size() > 1)
		PrintSymbols(evaluationSymbols);

	Symbol sym = evaluationSymbols[0];
	// Store text from the latest var2 as the final answer in the expression!
	result.text = sym.text;
	switch(sym.type)
	{
		case Symbol::CONSTANT:
			if (sym.text.Contains("."))
			{
				result.type = DataType::FLOAT;
				result.fResult = result.text.ParseFloat();
			}
			else {
				result.type = DataType::INTEGER;
				result.iResult = result.text.ParseInt();
			}
			break;
		case Symbol::STRING:
			result.type = sym.type;
			break;
		case Symbol::OPERATOR:
			return ok;
		default:
			assert(false);
	}
	return ok;
}

/// Evaluates operation between 1-2 symbols and 1 operator.
bool Expression::EvaluateOperation()
{
	/// Evaluate the operators, starting with those of highest priority.
	int priority = 5;
	/** 
		0	==, !=, >=, >, <, <=, ||
		1	+ -
		2	/ * %
		3	^
	*/
//	PrintSymbols(evaluationSymbols);
	while (priority >= 0)
	{
		int opSymIndex = -1;
		for (int i = 0; i < evaluationSymbols.Size(); ++i)
		{
			Symbol & opSym = evaluationSymbols[i];
			/// Skip non-operators.
			switch(opSym.type)
			{
				case Symbol::OPERATOR:
					break;
				default:
					continue;
			}			
			char cop = opSym.text.c_str()[0];
			/// Check operator priority, evaluate only the current priority level.
			bool correctPrioLevel = false;
			switch(priority)
			{
				case 0:
				{
					String str = opSym.text;
					if (str == "||" || str == "&&")
						correctPrioLevel = true;
					break;
				}
				case 1: 
				{
					String str = opSym.text;
					if (str == "==" || str == ">=" || str == "<=" || str == "!=" ||
						str == "<" || str == ">")
						correctPrioLevel = true;
					break;
				}
				case 2:
					switch(cop)
					{
						case '-':
						case '+':
							correctPrioLevel = true;
							break;
					}
					break;
				case 3:
					switch(cop)
					{
						case '*':
						case '/':
						case '%':
							correctPrioLevel = true;
							break;
					}
					break;
				case 4:
					switch(cop)
					{
						case '^':
							correctPrioLevel = true;
							break;
					}
					break;
				default:
					continue;
			}
			if (!correctPrioLevel)
				continue;
			assert(i < evaluationSymbols.Size() - 1);
			assert(i >= 0);

			Symbol * pre, * post;
			/// Probably a minus or plus should be embedded to the number.
			if (evaluationSymbols.Size() >= 3)
			{
				pre = &evaluationSymbols[i-1];
				post = &evaluationSymbols[i+1];
				assert(pre->type == Symbol::CONSTANT);
				assert(post->type == Symbol::CONSTANT);
			}
			// Unary operators?
			else 
			{
				pre = NULL;
				post = &evaluationSymbols[i + 1];	
			}

			// Evaluate the expression part using the given operator.
			Symbol symbol = Evaluate(pre, post, opSym);
			// Remove the operator and operands.
			evaluationSymbols.RemovePart(pre? i-1 : i, i+1);
			/// Insert the new result symbol in its place.
			evaluationSymbols.Insert(symbol, pre? i-1 : i);
			/// Move back i to properly evaluate the rest.
			i -= 1;
		}
		--priority;
	}
	return true;
}

/// Evaluates function if possible.
ExpressionResult Expression::EvaluateFunction(String functionName, List<String> arguments)
{
	ExpressionResult result; 
	for (int i = 0; i < functionEvaluators.Size(); ++i)
	{
		FunctionEvaluator * fe = functionEvaluators[i];
		bool ok = fe->EvaluateFunction(functionName, arguments, result);
		if (ok)
			return result;
	}
	result.text = "Unable to evaluate function by name "+functionName;
	result.type = DataType::NO_TYPE;
	return result;
}


/// Evaluates a binary expression-part of the following format:  sym1 op sym2
Symbol Expression::Evaluate(Symbol * sym1, Symbol * sym2, Symbol op)
{
//	std::cout<<"\nEvaluating: "<<(sym1? sym1->text : "") <<" "<<op.text<<" "<<(sym2? sym2->text : "");
	char cop = op.text.c_str()[0];
	char c2 = -1;
	if (op.text.Length() > 1)
		c2 = op.text.c_str()[1];
	
	// Special case unary operators affecting only sym2.
	if (sym1 == NULL)
	{
		Symbol result;
		result.text = String(sym2->text.ParseFloat() * -1);
		result.type = Symbol::CONSTANT;
		return result;
	}


	assert(sym1->type == Symbol::CONSTANT && sym2->type == Symbol::CONSTANT);

	// Use whatever format is best for them.
	String totalOperands = sym1->text + " " + sym2->text;
	/// Assume Int as lowest type?
	int typeNeeded = DataType::INTEGER;
	if (totalOperands.Contains("."))
		typeNeeded = DataType::FLOAT; // Could use double too.

	// Depending on type, convert it back to a string.
	Symbol resultSym;
	float f1, f2, fres;
	int i1, i2, ires;
	switch(typeNeeded)
	{
		case DataType::FLOAT:
			f1 = sym1->text.ParseFloat();
			f2 = sym2->text.ParseFloat();
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
			resultSym.text = String(fres);
			break;
		case DataType::INTEGER:
			i1 = sym1->text.ParseInt();
			i2 = sym2->text.ParseInt();
			switch(cop)
			{
				case '+':	ires = i1 + i2;	break;
				case '-':	ires = i1 - i2;	break;
				case '/':	ires = i1 / i2; break;
				case '*':	ires = i1 * i2;	break;
				case '^':	ires = pow((float)i1, (float)i2); break;
				case '%':	ires = i1 % i2; break;
				case '>': // Boolean operators.
					if (c2 == '=')
						ires = i1 >= i2;
					else
						ires = i1 > i2; 
					break;
				case '<':
					if (c2 == '=')
						ires = i1 <= i2;
					else
						ires = i1 < i2;
					break;
				case '=':
					if (c2 == '=')
						ires = (i1 == i2);
					break;
				case '!':
					if (c2 == '=')
						ires = (i1 != i2);
					break;
				case '|':
					if (c2 == '|')
						ires = i1 || i2;
					break;
				case '&':
					if (c2 == '&')
						ires = i1 && i2;
					break;
				default:
					assert(false);
			}
			resultSym.text = String(ires);
			break;
		default:
			assert(false);
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



