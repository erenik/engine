/// Emil Hedemalm
/// 2014-02-07
/// A class for estimating values using various methods, such as interpolation/extrapolation.

#include "Estimator.h"


EstimatorState::EstimatorState(int type, long long time)
: type(type), time(time){
	
}

EstimationStateVec3f::EstimationStateVec3f()
: EstimatorState(EstimatorType::VEC3F, 0)
{
	data = Vector3f();
}

/// Subclass for Vector3f handling
EstimationStateVec3f::EstimationStateVec3f(Vector3f value, long long timeStamp)
: EstimatorState(EstimatorType::VEC3F, timeStamp)
{
	data = value;
}


/*	enum modes {
		NONE, // This we've been using so far.
		INTERPOLATION,
		EXTRAPOLATION,
	};
	*/

/// Constructor. First argument sets array for which the estimation will be used.
EstimatorVec3f::EstimatorVec3f(int sampleDataArraySize, int initialMode /* = NONE */)
: mode(initialMode) 
{
	states = new EstimationStateVec3f[sampleDataArraySize];
	arraySize = sampleDataArraySize;
	currentIndex = 0;
	hasLooped = false;
	synchronizationDelay = 100;
	smoothingDuration = 100;
}

EstimatorVec3f::~EstimatorVec3f(){
	delete[] states;
	states = NULL;
}

/// Calculates values as estimated for given time.
Vector3f EstimatorVec3f::Calculate(long long forGivenTime){
	Vector3f value;
	switch(mode){
		// Just grab latest value.
		case EstimationMode::NONE:
		{
			if (currentIndex > 0)
				value = states[currentIndex-1].data;
			else if (hasLooped)
				value = states[arraySize-1].data;
			else
				value = Vector3f();
			break;
		}
		// Smooth between one to two values.
		case EstimationMode::INTERPOLATION: 
		{	
			/// Apply synchronization delay straight to the time wanted.
			forGivenTime -= synchronizationDelay;
			EstimationStateVec3f * after = NULL, * before = NULL;
			for (int i = currentIndex - 1; i < currentIndex || i != currentIndex && hasLooped; --i){
				if (i < 0)
					i = arraySize - 1;

				EstimationStateVec3f * state = &states[i];
				if (state->time > forGivenTime){
					after = state;
				}
				else if (state->time < forGivenTime){
					before = state;
					break;
				}
			}
			if (!after && !before)
				return Vector3f();
			if (!after)
				return before->data;
			if (!before)
				return after->data;
			/// Scale between them only if both are valid
			float timeBetween = (float)after->time - before->time;
			float relativeTime = (float)forGivenTime - before->time;
			/// This should give us a value between 0.0 and 1.0
			float relative = relativeTime / timeBetween;
			/// Interpolated value
			value = before->data * (1 - relative) + after->data * relative;
			break;
		}
		/// Use previous values to estimate current velocity, position, etc. to estimate for current-time!
		case EstimationMode::EXTRAPOLATION: 
		{
			/// Apply synchronization delay straight to the time wanted.
			forGivenTime -= synchronizationDelay;
			/// Fetch last 2 positions and extract estimated speed therein
			EstimationStateVec3f * lastValue, * nextLastValue;
			lastValue = GetState(-1);
			nextLastValue = GetState(-2);
			long long timeDiff = (lastValue->time - nextLastValue->time);
			if (timeDiff <= 0)
				return lastValue->data;
			Vector3f estimatedSpeed = (lastValue->data - nextLastValue->data) / timeDiff; 
			/// Apply speed and time different to last position value to estimate where we should be.
			long long lastToCurrentTime = forGivenTime - lastValue->time;
			Vector3f currentValue = lastValue->data + estimatedSpeed * lastToCurrentTime;
		
			/// Compare with our previous estimation. If they do not "overlap" a transition is required to smoothing things out.
			/// Pan over the duration of 1 second between our previous estimation and our new one?
			long long timeSinceNewEstimationBegan = forGivenTime - lastEstimationBeforeNewEstimation.time;
			/// Use synchronization delay here to decide smoothing-duration.
			float ratio = timeSinceNewEstimationBegan / (float)smoothingDuration;
			if (ratio > 1)
				ratio = 1.0f;
			if (ratio < 0){
				ratio = 0;
				std::cout<<"\nRatio 0 in Estimator::Calculate! Should only get 1 of these warnings or something might be wrong.";
				return Vector3f();
			}
			assert(ratio >= 0);
			Vector3f blendedEstimation = currentValue * ratio + lastEstimationBeforeNewEstimation.data * (1 - ratio);
			/// Use blended estimation!
			value = blendedEstimation;
		}
	}
	/// Check last estimation vs. current value, compared to last estimation time and current time!
//	currentVelocity = (value - lastEstimation) / (forGivenTime - lastEstimationTime);
			

	/// Save away it in case we need it later. (extrapolator...!)
	lastEstimation = value;
	lastEstimationTime = forGivenTime;
	return value;
}


void EstimatorVec3f::AddState(Vector3f vec, long long timeStamp)
{
	EstimationStateVec3f * v = &states[currentIndex];
	v->data = vec;
	v->time = timeStamp;
	++currentIndex;
	if (currentIndex >= arraySize)
		currentIndex = 0;

	/// New value, so save away our last estimation for reference.
	lastEstimationBeforeNewEstimation.data = lastEstimation;
	lastEstimationBeforeNewEstimation.time = lastEstimationTime;
}

/// Fetches estimation state by index. 0 refers to current/last index, with negative values being past. 
EstimationStateVec3f * EstimatorVec3f::GetState(int index){
	int arrayIndex = currentIndex + index;
	/// Check if has looped, or eh?
	if (arrayIndex < 0){
		arrayIndex += arraySize;
	}
	return &states[arrayIndex];
}