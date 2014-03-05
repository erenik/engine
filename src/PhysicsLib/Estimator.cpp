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
	currentIndex = -1;
	hasLooped = false;
	synchronizationDelay = 100;
	smoothingDuration = 100;
	extrapolatorValueSmoother = NULL;
}

EstimatorVec3f::~EstimatorVec3f(){
	delete[] states;
	states = NULL;
	if (extrapolatorValueSmoother)
		delete extrapolatorValueSmoother;
}

/// Calculates values as estimated for given time.
Vector3f EstimatorVec3f::Calculate(long long forGivenTime){
	Vector3f value;
	/// Apply synchronization delay straight to the time wanted.
	forGivenTime -= synchronizationDelay;	
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
			value = GetInterpolatedValue(forGivenTime);	
			break;
		}
		/// Use previous values to estimate current velocity, position, etc. to estimate for current-time!
		case EstimationMode::EXTRAPOLATION: 
		{
			bool good;
			value = GetExtrapolatedValue(forGivenTime, good);
			if (!good){
				std::cout<<"\nGiven time too old for extrapolation. Resorting to interpolation.";
				value = GetInterpolatedValue(forGivenTime);
			}
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
	++currentIndex;
	if (currentIndex >= arraySize){
		currentIndex = 0;
		hasLooped = true;
	}
	EstimationStateVec3f * v = &states[currentIndex];
	v->data = vec;
	v->time = timeStamp;
}

/// Required to use any extrapolation, since it requires another estimator inside for smoothing.
void EstimatorVec3f::EnableExtrapolation()
{
	/// 10 values should be enough for this.
	if (!extrapolatorValueSmoother)
		extrapolatorValueSmoother = new EstimatorVec3f(arraySize);
}

/// Sets bool flag to false if the given time is old, i.e. within a range of already known values. If so, GetInterpolatedValue should be used instead/afterward.
Vector3f EstimatorVec3f::GetExtrapolatedValue(long long forGivenTime, bool & good)
{
	assert(extrapolatorValueSmoother && "Call EnableExtrapolation first");
	if (!extrapolatorValueSmoother)
		return Vector3f();
	/// Fetch last 2 positions and extract estimated speed therein
	EstimationStateVec3f * lastValue, * nextLastValue;
	lastValue = GetState(0);
	nextLastValue = GetState(-1);
	long long timeDiff = (lastValue->time - nextLastValue->time);
	if (timeDiff <= 0)
		return lastValue->data;
	Vector3f estimatedSpeed = (lastValue->data - nextLastValue->data) / timeDiff; 
	/// Apply speed and time different to last position value to estimate where we should be.
	long long lastToCurrentTime = forGivenTime - lastValue->time;
	if (lastToCurrentTime < 0){
		good = false;
		return Vector3f();
	}
	Vector3f currentValue = lastValue->data + estimatedSpeed * lastToCurrentTime;

	/// Save new extrapolator value.
	extrapolatorValueSmoother->AddState(currentValue, forGivenTime);
	
	/// Fetch an interpolated value of the extrapolator at -10 ms in order to smooth things out.
	Vector3f blendedEstimation = extrapolatorValueSmoother->GetInterpolatedValue(forGivenTime - smoothingDuration);
	/// Use blended estimation!
	return blendedEstimation;
}

Vector3f EstimatorVec3f::GetInterpolatedValue(long long forGivenTime)
{
	EstimationStateVec3f * after = NULL, * before = NULL;
	bool loopedTwice = false;
	/// First find two given points in time that are both behind and before our requested time.
	for (int i = currentIndex - 1; i != currentIndex; --i){
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

		EstimationStateVec3f * state = &states[i];
		if (state->time > forGivenTime){
			after = state;
		}
		else if (state->time < forGivenTime){
			before = state;
			break;
		}
	}
	/// If we couldn't find neither before nor after requested time, just return now.
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
	Vector3f finalValue = before->data * (1 - relative) + after->data * relative;
	return finalValue;
}


/// Fetches estimation state by index. 0 refers to current/last index, with negative values being past. 
EstimationStateVec3f * EstimatorVec3f::GetState(int index){
	int arrayIndex = currentIndex + index;
	/// Check if has looped, or eh?
	if (arrayIndex < 0 && hasLooped){
		arrayIndex += arraySize;
	}
	return &states[arrayIndex];
}