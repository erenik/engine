// Emil Hedemalm
// 2014-02-16
/// Basic multimedia stream. Subclass to playback other stream-types.

#ifndef MULTIMEDIA_STREAM_H
#define MULTIMEDIA_STREAM_H

#include "String/AEString.h"
#include <iostream>
#include <Util.h>

/// Should include includes to OpenAL here.. or...?
#include "Audio/OpenAL.h"

/// Buffer sizes.. might need separate ones for audio and video...?
#define FOURKB		4096
#define AUDIO_BUFFER_SIZE (FOURKB * 64)

class Texture;
class Audio;

namespace StreamState {
enum streamStates {
	STREAM_ERROR,
	NOT_INITIALIZED, // Before opening a file.
	READY, // After being initialized only.
	PLAYING, // When Play() has been called.
	PAUSED, // When Pause() has been called.
	ENDED,	// When reaching end of file.
};
};

/// Basic multimedia stream. Subclass to playback various stream-types.
class MultimediaStream {
	friend class MultimediaManager;
public:
	// Default constructor, See types in MultimediaTypes.h
	MultimediaStream(int type);	
	virtual ~MultimediaStream();

	/// Attempts to open target file. Returns false upon failure.
    virtual bool Open(String path) = 0;
	/// Closes target file and stream.
	virtual void Close() = 0;

	/// Updates media time by checking how far the audio has played (easier to synchronize graphics to audio than reversed?)
	virtual void UpdateMediaTime();

	/// Current frame time of the media in milliseconds.
	int CurrentFrameTime();

	/// True after opening the file, false after any interaction.
	virtual bool IsReady() { return streamState == StreamState::READY; };
	/// Starts buffering of the stream. Nothing is done until this command has been executed successfully, following a call to Open.
	virtual bool Play();
	virtual bool IsPlaying() { return streamState == StreamState::PLAYING; };
	/// Sets pause state.
	virtual bool Pause(bool pauseLocally = true);    
	virtual bool IsPaused() { return streamState == StreamState::PAUSED; };
	virtual bool HasEnded() { return streamState == StreamState::ENDED; };
	/// Seeks to target time in milliseconds.
	virtual bool Seek(int toTime);
	/// Returns seconds each frame should consume on average.
	virtual float TimePerFrame();
	/** Streams the next frame. Does not perform any type of check that the frame is beyond current time, meaning the MultimediaManager has this responsibility!
		Frames to pass defines how many frames are to be evaluated. Default value is 1.
	*/
	virtual bool StreamNextFrame(int framesToPass = 1);
	/** Buffers audio data into buf, up to maximum of maxBytes. Returns amount of bytes buffered. 
		Amount of bytes may reach 0 even if the media has not ended. Returns -1 on error. Returns -2 if the stream is paused.
		If loop is set to true it will try to automatically seek to the beginning when it reaches the end of the file.
	*/
	virtual int BufferAudio(char * buf, int maxBytes, bool loop) = 0;

	/** Returns a texture which will be updated as the stream progresses. 
		If the stream lacks graphics NULL will be returned.
		If the texture has the rebufferize flag set it has to be re-buffered to gain new frame data.
	*/
	virtual Texture * GetFrameTexture();

	// Path we're streaming from.
	String path;

	/// If flagged, will restart once ended.
	bool loop;
	/// Returns the type of the stream. See MultimediaTypes.h. 
	int Type();

	/// Sets relative level.
	void SetVolume(float level);
	/// Returns amount of channels present in the audio stream.
	virtual int AudioChannels();
	/// Gets frequency of the audio. This is typically 48000 or similar?
	virtual int AudioFrequency();

	/// Name of the stream. Usually set to the file-name.
	String name;

	/// Since audio is buffered separately, time can be obtained here in seconds.
	virtual double AudioTime() {return audioTime;};

protected:	
	// Audio track handle.
	Audio * audio;

	/// Current frame we have avaiable for rendering.
	int currentFrame;
	/// FPS in float.
	float fps;
	/// Frames per second for video in nominator and denominator parts.
	int fpsNom, fpsDenom;
	/// Media time in milliseconds.
	long long mediaTime;
	/// Time we last called Update();
	long long lastUpdate;
	/// Time in milliseconds when the media started.
	long long startTime;

	/// Since audio is buffered separately, store its time here.
	double audioTime;
	/// Relative volume.
	float volume;
	/// flagged if the stream has audio data.
	bool hasAudio;
	/// Flagged if the stream has video data.
	bool hasVideo;
	
	/// See streamStates above. Initial value is StreamState::NOT_INITIALIZED
	int streamState;

	/// Audio-specific data.
	/// Channels
	int audioChannels;
	/// Audio frequency, or samples per second, usually 44.1 kHz or some similar rather high value.
	int samplesPerSecond;


private:

	/// Type, see MultimediaTypes.h
	int type;
};

#endif
