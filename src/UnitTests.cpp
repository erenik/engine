/// Emil Hedemalm
/// 2015-02-17
/// Performs unit tests for whatever functions you want. Is run after the output AppWindow is created.
/// Return true if you want to exit the program after the tests.

#include "MathLib/Matrix4f.h"

extern bool TrigonometryTests();

#include "MathLib/Function.h"
#include "MathLib/FunctionEvaluator.h"

bool UnitTests()
{
	/*
	Expression exp;
	exp.functionEvaluators.AddItem(&defMatFuncEval);
	exp.ParseExpression("abs(abs(-7) * Random(-4,-1)) * 4");
	ExpressionResult expr = exp.Evaluate();
	String text = expr.text;
	return false;
	*/

	if (TrigonometryTests())
		return true;

	Matrix4f::UnitTest();
//	Angle::UnitTest();

//	return 0;
	return false;
}


