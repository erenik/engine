/// Emil Hedemalm
/// 2015-02-26
/// Mixer for PCM-based audio data.

#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

#include "List/List.h"

class Audio;

// For organizing internally in the mixer, making sure audio is buffered correctly.
#define ABM AudioBufferMarker
struct AudioBufferMarker 
{
	Audio * audio;
	int pcmQueueIndex; // index in the queue it was last buffered to.
};

class AudioMixer 
{
	AudioMixer();
	~AudioMixer();
public:
	static void Allocate();
	static void Deallocate();

	/// Buffers floating point [-1,1]-based PCM data.
	void BufferPCMFloat(Audio * audio, float * buffer, int samples, int channels);
	/// Buffers short [-32768,32767]-based PCM data.
	void BufferPCMShort(Audio * audio, short * buffer, int samples, int channels, float volume);

	/// Sends update to driver, moves back data and updates marker locations.
	void Update();

	/// Depends on current marker in buffer.
	int SamplesToBuffer(Audio * forAudio);

private:
	/// Sends current buffer to audio driver.
	void SendToDriver(int driverID);
	AudioBufferMarker * GetMarker(Audio * forAudio);

	/// 
//	char * pcmQueueC;
	
	// Default?
	float * pcmQueueF;
	int queueSampleTotal;

	List<AudioBufferMarker*> markers;

};

/// 1 mixer.
extern AudioMixer * mixer;

#endif
