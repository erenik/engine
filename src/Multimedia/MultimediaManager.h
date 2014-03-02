/// Emil Hedemalm
/// 2014-02-16
/// Video manager for handling video streams. Videos may interact with the AudioManager for audio associated with the media.

#ifndef VIDEO_MANAGER_H
#define VIDEO_MANAGER_H

#define MultimediaMan (*MultimediaManager::Instance())

#include "Multimedia/MultimediaStream.h"

/// TODO: Consider throwing together both audio and video into one multimedia folder... not sure if wise though...
class MultimediaManager {
private:	
	MultimediaManager();
	~MultimediaManager();
	static MultimediaManager * multimediaManager;
public:
	static MultimediaManager * Instance();

	static void Allocate();
	static void Deallocate();
	/// Called once in the initializer thread after allocation but before the engine gets started.
	void Initialize();
	/// Called once in the deallocator thread when stop procedures have begun but before deallocation occurs.
	void Shutdown();

	/// Saves screenshot of all active screens with visual data to target folder? Returns amount of images saved. All images will have time-stamps on them appropriately.
	int SaveCurrentFramesToFile();

	/// Try to play video from target path. Returns NULL upon failure.
	MultimediaStream * Play(String fromPath);	

	/// Update all streams we've got by looking at how much time has passed since last update.
	void Update();

private:
	/// For properly progressing media time.
	long long lastUpdate;
	List<MultimediaStream *> streams;

};

#endif
