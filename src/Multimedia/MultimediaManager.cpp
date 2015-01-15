/// Emil Hedemalm
/// 2014-02-16
/// Video manager for handling video streams. Videos may interact with the AudioManager for audio associated with the media.

#include "MultimediaManager.h"

#include "File/FileUtil.h"
#include "FilePath/FilePath.h"
#include "Texture.h"
#include "Multimedia/Ogg/OggStream.h"


MultimediaManager * MultimediaManager::multimediaManager = NULL;

MultimediaManager::MultimediaManager()
{
	lastUpdate = Timer::GetCurrentTimeMs();
}

MultimediaManager::~MultimediaManager()
{
	streams.ClearAndDelete();
}

MultimediaManager * MultimediaManager::Instance()
{
    if (multimediaManager == NULL)
    {
        std::cout<<"Nouuuuuuu";
    }
	assert(multimediaManager);
	return multimediaManager;
}

void MultimediaManager::Allocate(){
	assert(multimediaManager == NULL);
	multimediaManager = new MultimediaManager();
}
void MultimediaManager::Deallocate(){
	assert(multimediaManager);
	delete multimediaManager;
	multimediaManager = NULL;
}
void MultimediaManager::Initialize()
{

}

/// Called once in the deallocator thread when stop procedures have begun but before deallocation occurs.
void MultimediaManager::Shutdown()
{
	/// Unbind audio from streams. Audio should be deleted by the audio-manager.
	for (int i = 0; i < streams.Size(); ++i){
		MultimediaStream * stream = streams[i];
		stream->audio = NULL;
	}
}


/// Saves screenshot of all active screens with visual data to target folder? Returns amount of images saved. All images will have time-stamps on them appropriately.
int MultimediaManager::SaveCurrentFramesToFile()
{
	int imagesSaved = 0;
	for (int i = 0; i < streams.Size(); ++i){
		MultimediaStream * stream = streams[i];
		if (!stream->hasVideo)
			continue;
		if (!stream->IsPlaying())
			continue;
		Texture * tex = stream->GetFrameTexture();
		if (!tex)
			continue;
		bool success = tex->Save(stream->name+"_"+String::ToString(stream->CurrentFrameTime()));
		if (success)
			imagesSaved ++;
	}
	return imagesSaved;
}

/// Try to play video from target path. Returns NULL upon failure.
MultimediaStream * MultimediaManager::Play(String fromPath)
{
	/// Check if the file exists.
	if (!FileExists(fromPath)){
		std::cout<<"\nMultimediaManager::Play: Invalid path: "<<fromPath;
		return NULL;
	}
	MultimediaStream * newStream = NULL;

	/// Check file ending.
	String fileType = FilePath::FileEnding(fromPath);
	if (fileType == "ogg")
	{
		/// Try play it with ogg
		/// TODO: Identify if it contains Theora or what it contains?
#ifdef OGG
		newStream = new OggStream();
		/// Open path.
		newStream->Open(fromPath);
		newStream->startTime = Timer::GetCurrentTimeMs();
		streams.Add(newStream);
		// Play it straight away..?
		newStream->Play();
#endif
	}
	else {
		std::cout<<"\nMultimediaManager::Play: Unknown video type. Unable to play.";
	}


	return newStream;
}


/// Update all streams we've got by looking at how much time has passed since last update.
void MultimediaManager::Update()
{
	long long currentTime = Timer::GetCurrentTimeMs();
	long long timeDiff = currentTime - lastUpdate;
	/// For lagg, don't process the timeDiff so much.
	if (timeDiff > 200)
		timeDiff = 20;
	for (int i = 0; i < streams.Size(); ++i){
		// Fetch streams
		MultimediaStream * ms = streams[i];
		// Remove those that are flagged.
		if (ms->IsPaused() || ms->HasEnded())
			continue;
		// Only play the ones specifically set to playing.
		if (!ms->IsPlaying())
			continue;
		// Check if they need to fetch a new frame.
		ms->UpdateMediaTime();
		
	//	ms->mediaTime += timeDiff;
		int frametimeStartMs = ms->CurrentFrameTime();
		if (ms->mediaTime > frametimeStartMs)
		{
			int timeToPass = ms->mediaTime - frametimeStartMs;
			float timePerFrame = ms->TimePerFrame();
			timePerFrame *= 1000;
			int framesToPass = timeToPass / timePerFrame;
			if (framesToPass > 1){
			//	std::cout<<"\nFrames to pass: "<<framesToPass;

			}
			ms->StreamNextFrame(framesToPass);
		}
		else {
			// std::cout<<"\nWaiting for next frame";
		}
	}
	lastUpdate = currentTime;
}

