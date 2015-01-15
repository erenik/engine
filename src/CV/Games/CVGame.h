/// Emil Hedemalm
/// 2014-09-15
/// Filter for games. Contains relevant functions for sizing the game content relative to filter settings.

#ifndef CV_GAME_H
#define CV_GAME_H

class Game2D;
class Integrator;
class Entity;

#include "CV/RenderFilters/CVRenderFilters.h"

/// A general class for games based on the computer vision pipeline.
class CVGame : public CVRenderFilter 
{
public:
	CVGame(int filterID);
	virtual ~CVGame();
	/// Called upon adding the filter to the active pipeline.
	virtual void OnAdd();
	// Should be called when deleting a filter while the application is running. Removes things as necessary.
	virtual void OnDelete();

	// Main processing function, sub-class and re-implement this.
	virtual int Process(CVPipeline * pipe);
	/// Build-in messaging system. Used for advanced applications to communicate with the game engine properly.
	virtual void ProcessMessage(Message * message);

	/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
	virtual void SetEnabled(bool value);

	/// Fetches all dynamically created entities associated with this filter. Must be overloaded.
	virtual List<Entity*> GetEntities();
	// Should be overloaded? If you paint to the output-texture?
	void Paint(CVPipeline * pipe);

protected:
	/// If this game uses a specific integrator.
	Integrator * integrator;
	/// The game we are to synchronize the general settings with. E.g. SetFrameSize varying with sizeRatio.
	Game2D * game;

private:
	// As its name shows, used to re-scale the game only when needed.
	Vector2i lastFrameSize;
	
	/// Resetting the game, similar to a hardware Reset button on an old console.
	CVFilterSetting * resetButton;
	/// Player control
	CVFilterSetting * useMouse;
	/** Size ratio, based on input. Used to scale down the playing field so that input will work decently 
		(for those cases when input-detection is worse along the edges/corners)
	*/
	CVFilterSetting *  sizeRatio;
	/// Z-depth of all entities, or at least most.
	CVFilterSetting * z;

};

#endif




