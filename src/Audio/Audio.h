// Emil Hedemalm
// 2013-03-22

#include "AudioSettings.h"

#ifdef USE_AUDIO

#ifndef AUDIO_H
#define AUDIO_H

#include <Util.h>
#include "AudioTypes.h"

#include <AL/al.h>

class MultimediaStream;

namespace AudioState {
enum audioStatus {
	NOT_INITIALIZED,
	READY,
	PLAYING,
	PAUSED,
	ENDED,
	AUDIO_ERROR,
};};

enum audioSource {
	AUDIO_SOURCE_NULL, OGG_STREAM, WAV,
};

/// Buffer object that stores state of a buffer of RAW PCM data.
class AudioBuffer {
public:
	AudioBuffer();
	// AL id
	ALuint alBuffer;
	/// If it has gotten good new data.
	bool buffered;
	/// Whether it is queued or not?
};

class Audio {
	friend class AudioManager;
public:
	Audio();
	/// Creates a new audio object using given path.
	Audio(String path);
	Audio(char type, String name, bool repeat = false, float awesomeness = 1.0);
	~Audio();		// Destructor if needed?
    void Nullify(); // Sets default values.

	/// Generate audio source if not existing.
	void CreateALObjects();

    /// Loads the actual data. Retyrns false if faylure. ;^;
    bool Load();

	void Play();			// Begins/resumes playback
	bool IsPlaying();
	void Resume();			// Resumes ONLY if the audio was currently paused.
	void Pause();			// Pause at current time
	void Stop();			// Stops and brings currentTime to 0.
	// Buffers new data from underlying streams and pushes it into AL for playback.
	void Update();			

	int type;				// BGM, BGS, etc.
	bool repeat;			// If it should repeat or not
	float volume;			// Awesomeness-rating
	float currentTime;		// Current time when playing/pausing.

	String name;	// Name of the audio, short
	String path;	// Path for the target audio in question

	/// Why is this stream-object here?
    MultimediaStream * audioStream;

private:
	/// Unqueues all AL buffers. This is wanted when stopping or changing locations,... probably.
	void UnqueueBuffers();

	/// Flag for a workaround.
	bool playbackEnded;
	int state;			// Playing, paused, stopped, error, etc.
	/// Updates playback volume. 
	void UpdateVolume(float masterVolume);
	/// For disabling playback functions. Usually set by the AudioManager ONLY.
	static bool audioEnabled;

	/// Total volume as calculated with UpdateVolume. 
	float absoluteVolume;
	
    bool loaded;
	//TODO: add .wav plaback variables/classes
	
	/// Raw audio data array used when quering data from the underlying stream.
	char * bufferData;
	/// AL source, pretty much the ID of the sound in the AL-world.
	ALuint alSource;
	/// Buffers for buffering audio data. Amount of buffers should be stored in another variable.
	List<AudioBuffer*> audioBuffers;
	/// Queued buffers. Order in which they are queued is relevant!
	List<AudioBuffer*> queuedBuffers;
	/// For setting initial size of the bufferData array.
	static int defaultBufferSize;
};

#endif

#endif // USE_AUDIO
