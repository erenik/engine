/// Emil Hedemalm
/// 2014-02-07
/// A class for estimating values using various methods, such as interpolation/extrapolation.

#include "Estimator.h"
#include <fstream>
#include <String/AEString.h>

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


/// Tests the estimator, printing results to file and also console output.
void EstimatorVec3f::Test(int vectorValuesToGenerate, int samplesPerValue)
{
	EstimatorVec3f testEstimator(1000);
	testEstimator.EnableExtrapolation();
	testEstimator.mode = EstimationMode::EXTRAPOLATION;
	testEstimator.synchronizationDelay = -100;

	/// Generate some values.
	List<Vector3f> values;
	for (int i = 0; i < vectorValuesToGenerate; ++i){
		values.Add(Vector3f(rand()%100, rand()%100, rand()%100));
	}

	std::fstream file;
	String filename = "EstimatorVec3f_ExtrapolationTest.txt";
	file.open(filename.c_str(), std::ios_base::out);

	long long time = 0;
	/// Insert them one at a time.
	for (int i = 0; i < values.Size(); ++i)
	{
		testEstimator.AddState(values[i], time);
		file<<"\nAdding state: "<<values[i]<<" with time: "<<time;
		std::cout<<"\nAdding state: "<<values[i]<<" with time: "<<time;
		/// Calculate a few extrapolation values between each insertion.
		for (int j = 0; j < samplesPerValue; ++j)
		{
			int estimationTime = time + 100;
			Vector3f estimatedValue = testEstimator.Calculate(estimationTime);
			time += 100 / samplesPerValue;
			file<<"\nEstimation time: "<<estimationTime<<" value estimated: "<<estimatedValue;
		}
	}

	file.close();
	std::cout<<"\nTest done adn written out to file: "<<filename;
}

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
	extrapolationEnabled = false;
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
			/// Check if the time is more recent than any values within, if so return the latest time straight away?
			value = GetInterpolatedValue(forGivenTime);	
			break;
		}
		/// Use previous values to estimate current velocity, position, etc. to estimate for current-time!
		case EstimationMode::EXTRAPOLATION: 
		case EstimationMode::EXTRAPOLATION_PLUS_COLLISION_CORRECTION:
		{
			bool good;
			value = GetExtrapolatedValue(forGivenTime, good);
			if (!good){
				/// Maybe warn or something, that interpolation is better?
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
	/// Check last estimation vs. current value, compared to last estimation time and current time!
//	currentVelocity = (value - lastEstimation) / (forGivenTime - lastEstimationTime);
			

	/// Save away it in case we need it later. (extrapolator...!)
	lastCalculation = value;
//	lastEstimationTime = forGivenTime;
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

	/// If using extrapolation, do more things now.
	if (extrapolationEnabled)
	{
		extrapolatorBase.data = lastExtrapolation.data;
		extrapolatorBase.time = lastExtrapolation.time;

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
		estimatedVelocity += (vec - previousState->data) / AbsoluteValue(denom2);
	}
}

/// Required to use any extrapolation, since it requires another estimator inside for smoothing.
void EstimatorVec3f::EnableExtrapolation()
{
	extrapolationEnabled = true;
	/// 10 values should be enough for this.
	if (!extrapolatorValueSmoother)
		extrapolatorValueSmoother = new EstimatorVec3f(arraySize);
}

/// Sets bool flag to false if the given time is old, i.e. within a range of already known values. If so, GetInterpolatedValue should be used instead/afterward.
Vector3f EstimatorVec3f::GetExtrapolatedValue(long long forGivenTime, bool & good)
{
	EstimationStateVec3f * lastState = GetState(0);
	int diffToLastState = forGivenTime - lastState->time;
	/// If timeDiff is negative, mark the request as negative and just return the latest value.
	if (diffToLastState < 0){
		good = false;
		return lastState->data;
	}
	/// Alright. Fresh start using the approach I had on my presentation slides, which is a "bit" easier than the confusion I've managed to come up with here so far.
	int timeDiff = (forGivenTime - extrapolatorBase.time);
	Vector3f estimatedValue = extrapolatorBase.data + estimatedVelocity * timeDiff;
	lastExtrapolation.data = estimatedValue;
	lastExtrapolation.time = forGivenTime;
	return estimatedValue;

/*** RETHINKING EVERYTHING

	std::cout<<"\nGetExtrapolatedValue called with time: "<<forGivenTime;
	assert(extrapolatorValueSmoother && "Call EnableExtrapolation first");
	if (!extrapolatorValueSmoother)
		return Vector3f();
	/// Fetch last 2 positions and extract estimated speed therein
	EstimationStateVec3f * lastValue, * nextLastValue;
	lastValue = GetState(0);
	nextLastValue = GetState(-1);
	/// Time difference in milliseconds
	long long timeDiff = (lastValue->time - nextLastValue->time);
	Vector3f currentValue;	
	/// When entering initial stuff.
	if (timeDiff <= 0)
		currentValue = lastValue->data;
	/// Decent timeDiff value
	else
	{
		/// Estimated speed per millisecond.
		Vector3f estimatedSpeed = (lastValue->data - nextLastValue->data) / timeDiff; 
		/// If no change between this and the last, just use the last value.
		if (estimatedSpeed.LengthSquared() == 0)
		{
			std::cout<<"\nNo change present between last values";
			currentValue = lastValue->data;
		}
		/// If a change is present, apply it to get a new estimation.
		else {
			/// Apply speed and time different to last position value to estimate where we should be.
			long long lastToCurrentTime = forGivenTime - lastValue->time;
			if (lastToCurrentTime < 0){
				good = false;
				return Vector3f();
			}
			currentValue = lastValue->data + estimatedSpeed * lastToCurrentTime;
		}
	}
	/// Save new extrapolator estimation value.
	extrapolatorValueSmoother->AddState(currentValue, forGivenTime);
	
	/// Fetch an interpolated value of the extrapolator at -10 ms in order to smooth things out.
	Vector3f blendedEstimation = extrapolatorValueSmoother->GetInterpolatedValue(forGivenTime - smoothingDuration);
	/// Use blended estimation!
	return blendedEstimation;
	*/
}

Vector3f EstimatorVec3f::GetInterpolatedValue(long long forGivenTime)
{
	EstimationStateVec3f * after = NULL, * before = NULL;
	bool loopedTwice = false;
	/// First find two given points in time that are both behind and before our requested time.
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

		EstimationStateVec3f * state = &states[i];
		if (state->time > forGivenTime){
			after = state;
		}
		else if (after)
		{
			before = &states[i];
			break;
		}
		else if (state->time < forGivenTime){
			before = state;
			break;
		}
		/// Break if we find both wanted values early.
		if (before && after)
			break;
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
//	std::cout<<"\nRelative: "<<relative;
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
	if (arrayIndex < 0)
		arrayIndex = 0;
	return &states[arrayIndex];
}