/// Emil Hedemalm
/// 2014-07-21
/// 2D ball-based games using computer vision based input

#include "Random/Random.h"
#include "CV/Games/CVGame.h"
#include "CV/OpenCV.h"

class Entity;
class Texture;
class Pong;

// The classic Pong based on IPM
class CVPong : public CVGame
{
public:
	CVPong();
	virtual ~CVPong();
	virtual int Process(CVPipeline * pipe);
	
private:

	// The actual game.
	Pong * pong;


	/// Scale of both the ball and paddle entities.
	CVFilterSetting * ballScale, * paddleScale, * distanceFromCenter, * initialBallSpeed;
	/// AI speed?
	CVFilterSetting * aiSpeed;
	/// o-o;
	CVFilterSetting * numBalls, * ballSpeedIncreasePerCollision;
};





