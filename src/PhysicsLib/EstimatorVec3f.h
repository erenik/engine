/// Emil Hedemalm
/// 2014-09-12
/// Estimator for vector-types.

#ifndef ESTIMATOR_VEC3F_H
#define ESTIMATOR_VEC3F_H

#include "Estimator.h"

/// Subclass for Vector3f handling
class EstimationVec3f : public Estimation {
public:
	EstimationVec3f();
	EstimationVec3f(Vector3f value, int64 timeStampInMs);
	Vector3f value;
};

/// An object for calculating and estimating values, for example with interpolation/extrapolation. Subclass for more custom behaviour.
class EstimatorVec3f : public Estimator {
public:
	/// Tests the estimator, printing results to file and also console output.
	static void Test(int vectorValuesToGenerate, int samplesPerValue);

	/// Empty constructor for variable-lengthed data which is edited as time goes.
	EstimatorVec3f();
	/// Constructor. First argument sets array for which the estimation will be used.
	EstimatorVec3f(int sampleDataArraySize, int initialMode = EstimationMode::NONE);
	void Nullify();
	/// Virtual destructor.
	virtual ~EstimatorVec3f();

	
	/** Estimates values for given time. If loop is true, the given time will be modulated to be within the interval of applicable time-values.
		If the estimator's output pointer is set, data for the given estimation will be written there accordingly.
	*/
	virtual void Estimate(int64 forGivenTimeInMs, bool loop);
	/// Proceeds a time-step.
	virtual void Process(int timeInMs);

	/// Calculates values as estimated for given time.
//	Vector3f Calculate(long long forGivenTime);

	void AddState(Vector3f vec, int64 timeInMs);
	/// Required to use any extrapolation, since it requires another estimator inside for smoothing.
	void EnableExtrapolation();
	/// See modes above.
	int mode;
	/// Delay to be applied for interpolation and smoothing duration when using extrapolation. 
	int synchronizationDelay;
	/// Extrapolation smoothing duration. Must be positive.
	long long smoothingDuration;
	/// Value set when you call Calculate(); Use this if you know that you have already called Calculate once this frame.
	Vector3f lastCalculation;
	/// Current velocity using estimation.
	Vector3f CurrentVelocity() {return currentVelocity;};

	/// Result destination.
	Vector3f * variableToPutResultTo;

protected:
	/// Sets bool flag to false if the given time is old, i.e. within a range of already known values. If so, GetInterpolatedValue should be used instead/afterward.
	Vector3f GetExtrapolatedValue(long long forGivenTime, bool & good);
	

	/// Fetches estimation state by index relative to currentIndex. 0 refers to next state, with negative values being past. 
	EstimationVec3f * GetState(int index);
	
	/// Flag, since more actions are taken when extrapolation is wanted.
	bool extrapolationEnabled;

	/// Two latest values that the extrapolator yielded. Used to transition between them.
	EstimatorVec3f * extrapolatorValueSmoother;

	/// Base for extrapolation, gets data from lastEstimation upon each new state-update.
	EstimationVec3f extrapolatorBase;
	/// Last extrapolation calculation made.
	EstimationVec3f lastExtrapolation;
	/// Estiamted velocity. Could maybe be inserted when adding a state, but for now it will be approximated as well.
	Vector3f estimatedVelocity;

	/// Used to get a feeling of how fast things are going as far as the estimations are cornerned.
	Vector3f currentVelocity;
};

#endif