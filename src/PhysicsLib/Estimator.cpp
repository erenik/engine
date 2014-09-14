/// Emil Hedemalm
/// 2014-02-07
/// A class for estimating values using various methods, such as interpolation/extrapolation.

#include "Estimator.h"

EstimatorState::EstimatorState(int type)
	: type(type)
{
}

EstimatorState::EstimatorState(int type, int64 time)
: type(type), time(time)
{	
}


/// Sets finished to false.
Estimator::Estimator()
{
	finished = false;
	timeElapsedMs = 0;
}

