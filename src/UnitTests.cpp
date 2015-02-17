/// Emil Hedemalm
/// 2015-02-17
/// Performs unit tests for whatever functions you want. Is run after the output window is created.
/// Return true if you want to exit the program after the tests.

#include "MathLib/Matrix4f.h"

extern bool TrigonometryTests();

bool UnitTests()
{

	if (TrigonometryTests())
		return true;

	Matrix4f::UnitTest();
//	Angle::UnitTest();

//	return 0;
	return false;
}


