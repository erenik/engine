/// Emil Hedemalm
/// 2014-09-12
/// Default works with interpolation, but may be extended to handle extrapolation or other estimation variants.

#ifndef ESTIMATOR_FLOAT_H
#define ESTIMATOR_FLOAT_H

#include "Estimator.h"
#include <cstdarg>

/// Subclass for Vector3f handling
class EstimationFloat : public Estimation {
public:
	EstimationFloat();
	EstimationFloat(float fValue, int64 timeStamp);
	float value;
};

class EstimatorFloat : public Estimator
{
public:
	/// Constructor which sets this estimator up to write to a specific.
	EstimatorFloat();

	/** Estimates values for given time. If loop is true, the given time will be modulated to be within the interval of applicable time-values.
		If the estimator's output pointer is set, data for the given estimation will be written there accordingly.
	*/
	virtual void Estimate(const Time & forGivenTime, bool loop);
	
	/// Process, i.e. increase recorded duration and evaluate the floating point value at the new time point.
	virtual void Process(int timeInMs);
	
	/// For adding any length of states as arguments. timestamps in milliseconds.
	void AddStatesMs(int states, float f1, int timeStamp1, float f2, int timeStamp2, ...);
	/// For adding any length of states as arguments. timestamps in seconds.
	void AddStatesSeconds(int states, float f1, float timeStamp1, float f2, float timeStamp2, ...);
	
	/// Adding a state at the end. time stamp in Ms!
	void AddStateMs(float f, int64 timeStampInMs);
	/// Inserts the state at a decent location based on the time-stamp.
	void InsertState(float f, int64 timeStampInMs);

	/// o.o
	float * variableToPutResultTo;
	List<float*> variablesToPutResultTo;
	
private:
	/// Fetches index compared to the last added one.
//	EstimationFloat * GetState(int index);
	/// Fetches interpolated value for given time-stamp.
	float GetInterpolatedValue(int64 forGivenTime);
	/// Calculates values as estimated for given time.
//	Vector3f Calculate(int64 forGivenTime);
	
	// o.o
	float lastCalculation;
	
	/// Estiamted velocity. Could maybe be inserted when adding a state, but for now it will be approximated as well.
	float estimatedVelocity;

	/// Base for extrapolation, gets data from lastEstimation upon each new state-update.
	EstimationFloat extrapolatorBase;
	/// Last extrapolation calculation made.
	EstimationFloat lastExtrapolation;
	
	/// Current index in which new data has been placed. meaning all indexes behind it, including itself, are populated. For circular arrays.
	int currentIndex;
	
	/// Flagged once after all elements have been populated.
	bool hasLooped;

	bool good;
};

#endif