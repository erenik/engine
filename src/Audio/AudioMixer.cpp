/// Emil Hedemalm
/// 2015-02-26
/// Mixer for PCM-based audio data.

#include "AudioMixer.h"
#include "Globals.h"
#include "DataTypes.h"
#include "Windows/WindowsCoreAudio.h"

extern AudioMixer * mixer = NULL;

AudioMixer::AudioMixer()
{
	// Default? 100k samples
	queueSampleTotal = 100000;
	pcmQueueF = new float[queueSampleTotal];
	
	/// Initialize buffah.
	memset(pcmQueueF, 0, queueSampleTotal * sizeof(pcmQueueF));
}

AudioMixer::~AudioMixer()
{
	SAFE_DELETE_ARR(pcmQueueF);
}

void AudioMixer::Allocate()
{
	assert(mixer == NULL);
	mixer = new AudioMixer();
}
void AudioMixer::Deallocate()
{
	SAFE_DELETE(mixer);
}

/// Buffers floating point [-1,1]-based PCM data.
void AudioMixer::BufferPCMFloat(Audio * audio, float * buffer, int samples, int channels)
{

}
/// Buffers short [-32768,32767]-based PCM data.
void AudioMixer::BufferPCMShort(Audio * audio, short * buffer, int samples, int channels, float volume)
{
	// Check starting point based on existing markers.
	ABM * abm = GetMarker(audio);
	// Add new marker if needed.
	if (!abm)
	{
		abm = new ABM();
		abm->audio = audio;
		abm->pcmQueueIndex = 0;
		markers.Add(abm);
	}
	abm->pcmQueueIndex = 0;
	memset(pcmQueueF, 0, queueSampleTotal * sizeof(float));
	// For each sample they want to buffer...
	for (int i = 0; i < samples; ++i)
	{
		// Set 0 so we avoid some noise? No.
		int floatsProcessed = 0;

		// Debug
		uchar * buf = (uchar*)buffer;
		int sample1 = (uchar)buf[i * 2];
		int sample2 = (uchar)buf[i * 2 + 1];

		/// These 2 should hopefully be the same..
		int sampleC =  sample1 | (sample2 << 8);
		int sample = buffer[i];
		short sampleCS = sampleC;
		assert(sample == sampleCS);
		// Convert to float.
		float f;
		/// 0 to 32767 is the same, over that and we go negative. -> simulate going from int to short.
	//	int overHalf = (sample & (1 << 15)) ? 1 : 0;
	//	int iSample = (sample % 32768) - 32768 * overHalf;
		short shortSample = buffer[i];
		f = ((float) shortSample) / (float) 32768;
		if( f > 1) 
			f = 1;
		if( f < -1 ) 
			f = -1;
		// Multiply by volume?
		f *= volume;
		pcmQueueF[abm->pcmQueueIndex + i] += f;

		if (debug == -17)
			std::cout<<"\n"<<pcmQueueF[abm->pcmQueueIndex + i]<<" sample: "<<sample<<" sample1/2: "<<sample1<<"/"<<sample2;
		++floatsProcessed;
//		++floatsProcessed;
	}
	/// Increase index?

	/// Buffer it?
	WMMDevice * device = WMMDevice::MainOutput();
	//	int bytesToBuffer = device->BytesToBuffer();
	int samplesLeft = samples;
	int samplesSent = 0;
	while(samplesLeft > 0)
	{
		int samplesToBuffer = device->SamplesToBuffer();
		if (samplesToBuffer <= 0)
		{
			Sleep(5);
			continue;
		}
		device->BufferData(((char*)pcmQueueF)+ samplesSent * sizeof(float), samples * sizeof(float));
		samplesLeft -= samplesToBuffer;
		samplesSent += samplesToBuffer;
	}

}

/// Sends update to driver, moves back data and updates marker locations.
void AudioMixer::Update()
{

}

/// Depends on current marker in buffer.
int AudioMixer::SamplesToBuffer(Audio * forAudio)
{
	ABM * abm = GetMarker(forAudio);
	if (!abm)
		// Full size possible
		return queueSampleTotal;
	/// Subset based on current index -> how far we have buffered last frame counting
	return queueSampleTotal - abm->pcmQueueIndex - 1;
}

/// Sends current buffer to audio driver.
void AudioMixer::SendToDriver(int driverID)
{

}




AudioBufferMarker * AudioMixer::GetMarker(Audio * forAudio)
{
	for (int i = 0; i < markers.Size(); ++i)
	{
		AudioBufferMarker * abm = markers[i];
		if (abm->audio == forAudio)
			return abm;
	}
	return NULL;
}
