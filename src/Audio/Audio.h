// Emil Hedemalm
// 2013-03-22

#include "AudioSettings.h"

#ifdef USE_AUDIO

#ifndef AUDIO_H
#define AUDIO_H

#include <Util.h>
#include "AudioTypes.h"

#include "OpenAL.h"

class MultimediaStream;

namespace AudioState {
enum audioStatus {
	NOT_INITIALIZED,
	READY,
	PLAYING,
	PAUSED,
	ENDING, // While playing on the last buffer.
	ENDED,
	AUDIO_ERROR,
};};

enum audioSource {
	AUDIO_SOURCE_NULL, OGG_STREAM, WAV,
};



class Audio {
	friend class AudioManager;
public:
	Audio();
	/// Creates a new audio object using given path.
	Audio(String path);
	Audio(char type, String source, bool repeat = false, float awesomeness = 1.0);
	~Audio();		// Destructor if needed?
    void Nullify(); // Sets default values.

	// Yer.
	void BufferData(MultimediaStream * fromStream, AudioBuffer * intoBuffer);

	/// Generate audio source if not existing.
	void CreateALObjects();

    /// Loads the actual data. Retyrns false if faylure. ;^;
    bool Load();

	void Play();			// Begins/resumes playback
	bool IsPlaying();
	void Resume();			// Resumes ONLY if the audio was currently paused.
	void Pause();			// Pause at current time
	void Stop(bool andSeekToStart);			// Stops and brings currentTime to 0.
	// Buffers new data from underlying streams and pushes it into AL for playback.
	void Update();			

	int type;				// BGM, BGS, etc.
	bool repeat;			// If it should repeat or not
	float volume;			// Awesomeness-rating
	
	// Actually quite bothersome, but AL will count the time that the buffers have been active.
	float currentBufferTime;
	float bufferDuration;
	double totalDurationBuffered;
	int buffersPassed;

	String name;	// Name of the audio, short
	String path;	// Path for the target audio in question

	/// Why is this stream-object here?
    MultimediaStream * audioStream;

	/// Playback time in milliseconds.
	long long PlaybackTimeMs();

	/** If true, deletion should be performed upon finished playback of this audio-clip. 
		Recommended for "fire-and-forget" sound effects.
	*/
	bool deleteOnEnd;

	/// Default directory from where audio is assumed to be stored. This to help build paths to find audio straight away?
	static String audioDirectory;
private:

	void QueueBuffer(AudioBuffer * buf);
	void UnqueueBuffer(AudioBuffer * buf);

	/// Bad boolean, should go away after audio is re-worked with an own messaging system.
	bool firstBufferDone;

	// Playback time in seconds.
	float playbackTime;


	/// Unqueues all AL buffers. This is wanted when stopping or changing locations,... probably.
	void UnqueueBuffers();

	/// Flag for a workaround.
	bool playbackEnded;
	int state;			// Playing, paused, stopped, error, etc.
	/// Updates playback volume. 
	void UpdateVolume(float masterVolume);

	/// Total volume as calculated with UpdateVolume. 
	float absoluteVolume;
	
    bool loaded;
	//TODO: add .wav plaback variables/classes
	
	/// AL source, pretty much the ID of the sound in the AL-world.
	unsigned int alSource; // (ALuint)

	/// Buffers for buffering audio data. Amount of buffers should be stored in another variable.
	List<AudioBuffer*> audioBuffers;
	/// Queued buffers. Order in which they are queued is relevant!
	List<AudioBuffer*> queuedBuffers;
};

#endif

#endif // USE_AUDIO
