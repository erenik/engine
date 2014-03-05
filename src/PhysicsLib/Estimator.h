/// Emil Hedemalm
/// 2014-02-07
/// A class for estimating values using various methods, such as interpolation/extrapolation.

#ifndef ESTIMATOR_H
#define ESTIMATOR_H

#include "MathLib.h"
#include "List/List.h"

namespace EstimationMode {
enum modes {
		NONE, // This we've been using so far.
		INTERPOLATION,
		EXTRAPOLATION,
};	};
	

namespace EstimatorType {
enum estimTypes {
	NULL_TYPE,
	VEC3F,

};};

/// A state to be entered into the estimator.
class EstimatorState {
public:
	EstimatorState(int type, long long time);
	long long time;
	int type;
};

/// Subclass for Vector3f handling
class EstimationStateVec3f : public EstimatorState {
public:
	EstimationStateVec3f();
	EstimationStateVec3f(Vector3f value, long long timeStamp);
	Vector3f data;
};

/// An object for calculating and estimating values, for example with interpolation/extrapolation. Subclass for more custom behaviour.
class EstimatorVec3f {
public:
	/// Constructor. First argument sets array for which the estimation will be used.
	EstimatorVec3f(int sampleDataArraySize, int initialMode = EstimationMode::NONE);
	virtual ~EstimatorVec3f();
	/// Calculates values as estimated for given time.
	Vector3f Calculate(long long forGivenTime);
	void AddState(Vector3f vec, long long timeStamp);
	/// Required to use any extrapolation, since it requires another estimator inside for smoothing.
	void EnableExtrapolation();
	/// See modes above.
	int mode;
	/// Delay to be applied for interpolation and smoothing duration when using extrapolation. 
	int synchronizationDelay;
	/// Extrapolation smoothing duration. Must be positive.
	long long smoothingDuration;
	/// Current velocity using estimation.
	Vector3f CurrentVelocity() {return currentVelocity;};
protected:
	/// Sets bool flag to false if the given time is old, i.e. within a range of already known values. If so, GetInterpolatedValue should be used instead/afterward.
	Vector3f GetExtrapolatedValue(long long forGivenTime, bool & good);
	Vector3f GetInterpolatedValue(long long forGivenTime);

	/// Fetches estimation state by index relative to currentIndex. 0 refers to next state, with negative values being past. 
	EstimationStateVec3f * GetState(int index);
	EstimationStateVec3f * states;
	/// Size of states-array from which we sample.
	int arraySize;
	/// current index in which new data has been. meaning all indexes behind it, including itself, are populated.
	int currentIndex;
	/// Flagged once after all elements have been populated.
	bool hasLooped;

	/// Two latest values that the extrapolator yielded. Used to transition between them.
	EstimatorVec3f * extrapolatorValueSmoother;

	Vector3f lastEstimation;
	long long lastEstimationTime;
	/// Used to get a feeling of how fast things are going as far as the estimations are cornerned.
	Vector3f currentVelocity;
};


/// Sub-class this to add custom behaviour and actual estimators (see above).
class Estimator {
public:
	/// Proceeds a time-step.
	virtual void Process() = 0;
};

#endif
