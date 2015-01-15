/// Emil Hedemalm
/// 2014-07-21
/// 2D ball-based brick-breaking game using computer vision based input.

#include "Random/Random.h"
#include "CV/Games/CVGame.h"
#include "CV/OpenCV.h"

class Entity;
class Texture;
class Breakout;

// The classic Pong based on IPM
class CVBreakout : public CVGame 
{
public:
	CVBreakout();
	virtual ~CVBreakout();
	virtual int Process(CVPipeline * pipe);
	
private:
	/// Scale of both the ball and paddle entities.
	CVFilterSetting * ballScale, * paddleScale, * distanceFromCenter, * initialBallSpeed;
	/// AI speed?
	CVFilterSetting * aiSpeed;
	/// o-o;
	CVFilterSetting * numBalls, * ballSpeedIncreasePerCollision;
	/// Before KO..!
	CVFilterSetting * lives;
	/// For debug-testing.
	CVFilterSetting * level;

	Breakout * breakout;
};







