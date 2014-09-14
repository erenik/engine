/// Emil Hedemalm
/// 2014-09-12
/// Default works with interpolation, but may be extended to handle extrapolation or other estimation variants.

#ifndef ESTIMATOR_FLOAT_H
#define ESTIMATOR_FLOAT_H

#include "Estimator.h"
#include <cstdarg>

/// Subclass for Vector3f handling
class EstimationStateFloat : public EstimatorState {
public:
	EstimationStateFloat();
	EstimationStateFloat(float fValue, int64 timeStamp);
	float value;
};

class EstimatorFloat : public Estimator
{
public:
	/// Constructor which sets this estimator up to write to a specific.
	EstimatorFloat();
	/// Process, i.e. increase recorded duration and evaluate the floating point value at the new time point.
	virtual void Process(int timeInMs);
	
	/// For adding any length of states as arguments.
	void AddStates(int states, float f1, int timeStamp1, float f2, int timeStamp2, ...);
	/// o.o
	void AddState(float f, int timeStamp);
	
	/// o.o
	float * variableToPutResultTo;
	
private:
	/// Fetches index compared to the last added one.
	EstimationStateFloat * GetState(int index);
	/// Fetches interpolated value for given time-stamp.
	float GetInterpolatedValue(int64 forGivenTime);
	/// Calculates values as estimated for given time.
//	Vector3f Calculate(int64 forGivenTime);
	
	// o.o
	float lastCalculation;
	List<EstimationStateFloat> states;

	/// Estiamted velocity. Could maybe be inserted when adding a state, but for now it will be approximated as well.
	float estimatedVelocity;

	/// Base for extrapolation, gets data from lastEstimation upon each new state-update.
	EstimationStateFloat extrapolatorBase;
	/// Last extrapolation calculation made.
	EstimationStateFloat lastExtrapolation;
	
	/// Current index in which new data has been placed. meaning all indexes behind it, including itself, are populated. For circular arrays.
	int currentIndex;
	
	/// Flagged once after all elements have been populated.
	bool hasLooped;

	bool good;
};

#endif