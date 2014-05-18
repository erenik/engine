/// Emil Hedemalm
/// 2014-04-29
/// Render-filters, for visual effects or writing to file. Some use the native graphics system to provide 3D-rendering(?)

#ifndef CV_RENDER_FILTER_H
#define CV_RENDER_FILTER_H

#include "CVFilter.h"

// Base class for any filter whose main purpose is rendering output.
class CVRenderFilter : public CVFilter 
{
public:
	// See CVFilterTypes.h for IDs
	CVRenderFilter(int filterID);
	virtual int Process(CVPipeline * pipe);
	// Should be overloaded? If you paint to the output-texture?
	virtual void Paint(CVPipeline * pipe);
};

class CVVideoWriter : public CVRenderFilter 
{
public:
	CVVideoWriter();
	virtual int Process(CVPipeline * pipe);
private:
	CVFilterSetting * outputFile, * duration, * startButton, * encodingFps;
	// Woo
	cv::VideoWriter videoWriter;
	// Temporary internal state variables
	bool writing;
	int frame;
	bool imageSequence;
	String targetFolder;
};



class Entity;

/// Filter that renders an image gallery on top of a hand, reacting to gesture input if there is any such in the pipeline
class CVImageGalleryHand : public CVRenderFilter 
{
public:
	CVImageGalleryHand();
	virtual int Process(CVPipeline * pipe);
	
	/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
	virtual void SetEnabled(bool state);
private:
	CVFilterSetting * directory, * minimumTimeBetweenSwitches;
	String currentDirectory;
	List<String> files;
	int currentImage;
	Entity * galleryEntity;
	long long lastSwap;
	int fingersLastFrame;
};

class MultimediaStream;

// Projects a movie onto best known quad or... polygon.
class CVMovieProjector : public CVRenderFilter 
{
public:
	CVMovieProjector();
	virtual ~CVMovieProjector();
	virtual int Process(CVPipeline * pipe);
	// Should be called when deleting a filter while the application is running. Removes things as necessary.
	virtual void OnDelete();
	virtual void Paint(CVPipeline * pipe);
	
	/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
	virtual void SetEnabled(bool state);
private:
	CVFilterSetting * directory, * minimumTimeBetweenSwitches;
	String movieFile;
	Entity * movieEntity;
	MultimediaStream * movieStream;
};


class Track;

class CVMusicPlayer : public CVRenderFilter 
{
public:
	CVMusicPlayer();
	virtual ~CVMusicPlayer();
	virtual int Process(CVPipeline * pipe);
	// Should be called when deleting a filter while the application is running. Removes things as necessary.
	virtual void OnDelete();
	virtual void Paint(CVPipeline * pipe);
	/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
	virtual void SetEnabled(bool state);
private:
	CVFilterSetting * directory;
	String audioFile;
	String currentDirectory;
	Entity * audioEntity;
	List<String> files;
	// Index of current track among the files.
	int currentTrack;
	int lastFinger;
	int64 lastFingerStart;
	int64 lastFingerDuration;
	// o-o
	Track * audioTrack;

};




#endif