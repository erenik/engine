/// Emil Hedemalm
/// 2014-02-07
/// A class for estimating values using various methods, such as interpolation/extrapolation.

#ifndef ESTIMATOR_H
#define ESTIMATOR_H

#include "MathLib.h"
#include "List/List.h"
#include "System/DataTypes.h"

namespace EstimationMode {
enum modes {
		NONE, // This we've been using so far.
		INTERPOLATION,
		/* Raw extrapolation allows for faster response-time but may also produce artifacts like passing through walls and back. */
		EXTRAPOLATION,
		/* This variant of extrapolation tries to take into consideration some collision-detection in order to avoid some artifacts. */
		EXTRAPOLATION_PLUS_COLLISION_CORRECTION, // Uses collission detection to try and avoid lesser errors in the extrapolation.
		INTER_PLUS_EXTRA, // Interpolation + extrapolation
		/** This mode uses the Estimator purely to compare previous values of simulated positions versus Server-provided values. 
			This mode will allow for instantaneous reactions, but server corrections might behave a bit strangely, like turning back after turning too much?
		*/
		CLIENT_SIMULATION_WITH_SERVER_CORRECTIONS, 
};	};
	

namespace EstimatorType {
enum estimTypes {
	NULL_TYPE,
	FLOAT,
	VEC3F,
};};

/// A state to be entered into the estimator.
class EstimatorState {
public:
	EstimatorState(int type);
	EstimatorState(int type, int64 time);
	long long time;
	int type;
};



/// Sub-class this to add custom behaviour and actual estimators (see above).
class Estimator 
{
public:
	/// Sets finished to false.
	Estimator();

	/// Proceeds a time-step.
	virtual void Process(int timeInMs) = 0;

	/// When true, the final state has been reached, and there is no longer any use for this estimator to be active or exist anymore (unless more states are added)
	bool finished;

	/// Duratoin in milliseconds.
	int timeElapsedMs;
};

#endif
