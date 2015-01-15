/// Emil Hedemalm
/// 2014-09-12
/// o.o

#include "EstimatorFloat.h"

/// Size of states-array from which we sample.
#define arraySize states.Size()

EstimationFloat::EstimationFloat()
	: Estimation(EstimatorType::FLOAT)
{
}

EstimationFloat::EstimationFloat(float fValue, int64 timeStamp)
	: Estimation(EstimatorType::FLOAT, timeStamp), value(fValue)
{
}


/// Constructor which sets this estimator up to write to a specific.
EstimatorFloat::EstimatorFloat()
{
	variableToPutResultTo = NULL;
	currentIndex = -1;
	
}

/** Estimates values for given time. If loop is true, the given time will be modulated to be within the interval of applicable time-values.
	If the estimator's output pointer is set, data for the given estimation will be written there accordingly.
*/
void EstimatorFloat::Estimate(int64 forGivenTimeInMs, bool loop)
{
	int64 modulatedTime = forGivenTimeInMs;
	if (loop)
		modulatedTime = forGivenTimeInMs % totalIntervalMs + minMs;
	
	Estimation * before, * after;
	float ratioBefore, ratioAfter;
	this->GetStates(before, after, ratioBefore, ratioAfter, modulatedTime);
//	std::cout<<"\nRelative: "<<relative<<" = "<<relativeTime<<" / "<<timeBetween<<", after->time:"<<after->time<<" before->time:"<<before->time
//		<<" Before/after index: "<<beforeIndex<<"/"<<afterIndex;

	/// Interpolated value
	float beforeF = (before? ((EstimationFloat *)before)->value * ratioBefore : 0);
	float afterF = (after? ((EstimationFloat *)after)->value * ratioAfter : 0);
	float finalValue =  beforeF + afterF;
//	std::cout<<"\nFinal value: "<<finalValue<<" before:"<<before<<" after:"<<after;
	if (finalValue > 10.f)
	{
	//	std::cout<<"\n Lall";
	}

	if (lastCalculation < finalValue)
	{
	//	std::cout<<" lastCalc "<<lastCalculation;
	}
	
//	std::cout<<"\nFinal value: "<<finalValue;

	if (variableToPutResultTo)
		*variableToPutResultTo = finalValue;
	else {
		std::cout<<"\nEstimating with no output result placement.";
	}

	lastCalculation = finalValue;
}	

/// Process, i.e. increase recorded duration and evaluate the floating point value at the new time point.
void EstimatorFloat::Process(int timeInMs)
{
	if (finished)
		return;
	/// Increment elapsed time.
	timeElapsedMs += timeInMs;

	// Estimate value! Final value will be written as necessary straight from the Estimate function.
	this->Estimate(timeElapsedMs, loop);

	/// Mark as finished when applicable.
	int64 time = states.Last()->time;
	if (timeElapsedMs > time)
		finished = true;
	
	/*

	float value;
	int mode = EstimationMode::INTERPOLATION;
	switch(mode){
		// Just grab latest value.
		case EstimationMode::NONE:
		{
			if (currentIndex >= 0)
				value = states[currentIndex].data;
			else if (hasLooped)
				value = states[arraySize-1].data;
			else
				value = Vector3f();
			break;
		}
		// Smooth between one to two values.
		case EstimationMode::INTERPOLATION: 
		{	
			/// Check if the time is more recent than any values within, if so return the latest time straight away?
			value = GetInterpolatedValue(timeElapsedMs);	
			break;
		}
		/// Use previous values to estimate current velocity, position, etc. to estimate for current-time!
		case EstimationMode::EXTRAPOLATION: 
		case EstimationMode::EXTRAPOLATION_PLUS_COLLISION_CORRECTION:
		{
			bool good;
			value = GetExtrapolatedValue(forGivenTime, good);
			if (!good){
				/// If not good time, just fetch latest input value.
				value = GetState(0)->data;
			}
			break;
		}
		case EstimationMode::INTER_PLUS_EXTRA:
		{
			bool good;
			value = GetExtrapolatedValue(forGivenTime, good);
			if (!good){
				value = GetInterpolatedValue(forGivenTime);
			}

			break;
		}
	}			

*/
}

/// For adding any length of states as arguments.
void EstimatorFloat::AddStatesMs(int numStates, float f1, int timeStamp1, float f2, int timeStamp2, ...)
{
	AddStateMs(f1, timeStamp1);
	AddStateMs(f2, timeStamp2);

	// Variable length arguments.
	// http://www.cprogramming.com/tutorial/c/lesson17.html
	/* Initializing arguments to store all values after num */
	va_list argptr;    
	int extraStates = numStates - 2;
    va_start ( argptr, timeStamp2);
    // Sum all the inputs; we still rely on the function caller to tell us how many there are.
    for (int x = 0; x < extraStates; x++ )        
    {
		/// Has to be double, as floats are not supported in stdarg, see http://stackoverflow.com/questions/11270588/variadic-function-va-arg-doesnt-work-with-float
		float fx = (float) va_arg (argptr, double);
		int timeStampx = (int) va_arg (argptr, int);
	//	std::cout<<"\nfx"<<fx<<" timestamp"<<timeStampx;
		AddStateMs(fx, timeStampx);
    }
    va_end ( argptr );      // Cleans up the list
}	

