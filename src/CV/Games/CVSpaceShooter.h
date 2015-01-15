/// Emil Hedemalm
/// 2014-07-25
/// A space-shooter game based on the input from computer vision imaging

#include "CV/Games/CVGame.h"
#include "CV/OpenCV.h"

class Texture;
class SpaceShooter;

class CVSpaceShooter : public CVGame 
{
public:
	CVSpaceShooter();
	virtual ~CVSpaceShooter();

	virtual int Process(CVPipeline * pipe);
	
private:
	/// Player control
	CVFilterSetting * yOnly;

	/// Scale of both the ball and paddle entities.
	CVFilterSetting * shipScale, * flyingSpeed;
	/// AI speed?
	CVFilterSetting * aiSpeed;
	/// Before KO..!
	CVFilterSetting * lives;
	/// For debug-testing.
	CVFilterSetting * level;

	CVFilterSetting * flipX;

	// The actual game.
	SpaceShooter * spaceShooter;

};
