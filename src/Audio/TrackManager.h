/// Emil Hedemalm
/// 2014-01-07
/// A manager for handling sets of tracks, and ensuring playback as needed.

#ifndef TRACK_MANAGER_H
#define TRACK_MANAGER_H

#include <List/List.h>

#define TrackMan (*TrackManager::Instance())

class Track;
class Audio;

class TrackCategory {
public:
	String name;
	List<Track*> tracks;
};

class TrackManager {
	TrackManager();
	~TrackManager();
	static TrackManager * trackManager;
public:
	static void Allocate();
	static void Deallocate();
	static TrackManager * Instance();

	// Pauses active tracks.
	void Pause();
	// Resumes active tracks.
	void Resume();
	/// Creates a new track-object, checking that the file exists and returns true upon success. No check is done to verify that it is a valid Audio file.
	Track * CreateTrack(String name, String source, String category = "");
	Track * GetTrack(String byName);
	List<Track*> GetTracks();

	/// If any track is currently in the play-state.
	bool IsPlaying();

	/// Called to play a track
	void Play(Track * track);

	/// Plays target track.
	Track * PlayTrack(String trackNameOrSource);
	/// Attempts to play a track by name. Returns NULL if it was not found.
	Track * PlayTrackByName(String trackName);
	/// Plays a random track of specified category, returns the given track.
	Track * PlayTrackFromCategory(String category, bool loop);
	/// Getters
	Track * GetTrackBySource(String sourceUrl);
	TrackCategory * GetCategory(String byName);
	/// Called each time an audio object is to be deleted in order to remove references from it.
	void OnAudioDeleted(Audio * audio);
private:
	/// Called when a new track is being played. Makes sure messages are sent to all interested.
	void OnTrackPlayed(Track * track);

	/// All currently played tracks. To enable cross-fading and pausing/stopping as needed.
	List<Track*> activeTracks;
	List<Track*> tracks;
	List<TrackCategory*> categories;
};

#endif