/// For adding any length of states as arguments. timestamps in seconds.
void EstimatorFloat::AddStatesSeconds(int states, float f1, float timeStamp1, float f2, float timeStamp2, ...)
{
	AddStateMs(f1, timeStamp1 * 1000);
	AddStateMs(f2, timeStamp2 * 1000);

	// Variable length arguments.
	// http://www.cprogramming.com/tutorial/c/lesson17.html
	/* Initializing arguments to store all values after num */
	va_list argptr;    
	int extraStates = states - 2;
    va_start ( argptr, timeStamp2);
    // Sum all the inputs; we still rely on the function caller to tell us how many there are.
    for (int x = 0; x < extraStates; x++ )        
    {
		/// Has to be double, as floats are not supported in stdarg, see http://stackoverflow.com/questions/11270588/variadic-function-va-arg-doesnt-work-with-float
		float fx = (float) va_arg (argptr, double);
		float timeStampx = (float) va_arg (argptr, double);
	//	std::cout<<"\nfx"<<fx<<" timestamp"<<timeStampx;
		int64 millis = timeStampx * 1000;
		AddStateMs(fx, millis);
    }
    va_end ( argptr );      // Cleans up the list

}


/// o.o
void EstimatorFloat::AddStateMs(float fValue, int64 timeStampInMs)
{
	// Set lastCalculation to the initially added value!
	if (states.Size() == 0)
		lastCalculation = fValue;

	EstimationFloat * newState = new EstimationFloat(fValue, timeStampInMs);
	Estimator::AddState(newState);
}

/// Inserts the state at a decent location based on the time-stamp.
void EstimatorFloat::InsertState(float f, int64 timeStampInMs)
{
	EstimationFloat * newState = new EstimationFloat(f, timeStampInMs);
	Estimator::InsertState(newState);
}

/// Fetches interpolated value for given time-stamp.
float EstimatorFloat::GetInterpolatedValue(int64 forGivenTimeInMs)
{
	Estimation * after = NULL, * before = NULL;
	float ratioBefore, ratioAfter;
	Estimator::GetStates(before, after, ratioBefore, ratioAfter, forGivenTimeInMs);
	
	EstimationFloat * afterF = (EstimationFloat*)after, * beforeF = (EstimationFloat*) before;
	float finalValue = beforeF->value * ratioBefore + afterF->value * ratioAfter;
	return finalValue;
	

	/*
	bool loopedTwice = false;
	int beforeIndex = -1, afterIndex = -1;
	/// First find two given points in time that are behind and before our requested time.
	for (int i = currentIndex; true; --i){
		/// Check if we should move i to the top cyclicly.
		if (i < 0){
			/// Only if we've looped at least once with entries.
			if (!hasLooped)
				break;
			i = arraySize - 1;
			if (!loopedTwice)
				loopedTwice = true;
			else if (loopedTwice)
				break;
		}

		EstimationFloat * state = &states[i];
		if (state->time > forGivenTimeInMs){
			after = state;
			afterIndex = i;
		}
		else if (after)
		{
			before = &states[i];
			beforeIndex = i;
			break;
		}
		else if (state->time < forGivenTimeInMs){
			before = state;
			beforeIndex = -1;
			break;
		}
		/// Break if we find both wanted values early.
		if (before && after)
			break;
	}
	/// If we couldn't find neither before nor after requested time, just return now.
	if (!after && !before){
	//	std::cout<<"No after or before state found in estimator.";	
		return 0.f;
	}
	if (!after){
	//	std::cout<<"No after found, using before state.";		
		return before->value;
	}
	if (!before)
	{
	//	std::cout<<"No before found, using after state.";		
		return after->value;
	}
	/// Scale between them only if both are valid
	float timeBetween = (float) (after->time - before->time);
	float relativeTime = (float) (forGivenTimeInMs - before->time);
	if (timeBetween == 0)
	{
		timeBetween = relativeTime;
	}
	/// This should give us a value between 0.0 and 1.0
	float relative = relativeTime / timeBetween;
//	std::cout<<"\nRelative: "<<relative<<" = "<<relativeTime<<" / "<<timeBetween<<", after->time:"<<after->time<<" before->time:"<<before->time
//		<<" Before/after index: "<<beforeIndex<<"/"<<afterIndex;
	/// Interpolated value
	float finalValue = before->value * (1 - relative) + after->value * relative;
	return finalValue;
	*/
}


/// Fetches estimation state by index. 0 refers to current/last index, with negative values being past. 
/*
EstimationFloat * EstimatorFloat::GetState(int index)
{
	assert(states.Size());
	int arrayIndex = currentIndex + index;
	/// Check if has looped, or eh?
	if (arrayIndex < 0 && hasLooped)
	{
		arrayIndex += arraySize;
	}
	if (arrayIndex < 0)
		arrayIndex = 0;
	return &states[arrayIndex];
}
*/