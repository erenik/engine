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
class Estimation {
public:
	Estimation(int type);
	Estimation(int type, int64 time);
	virtual ~Estimation();
	int64 time;
	int type;
};



/// Sub-class this to add custom behaviour and actual estimators (see above).
class Estimator 
{
public:
	/// Sets finished to false.
	Estimator();
	virtual ~Estimator();

	/** Estimates values for given time. If loop is true, the given time will be modulated to be within the interval of applicable time-values.
		If the estimator's output pointer is set, data for the given estimation will be written there accordingly.
	*/
	virtual void Estimate(int64 forGivenTimeInMs, bool loop) = 0;
	/// Proceeds a time-step.
	virtual void Process(int timeInMs) = 0;
	/// Adding a new state.
	virtual void AddState(Estimation * state);
	/// Attempts to insert the given state at a decent place based on it's time-stamp.
	virtual void InsertState(Estimation * state);

	/// When true, the final state has been reached, and there is no longer any use for this estimator to be active or exist anymore (unless more states are added)
	bool finished;

	/// Duratoin in milliseconds.
	int timeElapsedMs;

	/// Minimum and maximum time in ms which the currently stored estimation states cover. Both are 0.0 by default.
	int64 minMs, maxMs, totalIntervalMs;

	/// Current index in which new data has been placed. meaning all indexes behind it, including itself, are populated.
	int currentIndex;
	/// Flagged once after all elements have been populated.
	bool hasLooped;
	// o.o
	bool cyclic;

	/// True if an estimator should loop its input time values. Default false.
	bool loop;

	/// If true, the handler of estimators should obtain the correct initial value upon attaching the estimator to said value. Default false.
	bool inheritFirstValue;

protected:
	/** Calculates and returns the two states which should be used for calculating the current value in e.g. Estimate
		Both one and two may be NULL.
	*/
	void GetStates(Estimation * & before, Estimation * & after, float & ratioBefore, float & ratioAfter, int64 forGivenTimeInMs);

	/// Yo.
	List<Estimation*> states;
};

#endif
