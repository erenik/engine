/// Emil Hedemalm
/// 2014-08-22
/// An expression resulting in some constant or other simplified expression.
/// http://en.wikipedia.org/wiki/Expression_%28mathematics%29

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "Symbol.h"
#include "Variable.h"
#include "DataTypes.h"

class Expression;

class ExpressionResult 
{
public:
	// Bad typ defualt constructor.
	ExpressionResult(int type = -1);
	/// Returns the result as a float.
	float GetFloat();
	/// Will be any of the defined enumerated constants in DataTypes.h
	int type;
	/// The result in text-form.
	String text;
	float fResult;
	int iResult;
};

class Expression 
{
public:
	Expression();
	/// Creates and parses the expression as based on the provided text, making it ready to call Evaluate straight away.
	Expression(String text);
	/// For creating parenthesis-based sub-expressions.
	Expression(List<Symbol> symbols);
	~Expression();

	/// Creates the constant symbols, such as PI, to be readily available for use in any expression.
	static void InitializeConstants();
	
	/** Parses the given expression, creating Expression and Variable objects as needed in order to evaluate this function.
		If semantic errors are detected, it will return false.
	*/
	bool ParseExpression(String exp);

	/// Evaluating without any arguments
	ExpressionResult Evaluate();
	/** Evaluating when variable states are required to be known. 
		Variables required may be queried with RequiredVariables().
	*/
	ExpressionResult Evaluate(List<Variable> variables);
	
	/** Returns a list of names of all variables which are required to evaluate the expression.
		No distinction is made as to what type the variables may have. This should be up to the user.
	*/
	List<String> RequiredVariables();

	/// An expression is basically just a bunch of symbols.
	List<Symbol> symbols;

	/// For debugging, may be set using PrintExpressionSymbols in OnEnter.ini or similar files which are loaded upon start-up.
	static bool printExpressionSymbols;
private:
	
	/// Evaluates target variable. Expects a variable name and write the value back in the same string. Returns false if it doesn't exist, setting the error string appropriately.
	bool EvaluateVariable(String & inputOutputString);

	/// Tries to evaluate the expression.
	bool TryEvaluate();
	/// Evaluates a binary expression-part of the following format:  sym1 op sym2
	Symbol Evaluate(Symbol sym1, Symbol sym2, Symbol op);
	/// Works with the knownVariables list.
	Variable * GetVariableForName(String name);

	/// o.o
	List<Variable> knownVariables;

	/// List of constant variables which are should be filled up once upon initialization with constants such as "pi".
	static List<Variable> constantVariables;

	/// Result of it all.
	ExpressionResult result;

	/** If set, this means this expression is a part of a larger expression.
		Upon finishing evaluation the result will be stored in the symbol as well as the returned ExpressionResult.
	*/
	Symbol * resultSymbol;
};

#endif



