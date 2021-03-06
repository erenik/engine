/// Emil Hedemalm
/// 2015-02-08 (much older originally)
/// Aeonic Games Math

#include "AEMath.h"
#include <cstdlib>
#include "Trigonometry.h"

float oneDivRandMaxFloat = 0;

MathLib mathLib;

MathLib::MathLib()
{
	Init();
}
MathLib::~MathLib()
{
	Free();
}

const float MathLib::OneDivRandMaxFloat() {
	return oneDivRandMaxFloat;
}


// Initializes the various numbers specified above.
void MathLib::Init()
{
	oneDivRandMaxFloat = 1.f / RAND_MAX;
	InitSampled360();
}

void MathLib::Free()
{
	DeallocSampled360();
}

