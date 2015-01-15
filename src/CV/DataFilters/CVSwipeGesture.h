/// Emil Hedemalm
/// 2014-11-10
/// Swipe gesturrrrrres!~

#include "CVDataFilters.h"
#include "Time/Time.h"
#include "CV/Data/CVSwipe.h"

class CVSwipeGesture : public CVDataFilter
{
public:
	CVSwipeGesture();
	virtual ~CVSwipeGesture();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * smoothing, * minPointsForSwipe, * minPointsBeforeLeavingSwipe;
	CVFilterSetting * maxFramesToAnalyze, * maxSwipeDuration, * minFramesWithFlow;
	float pointsSmoothed;
	Vector2f directionSmoothed;
	// See SwipeState namespace above.
	int swipeState;
	int framesWithFlow;
	/// All averaged directions accumulated during the swipe-gesture. Used to calculate the overall direction of the entire swipe.
	List<Vector2f> swipeDirections;
	Time swipeStart;
};
