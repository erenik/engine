/// Emil Hedemalm
/// 2014-04-29
/// Render-filters, for visual effects or writing to file. Some use the native graphics system to provide 3D-rendering(?)

#ifndef CV_RENDER_FILTERS_H
#define CV_RENDER_FILTERS_H

#include "CVRenderFilter.h"

class Session;

class CVVideoWriter : public CVRenderFilter 
{
public:
	CVVideoWriter();
	virtual int Process(CVPipeline * pipe);
	/// Fetches all dynamically created entities associated with this filter. Must be overloaded.
	virtual List<Entity*> GetEntities();
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

/// Filter that renders an image gallery on top of a hand, reacting to gesture input if there is any such in the pipeline
class CVImageGalleryHand : public CVRenderFilter 
{
public:
	CVImageGalleryHand();
	virtual int Process(CVPipeline * pipe);
	virtual void Paint(CVPipeline * pipe);
	/// Fetches all dynamically created entities associated with this filter. Must be overloaded.
	virtual List<Entity*> GetEntities();

	// Should be called when deleting a filter while the application is running. Removes things as necessary.
	virtual void OnDelete();
	
	/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
	virtual void SetEnabled(bool state);
private:
	void SetTexture(String source);
	void UpdateScale(CVHand & hand);

	CVFilterSetting * directory, * minimumTimeBetweenSwitches;
	String currentDirectory;
	List<String> files;
	int currentImage;
	Entity * galleryEntity;
	Texture * tex;
	long long lastSwap;
	int fingersLastFrame;
};


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
	/// Fetches all dynamically created entities associated with this filter. Must be overloaded.
	virtual List<Entity*> GetEntities();

	
	/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
	virtual void SetEnabled(bool state);
private:
	CVFilterSetting * directory, * minimumTimeBetweenSwitches;
	String movieFile;
	Entity * movieEntity;
	MultimediaStream * movieStream;

	// Yo.
	CVFilterSetting * framesToSmooth;
	Vector3f averagePosition;
	Vector3f averageScale;
};


class CVMusicPlayer : public CVRenderFilter 
{
public:
	CVMusicPlayer();
	virtual ~CVMusicPlayer();
	virtual int Process(CVPipeline * pipe);
	// Should be called when deleting a filter while the application is running. Removes things as necessary.
	virtual void OnDelete();
	virtual void Paint(CVPipeline * pipe);
	/// Fetches all dynamically created entities associated with this filter. Must be overloaded.
	virtual List<Entity*> GetEntities();
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


// A hand-based model-viewer.
class CVModelViewer : public CVRenderFilter 
{
public:
	CVModelViewer();
	virtual int Process(CVPipeline * pipe);
	
	// Should be called when deleting a filter while the application is running. Removes things as necessary.
	virtual void OnDelete();	
	virtual List<Entity*> GetEntities();
	/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
	virtual void SetEnabled(bool state);
private:
	void SetRotationSpeed(Vector3f speed);


	// Name.
	CVFilterSetting * model;
	// Buttons for manual interaction
	CVFilterSetting * rotationSpeed,  * resetPosition, * manualRotation, * playSkeletalAnimation, * animateUsingShaders;

	CVFilterSetting * pauseAnimation;
	Entity * modelEntity;
};

class CVMenuInteraction : public CVRenderFilter 
{
public:
	CVMenuInteraction();
	virtual int Process(CVPipeline * pipe);
	virtual List<Entity*> GetEntities();
	
private:

	/// o-o
	void ClearPreviousPositions();

	CVFilterSetting * minSwipeLength, * minSwipePoints, * swipeTimeWindow,
		* minVelocity, * maxSwipeDurationMillis;

	struct TimePosition 
	{
		Vector2f position;
		Time time;
	};

	/// Previous positions of the interacting point cloud.
	List<TimePosition> previousPositions;

};


class CVSpriteAnimation : public CVRenderFilter 
{
public:
	CVSpriteAnimation();
	virtual int Process(CVPipeline * pipe);
	virtual List<Entity*> GetEntities();	
	// Should be called when deleting a filter while the application is running. Removes things as necessary.
	virtual void OnDelete();	
private:
	CVFilterSetting * scale;
	Entity * spriteEntity;
	Vector3f smoothedPosition;
};


class Recipe 
{
public:
	String name;
	String referenceID;
	String imageSource;
};

/** For interactive kitchen demonstration, of a plate that will have content projected onto it, 
	and a selection of menus being browsable using swipe-gestures.
*/
class CVInteractivePlate : public CVRenderFilter 
{
public:
	CVInteractivePlate();
	virtual ~CVInteractivePlate();
	virtual int Process(CVPipeline * pipe);
	virtual void ProcessMessage(Message * message);
	virtual List<Entity*> GetEntities();
	// Should be called when deleting a filter while the application is running. Removes things as necessary.
	virtual void OnDelete();

private:

	void FetchRecipes(bool fromImageDir = false);

	void NextMenu();
	void PreviousMenu();
	void UpdatePlate();
	void BringUpRecipe();
	void BringDownRecipe();

	void PostToPi();

	void SetSleep(bool sleepState);

	void GrabFoodImages();
	void UpdateScale();

	List<Recipe> recipes;

	CVFilterSetting * platePositionSmoothingFactor;
	CVFilterSetting * plateRotationSpeed, * plateScale, * rotationOffset;
	// An integer. Swipes manipulate this value.
	CVFilterSetting * currentMenu, * webPage, * remoteBrowser;
	/// Time before sleep will come into effect.
	CVFilterSetting * sleepTime;
	CVFilterSetting * lockPlateInCenter, * lockPlateScale;

	Entity * plateEntity;

	Vector3f averagedPosition;
	float averagedScale;
	float recalculatedScale;
	bool sleeping;
	Time lastInteraction;
	List<String> foodImages;
};


class CVPiano : public CVRenderFilter 
{
public:
	CVPiano();
	virtual ~CVPiano();
	virtual int Process(CVPipeline * pipe);
	virtual List<Entity*> GetEntities();
private:
	/// o.o
	void PlayKey(Vector2i atPosition);
	void LoadKeysFromDirectory(String dir);

	CVFilterSetting * playSound;

	List<String> keySources;
};

class CVInteractiveSphere : public CVRenderFilter 
{
public:
	CVInteractiveSphere();
	virtual ~CVInteractiveSphere();
	virtual int Process(CVPipeline * pipe);
	virtual void ProcessMessage(Message * message);
	virtual List<Entity*> GetEntities();
private:

	void GoToItem(int index);
	void UpdateProjectionScale();
	void GrabTextures();
	void SetTexturesForBoards();
	void OnHostChanged();


	void AssignProjectionSphere();

	CVFilterSetting * sphereScale, * boardScale, * boardDistance,
		* rotationSpeed, * focusItem, * instantSwitches,
		* boardAlpha, * textureDir, * boardRotation, * sphereRotation,
		* connectToPeer, * changeHost, * projectedContent, * syncWithClient,
		* projectionScreenScale, * projectionScreenSphereSegmentSize, * projectionScreenSphereOffset,
		* whiteBackground, * projectionModel, * sphereColor, * port,
		* switchDuration;

	/// 0 - None, 1 - Me, 2 - you
	int host;
	float rotationBetween;

	List<String> textures;

	List<Entity*> boardEntities;
	Entity * boardReferenceEntity;
	Entity * sphereEntity;
	Entity * projectionScreenEntity;
	Texture * projectionTexture;


	Session * session;
};


#endif



