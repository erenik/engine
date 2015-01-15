/// Emil Hedemalm
/// 2014-09-15
/// Dividing up into own files.

#ifndef CV_RENDER_FILTER_H
#define CV_RENDER_FILTER_H

#include "CV/CVFilter.h"
#include "CV/OpenCV.h"
#include "Time/Time.h"

class CVHand;
class Entity;
class Texture;
class MultimediaStream;
class Track;
class ParticleSystem;

// Base class for any filter whose main purpose is rendering output.
class CVRenderFilter : public CVFilter 
{
public:
	// See CVFilterTypes.h for IDs
	CVRenderFilter(int filterID);
	virtual ~CVRenderFilter();
	virtual int Process(CVPipeline * pipe);
	// Should be overloaded? If you paint to the output-texture?
	virtual void Paint(CVPipeline * pipe);

	/// Fetches all dynamically created entities associated with this filter. Must be overloaded.
	virtual List<Entity*> GetEntities() = 0;

	/// By default, calls GetEntities and sets their visibility according to the given state.
	virtual void SetEnabled(bool value);

	// Defaults to true.
	bool renderOntoEditor;
};

#endif
