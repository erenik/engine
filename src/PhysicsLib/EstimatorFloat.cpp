/// Emil Hedemalm
/// 2014-09-12
/// o.o

#include "EstimatorFloat.h"

/// Size of states-array from which we sample.
#define arraySize states.Size()

EstimationStateFloat::EstimationStateFloat()
	: EstimatorState(EstimatorType::FLOAT)
{
}

EstimationStateFloat::EstimationStateFloat(float fValue, int64 timeStamp)
	: EstimatorState(EstimatorType::FLOAT, timeStamp), value(fValue)
{
}


/// Constructor which sets this estimator up to write to a specific.
EstimatorFloat::EstimatorFloat()
{
	variableToPutResultTo = NULL;
	currentIndex = -1;
	
}

/// Process, i.e. increase recorded duration and evaluate the floating point value at the new time point.
void EstimatorFloat::Process(int timeInMs)
{
	if (finished)
		return;
	// just testin..
	timeElapsedMs += timeInMs;
	

	float value;
	int mode = EstimationMode::INTERPOLATION;
	switch(mode){
		// Just grab latest value.
		case EstimationMode::NONE:
		{
			/*
			if (currentIndex >= 0)
				value = states[currentIndex].data;
			else if (hasLooped)
				value = states[arraySize-1].data;
			else
				value = Vector3f();
				*/
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
			/*
			bool good;
			value = GetExtrapolatedValue(forGivenTime, good);
			if (!good){
				/// If not good time, just fetch latest input value.
				value = GetState(0)->data;
			}*/
			break;
		}
		case EstimationMode::INTER_PLUS_EXTRA:
		{
			/*
			bool good;
			value = GetExtrapolatedValue(forGivenTime, good);
			if (!good){
				value = GetInterpolatedValue(forGivenTime);
			}*/

			break;
		}
	}			

	/// Save away it in case we need it later. (extrapolator...!)
	lastCalculation = value;

	/// Mark as finished when applicable.
	if (timeElapsedMs > states.Last().time)
		finished = true;
	
	/// Apply value where requested.
	*variableToPutResultTo = value;
}

/// For adding any length of states as arguments.
void EstimatorFloat::AddStates(int numStates, float f1, int timeStamp1, float f2, int timeStamp2, ...)
{
	AddState(f1, timeStamp1);
	AddState(f2, timeStamp2);

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
		std::cout<<"\nfx"<<fx<<" timestamp"<<timeStampx;
		AddState(fx, timeStampx);
    }
    va_end ( argptr );      // Cleans up the list


}	

/// o.o
void EstimatorFloat::AddState(float fValue, int timeStamp)
{
	EstimationStateFloat newState = EstimationStateFloat(fValue, timeStamp);
	states.Add(newState);
	
	++currentIndex;
	if (currentIndex >= arraySize){
		currentIndex = 0;
		hasLooped = true;
	}
	/*
	/// If using extrapolation, do more things now.
	if (extrapolationEnabled)
	{
		extrapolatorBase.data = lastExtrapolation.data;
		extrapolatorBase.time = lastExtrapolation.time;
		/// If bad, fix.
		if (extrapolatorBase.data != extrapolatorBase.data){
			lastExtrapolation.data = Vector3f();
			estimatedVelocity = Vector3f();
			extrapolatorBase.data = Vector3f();
			return;
		}

		/// Get new velocity by looking at the last two state values, but also the difference between this most recent state value and our own predicted value
		/// First diff between our estimation and the new value.
		int denom = (timeStamp - extrapolatorBase.time);
		if (denom == 0){
			estimatedVelocity = Vector3f();
			return;
		}
		estimatedVelocity = (vec - extrapolatorBase.data) / AbsoluteValue(denom);
		/// Next the diff between the last two stored values.
		int previousIndex = currentIndex - 1;
		if (previousIndex < 0)
			previousIndex += arraySize;
		EstimationStateVec3f * previousState = &states[previousIndex];
		int denom2 = (timeStamp - previousState->time);
		if (denom2 == 0)
			return;
		estimatedVelocity += (vec - previousState->data) / AbsoluteValue(denom2);
		// Apply damping to estimated velocity?
	//	estimatedVelocity *= 0.95f;
		if (estimatedVelocity.x != estimatedVelocity.x)
		{
			std::cout<<"\nWooops.";
		}
	}
	*/
}


/// Fetches interpolated value for given time-stamp.
float EstimatorFloat::GetInterpolatedValue(int64 forGivenTime)
{
	EstimationStateFloat * after = NULL, * before = NULL;
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

		EstimationStateFloat * state = &states[i];
		if (state->time > forGivenTime){
			after = state;
			afterIndex = i;
		}
		else if (after)
		{
			before = &states[i];
			beforeIndex = i;
			break;
		}
		else if (state->time < forGivenTime){
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
		std::cout<<"No after or before state found in estimator.";	
		return 0.f;
	}
	if (!after){
		std::cout<<"No after found, using before state.";		
		return before->value;
	}
	if (!before){
		std::cout<<"No before found, using after state.";		
		return after->value;
	}
	/// Scale between them only if both are valid
	float timeBetween = (float) (after->time - before->time);
	float relativeTime = (float) (forGivenTime - before->time);
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
}


/// Fetches estimation state by index. 0 refers to current/last index, with negative values being past. 
EstimationStateFloat * EstimatorFloat::GetState(int index)
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
