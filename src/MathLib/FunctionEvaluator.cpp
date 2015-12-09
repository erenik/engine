/// Emil Hedemalm
/// 2015-12-08
/// Function evaluator for arbitrary mathematical/programmatical expressions (extending to Scripts later)

#include "FunctionEvaluator.h"
#include "Random/Random.h"

/**	Enables the following functions in expressions (and in extension Scripts):
		Random(min,max)		- Randomizes (floating point) between min and max.
		abs(val)			- Returns absolute value
*/
bool DefaultMathFunctionEvaluator::EvaluateFunction(String name, List<String> arguments, ExpressionResult & result)
{
	if (name == "Random")
	{
		static Random funcRand;
		float max = arguments[1].ParseFloat(), min = arguments[0].ParseFloat();
		result = ExpressionResult::Float(funcRand.Randf(max - min) + min);
		return true;
	}
	else if (name == "abs")
	{
		// Except 1 argument.
		result = ExpressionResult::Float(AbsoluteValue(arguments[0].ParseFloat()));
		return true;
	}
	return false;
}
bool DefaultMathFunctionEvaluator::IsFunction(String name)
{
	if (name == "Random" || name == "abs")
		return true;
	return false;
}
DefaultMathFunctionEvaluator defMatFuncEval;

