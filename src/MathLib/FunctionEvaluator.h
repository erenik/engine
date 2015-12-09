/// Emil Hedemalm
/// 2015-12-08
/// Function evaluator for arbitrary mathematical/programmatical expressions (extending to Scripts later)

#ifndef FUNCTION_EVALUATOR_H
#define FUNCTION_EVALUATOR_H

#include "Expression.h"

/// Sub-class to handle things.
class FunctionEvaluator
{
public:
	virtual bool EvaluateFunction(String byName, List<String> arguments, ExpressionResult & result) = 0;
	virtual bool IsFunction(String name) = 0;
};

/**	Enables the following functions in expressions (and in extension Scripts):
		Random(min,max)		- Randomizes (floating point) between min and max.
		abs(val)			- Returns absolute value
*/
class DefaultMathFunctionEvaluator : public FunctionEvaluator
{
public:
	virtual bool EvaluateFunction(String byName, List<String> arguments, ExpressionResult & result);
	virtual bool IsFunction(String name);
};
extern DefaultMathFunctionEvaluator defMatFuncEval;

#endif
