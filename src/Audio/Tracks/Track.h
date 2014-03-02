/// Emil Hedemalm
/// 2014-01-07
/// Overlay class for the handling audio, abstracted to the user's level.

#ifndef TRACK_H
#define TRACK_H

#include <String/AEString.h>
class TrackCategory;

class Audio;

class Track {
	friend class TrackManager;
	Track();
public:
	void Play();
	bool IsPlaying();
	void Pause();
	void Resume();
	void Stop();
	/// Sets the track to start/stop looping.
	void Loop(bool value);
	String name;
	String source;
	bool enabled;
	bool loop;
	TrackCategory * category;
private:
	/// Audio object returned upon initial playback.
	Audio * audio;
};

#endif