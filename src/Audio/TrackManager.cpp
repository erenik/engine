/// Emil Hedemalm
/// 2014-01-07
/// A manager for handling sets of tracks, and ensuring playback as needed.

#include "Tracks/Track.h"
#include "TrackManager.h"
#include "Message/MessageManager.h"
#include "File/FileUtil.h"

TrackManager * TrackManager::trackManager = NULL;

String defaultTrackPath = "sound";

TrackManager::TrackManager(){}
TrackManager::~TrackManager(){
	tracks.ClearAndDelete();
	categories.ClearAndDelete();
}

void TrackManager::Allocate(){
	assert(trackManager == NULL);
	trackManager = new TrackManager();
}

void TrackManager::Deallocate(){
	assert(trackManager);
	delete trackManager;
}

TrackManager * TrackManager::Instance(){
	assert(trackManager);
	return trackManager;
}


// Pauses active tracks.
void TrackManager::Pause()
{
	for (int i = 0; i < activeTracks.Size(); ++i)
	{
		Track * track = activeTracks[i];
		track->Pause();
	}
}

// Resumes active tracks.
void TrackManager::Resume()
{
	for (int i = 0; i < activeTracks.Size(); ++i)
	{
		Track * track = activeTracks[i];
		track->Resume();
	}
}

/// Creates a new track-object, checking that the file exists and returns true upon success. No check is done to verify that it is a valid Audio file.
Track * TrackManager::CreateTrack(String name, String source, String category)
{
	// Check that the source exists first..?
	if (!FileExists(source))
	{
		// Try to add default track path.
		if (!source.Contains(defaultTrackPath))
		{
			source = defaultTrackPath + "/" + source;
			return CreateTrack(name, source, category);
		}
		std::cout<<"\nNo such file with source: "<<source;
		return NULL;
	}
	Track * track = new Track();
	track->name = name;
	track->source = source;
	tracks.Add(track);
	TrackCategory * tc = GetCategory(category);
	if (!tc){
		tc = new TrackCategory();
		tc->name = category;
		categories.Add(tc);
	}
	if (tc){
		tc->tracks.Add(track);
		track->category = tc;
	}
	return track;
}

Track * TrackManager::GetTrack(String byName){
	for (int i = 0; i < tracks.Size(); ++i)
		if (tracks[i]->name == byName)
			return tracks[i];
	return NULL;
}

List<Track*> TrackManager::GetTracks(){
	return tracks;
}

/// If any track is currently in the play-state.
bool TrackManager::IsPlaying()
{
	for (int i = 0; i < activeTracks.Size(); ++i)
	{
		Track * track = activeTracks[i];
		if (track->IsPlaying())
			return true;
	}
	return false;
}

/// Plays target track.
Track * TrackManager::PlayTrack(String trackNameOrSource)
{
	Track * targetTrack = NULL;
	for (int i = 0; i < tracks.Size(); ++i){
		Track * track = tracks[i];
		if (track->source == trackNameOrSource ||
			track->name == trackNameOrSource
			)
		{
			targetTrack = track;
			break;
		}
	}
	// Not found? Try to load it from source.
	if (!targetTrack)
		targetTrack = CreateTrack(trackNameOrSource, trackNameOrSource, "track");
	if (targetTrack)
	{
		Play(targetTrack);
//		track->Play();
//		OnTrackPlayed(track);
	}
	return targetTrack;
}

/// Attempts to play a track by name. Returns NULL if it was not found.
Track * TrackManager::PlayTrackByName(String trackName)
{
	for (int i = 0; i < tracks.Size(); ++i){
		Track * track = tracks[i];
		if (track->name == trackName)
		{
			Play(track);
			track->Play();
			OnTrackPlayed(track);
			return track;
		}
	}
	return NULL;
}

/// Plays a random track of specified category, returns the given track.
Track * TrackManager::PlayTrackFromCategory(String category, bool loop){
	TrackCategory * tc = GetCategory(category);
	if (!tc)
		return NULL;
	List<String> songs;
	Track * t = NULL;
	int tries = 100;
	srand(Timer::GetCurrentTimeMs());
	while (t == NULL){
		--tries;
		int randomSong = rand() % tc->tracks.Size();
		t = tc->tracks[randomSong];
		if (!t->enabled)
			t = NULL;
		if (tries < 0)
			return NULL;
	}
	t->Play();
	t->Loop(loop);
	return t;
}


Track * TrackManager::GetTrackBySource(String sourceUrl)
{
	for (int i = 0; i < tracks.Size(); ++i)
	{
		Track * track = tracks[i];
		if (track->source == sourceUrl)
			return track;
	}	
	return NULL;
}

TrackCategory * TrackManager::GetCategory(String byName){
	for (int i = 0; i < categories.Size(); ++i)
		if (categories[i]->name == byName)
			return categories[i];
	return NULL;
}


/// Called each time an audio object is to be deleted in order to remove references from it.
void TrackManager::OnAudioDeleted(Audio * audio)
{
	for (int i = 0; i < tracks.Size(); ++i)
	{
		Track * track = tracks[i];
		if (track->audio == audio)
			track->audio = NULL;
	}
}

/// Called to play a track
void TrackManager::Play(Track * track)
{
	// Stop all other active ones.
	while (activeTracks.Size()){
		Track * activeTrack = activeTracks[0];
		activeTrack->Stop();
		activeTracks.Remove(activeTrack);
	}
	track->Play();
	/// All old tracks, pause or stop them first.
	MesMan.QueueMessages("TrackPlayed:"+track->name);
	activeTracks.Add(track);

}

/// Called when a new track is being played. Makes sure messages are sent to all interested.
void TrackManager::OnTrackPlayed(Track * track)
{
}



