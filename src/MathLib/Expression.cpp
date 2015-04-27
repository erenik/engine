/// Emil Hedemalm
/// 2014-08-22
/// An expression resulting in some constant or other simplified expression.
/// http://en.wikipedia.org/wiki/Expression_%28mathematics%29

#include "Expression.h"
#include "Constants.h"
#include <cmath>
#include "String/StringUtil.h"
#include "Function.h"

bool Expression::printExpressionSymbols = false;

// Bad typ defualt constructor.
ExpressionResult::ExpressionResult(int type)
	: type(type)
{
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
		if (isdigit(c) || c == '.')
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
			case ',':
				symbols.Add(Symbol(c, Symbol::ARGUMENT_ENUMERATOR));
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
	/// Make a copy of the original expression, as we will modify it.
	List<Symbol> evaluationSymbols = symbols;
	List<Expression> parenthesisExpressions;
	
	/// Search for parenthesis.
	int parenthesisStart = -1;
	int parenthesis = 0;
	int argumentStart = -1;
	bool argumentEnumerating = false;

	if (printExpressionSymbols)
		PrintSymbolsInALine(evaluationSymbols);
//	PrintSymbols(evaluationSymbols);

	for (int i = 0; i < evaluationSymbols.Size(); ++i)
	{
		Symbol & evalSymbol = evaluationSymbols[i];
		switch(evalSymbol.type)
		{
			case Symbol::BEGIN_PARENTHESIS:
				++parenthesis;
				// Store start of the parenthesis.
				if (parenthesis == 1)
				{
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
				/// Only evaluate 1 level up?
				if (parenthesis == 1)
				{
					// Comma encountered? Then we are definitely enumerating arguments.
					argumentEnumerating = true;
					/// Store the first part within a first parenthesis, and evaluate it!
					Expression pe(evaluationSymbols.Part(argumentStart + 1, i - 1));
					// Transfer the known variables too.
					pe.knownVariables = knownVariables;
					bool ok = pe.TryEvaluate();
					if (!ok)
					{
						this->result.text = pe.result.text;
						return false;
					}

//					PrintSymbols(evaluationSymbols);
					// Remove all the evaluation symbols that were part of the parenthesis and replace them with the result.
					evaluationSymbols.RemovePart(argumentStart + 1, i - 1);
//					PrintSymbols(evaluationSymbols);
					
					// Insert the new parenthesis-result-symbol. where the parenthesis was.
					Symbol sym(pe.result.text, Symbol::CONSTANT);
					/// Place it back right after the argument start token was (the first parenthesis or comma)
					evaluationSymbols.Insert(sym, argumentStart + 1);

//					PrintSymbolsInALine(evaluationSymbols);
						
					/// Move back i so that parsing will work out as intended?
					i -= pe.symbols.Size() - 1;
					/// Store the current position as argument start again for the next argument's parse to work out well.
					argumentStart = i;
				}
				break;
			}
			case Symbol::END_PARENTHESIS:
				--parenthesis;
				// If at "ground"-level, store the symbols within as a parenthesis expression to be evaluated first.
				if (parenthesis == 0)
				{
					/// Remove and store the parenthesis as an argument-list instead of evaluating it as an expression.
					if (argumentEnumerating)
					{
						/// Non-function evaluation of the contents of a parenthesis!
						Expression pe(evaluationSymbols.Part(parenthesisStart, i));

					// Remove all the evaluation symbols that were part of the parenthesis and replace them with the result.
//						PrintSymbols(evaluationSymbols);
						evaluationSymbols.RemovePart(parenthesisStart, i);
//						PrintSymbols(evaluationSymbols);
						List<Symbol> argSymbols;
						Symbol argSymbol;

						/// Evaluated arguments.
						List<Symbol> args;

						/// Evaluate the symbols detected within the parenthesis before merging them into a Function-Arguments symbol.
						int parenthesisCounter2 = 0;
						for (int j = 0; j < pe.symbols.Size(); ++j)
						{
							Symbol & argSym = pe.symbols[j];
							switch(argSym.type)
							{
								case Symbol::BEGIN_PARENTHESIS:
									if (parenthesisCounter2 > 0)
										argSymbols.Add(argSym);
									parenthesisCounter2++;
									break;
								case Symbol::END_PARENTHESIS:
									parenthesisCounter2--;		
									if (parenthesisCounter2 > 0)
										argSymbols.Add(argSym);
									if (parenthesisCounter2 != 0)
										break;
								case Symbol::ARGUMENT_ENUMERATOR:
								{
									// Evaluate the symbols.
									Expression argumentExp(argSymbols);
									ExpressionResult expRes = argumentExp.Evaluate(knownVariables);
									expRes.type != -1;
									argSymbol = expRes;

									args.Add(argSymbol);
									argSymbols.Clear();
									break;
								}
								default:
									argSymbols.Add(argSym);
									break;
							}
						}

						Symbol sym(args, Symbol::FUNCTION_ARGUMENTS);
						sym.text = "Args: "+String(args.Size());
						// Insert the new parenthesis-result-symbol. where the parenthesis was.
						evaluationSymbols.Insert(sym, parenthesisStart);
				//		PrintSymbols(evaluationSymbols);

						/// Move back i so that parsing will work out as intended?
						i -= pe.symbols.Size() - 1;
						/// Store the current position as argument start again for the next argument's parse to work out well.
						argumentStart = i;

						break;
					}
					/// Non-function evaluation of the contents of a parenthesis!
					else {
						Expression pe(evaluationSymbols.Part(parenthesisStart + 1, i - 1));
						// Transfer the known variables too.
						pe.knownVariables = knownVariables;
						bool ok = pe.TryEvaluate();
						if (!ok)
						{
							this->result.text = pe.result.text;
							return false;
						}

						// Remove all the evaluation symbols that were part of the parenthesis and replace them with the result.
	//					PrintSymbols(evaluationSymbols);
						evaluationSymbols.RemovePart(parenthesisStart, i);
	//					PrintSymbols(evaluationSymbols);
						
						// Insert the new parenthesis-result-symbol. where the parenthesis was.
						Symbol sym(pe.result.text, Symbol::CONSTANT);
						evaluationSymbols.Insert(sym, parenthesisStart);
//						PrintSymbolsInALine(evaluationSymbols);

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
	
	/// Evaluate any functions which are present.
		/// Go from right to left, to make it easier to handle + and - signs?
	for (int i = evaluationSymbols.Size() - 1; i >= 0; --i)
	{
		Symbol & sym = evaluationSymbols[i];
		switch(sym.type)
		{
			case Symbol::FUNCTION_ARGUMENTS:
			{
				/// Check if function name is available before?
				if (i == 0)
				{
					result.text = "Function argument list requires preceding function name.";
					return false;
				}
				Symbol & preceding = evaluationSymbols[i-1];
				if (preceding.type != Symbol::VARIABLE)
				{
					result.text = "Function argument list requires preceding function name. Encountered \'"+preceding.text+"\'";
					return false;
				}
				/// Get function with that name then!
				ExpressionResult result = Function::Evaluate(preceding.text, sym.symbols, knownVariables);
				if (result.type == -1)
				{
					this->result.text = result.text;
					return false;
				}
//				PrintSymbols(evaluationSymbols);
				// Save the result and replace the function and arguments with the new symbol o.o
				evaluationSymbols.RemovePart(i-1, i);
				Symbol resultSymbol(result.text, Symbol::CONSTANT);
				evaluationSymbols.Insert(resultSymbol, i-1);
//				PrintSymbols(evaluationSymbols);
				break;
			}
		}
	}
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
	/// Evaluate the operators, starting with those of highest priority.
	int priority = 5;
	/** 0	+ -
		1	/ * %
		2	^
	*/
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
					switch(cop)
					{
						case '-':
						case '+':
							correctPrioLevel = true;
							break;
					}
					break;
				case 1:
					switch(cop)
					{
						case '*':
						case '/':
						case '%':
							correctPrioLevel = true;
							break;
					}
					break;
				case 2:
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

	/// Ensure we end up with just 1 final symbol.
	assert(evaluationSymbols.Size() == 1);

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
	}
	return true;
}

/// Evaluates a binary expression-part of the following format:  sym1 op sym2
Symbol Expression::Evaluate(Symbol * sym1, Symbol * sym2, Symbol op)
{
	char cop = op.text.c_str()[0];
	
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



