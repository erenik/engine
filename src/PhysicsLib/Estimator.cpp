/// Emil Hedemalm
/// 2014-02-07
/// A class for estimating values using various methods, such as interpolation/extrapolation.

#include "Estimator.h"
#include "File/LogFile.h"

Estimation::Estimation(int type)
	: type(type)
{
}

Estimation::Estimation(int type, int64 time)
: type(type), time(time)
{	
}

Estimation::~Estimation()
{
}

/// Sets finished to false.
Estimator::Estimator()
{
	currentIndex = -1;
	finished = false;
	timeElapsedMs = 0;
	/// Minimum and maximum time in ms which the currently stored estimation states cover. Both are 0.0 by default.
	minMs = maxMs = 0;
	cyclic = false;

	/// True if an estimator should loop its input time values. Default false.
	loop = false;

	inheritFirstValue = false;
	maxStates = 100;
	searchType = SEARCH_ALL;
}

Estimator::~Estimator()
{
	states.ClearAndDelete();
}

/// Adding a new state.
void Estimator::AddState(Estimation * state)
{
	states.AddItem(state);
	if (states.Size() > maxStates)
	{
		Estimation * first = states[0];
		if (states.RemoveIndex(0, ListOption::RETAIN_ORDER))
			delete first;
	}
	++currentIndex;

	int64 timeInMs = state->time;
	// Update interval stats.
	maxMs = maxMs > timeInMs? maxMs : timeInMs;
	totalIntervalMs = maxMs - minMs;
}

/// Attempts to insert the given state at a decent place based on it's time-stamp.
void Estimator::InsertState(Estimation * state)
{
	bool added = false;
	int64 timeInMs = state->time;
	for (int i = 0; i < states.Size(); ++i)
	{
		Estimation * otherState = states[i];
		if (otherState->time > timeInMs)
		{
			states.Insert(state, i);
			added = true;
			break;
		}
	}
	// Simple insert if needed.
	if (!added)
	{
		states.AddItem(state);
		// Update interval stats.
		maxMs = maxMs > timeInMs? maxMs : timeInMs;
		totalIntervalMs = maxMs - minMs;
	}

	++currentIndex;
}

void Estimator::GetStates(Estimation * & before, Estimation * & after, float & ratioBefore, float & ratioAfter, int64 forGivenTimeInMs)
{
	after = before = NULL;
	if (states.Size() == 0)
		return;
	
	bool loopedTwice = false;
	int beforeIndex = -1, afterIndex = -1;
	float timeToBefore = 0;
	int distToBefore = 1000000;
	int distToAfter = 1000000;
	switch(searchType)
	{
		case SEARCH_ALL:
		{
			after = before = states[0];
			distToBefore = distToAfter = AbsoluteValue(after->time - forGivenTimeInMs);
			for (int i = 0; i < states.Size(); ++i)
			{
				Estimation * state = states[i];
				int timeDiff = AbsoluteValue(state->time - forGivenTimeInMs);
				if (state->time < forGivenTimeInMs && timeDiff < distToBefore)
				{
					before = state;
					distToBefore = timeDiff;
				} 
				else if (state->time > forGivenTimeInMs && timeDiff < distToAfter)
				{
					after = state;
					distToAfter = timeDiff;
				}
			}
			break;
		}
		case SOME_OLD_SHIT:
		{
			/// First find the two given points in time that are closest behind and before our requested time.
			for (int i = currentIndex % states.Size(); true; --i)
			{
				if (i < 0)
				{
					if (cyclic)
						i = states.Size() - 1;
					else 
						break;
				}
				Estimation * state = states[i];
				if (state->time >= forGivenTimeInMs)
				{
					// If we already have an after, check which one is closer.
					if (after)
					{
						if (state->time < after->time)
							after = state;
					}
					else{
						after = state;
						afterIndex = i;
					}
				}
				else if (after)
				{
					if (before)
					{
						if (state->time > before->time)
							before = state;
					}
					else {
						before = state;
						beforeIndex = i;
					}
				}
				else if (state->time < forGivenTimeInMs)
				{
					float timeTo = AbsoluteValue(state->time - forGivenTimeInMs);
					if (before == NULL || 
						(before && timeTo < timeToBefore))
					{
						timeToBefore = timeTo;			
						before = state;
						beforeIndex = i;
					}
				}
				/*
				else if (state->time < forGivenTimeInMs){
					before = state;
					beforeIndex = -1;
					break;
				}*/
				/// Break if we find both wanted values early. 
		//		if (before && after)
			//		break;
			}

			break;
		}
	}
	/// Regular interpolation?
	if (after && before)
	{
		/// Scale between them only if both are valid
		float timeBetween = (float) (after->time - before->time);
		float relativeTime = (float) (forGivenTimeInMs - before->time);
		if (timeBetween == 0)
		{
			timeBetween = relativeTime;
		}
		if (timeBetween == 0)
		{
			LogMain("Bad timeBetween in Estimator", ERROR);
			timeBetween = 1.f;
		}
		/// This should give us a value between 0.0 and 1.0
		float relative = relativeTime / timeBetween;
		ratioAfter = relative;
		ratioBefore = 1.0 - relative;
	}
	// Just the after value
	else if (after)
	{
		ratioAfter = 1.f; 
		ratioBefore = 0.f;
	}
	// Or jsut the before value.
	else if (before)
	{
		ratioBefore = 1.f;
		ratioAfter = 0.f;
	}
}
