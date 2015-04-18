/// Emil Hedemalm
/// 2015-02-26
/// Mixer for PCM-based audio data.

#include "AudioMixer.h"
#include "Globals.h"
#include "DataTypes.h"
#include "Windows/WindowsCoreAudio.h"
#include "AudioManager.h"

extern AudioMixer * masterMixer;

AudioMixer::AudioMixer()
{
	// Default? 100k samples
	queueSampleTotal = 1024 * 16;
	pcmQueueF = new float[queueSampleTotal];
	
	/// Initialize buffah.
	memset(pcmQueueF, 0, queueSampleTotal * sizeof(pcmQueueF));
	muted = false;
	volume = 1.f;
}

AudioMixer::~AudioMixer()
{
	SAFE_DELETE_ARR(pcmQueueF);
	markers.ClearAndDelete();
}

void AudioMixer::AllocateMaster()
{
	assert(masterMixer == NULL);
	masterMixer = new AudioMixer();
}
void AudioMixer::DeallocateMaster()
{
	SAFE_DELETE(masterMixer);
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
	/// Check possible samples to buffer.
	int possibleSamples = queueSampleTotal - abm->pcmQueueIndex;
	assert(samples <= possibleSamples);

	if (debug == -22)
		PrintQueue("At end of marker position before filling it", abm->pcmQueueIndex - 5, abm->pcmQueueIndex + 10 - 5);

//	abm->pcmQueueIndex = 0;
//	memset(pcmQueueF, 0, queueSampleTotal * sizeof(float));
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
	if (debug == -22)
		PrintQueue("Start of where we filled data (-5,+5), should have values for all 10", abm->pcmQueueIndex - 5, abm->pcmQueueIndex + 5);
	abm->pcmQueueIndex += samples;
	if (debug == -22)
		PrintQueue("End of marker position after filling it", abm->pcmQueueIndex -5, abm->pcmQueueIndex + 10 - 5);
}

/// Sends update to driver, moves back data and updates marker locations.
void AudioMixer::Update()
{
	SendToDriver(audioDriver);
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
	if (driverID != AudioDriver::WindowsCoreAudio)
		return;
#ifdef WINDOWS
	/// Buffer it?
	WMMDevice * device = WMMDevice::MainOutput();
	if (!device)
		return;
	//	int bytesToBuffer = device->BytesToBuffer();
	/// Always send a constant amount of samples, based on the mixer's buffer-size?
	int defaultSamplesToSend = queueSampleTotal * 0.1;
	/// Check availability in driver.
	int driverSamplesToBuffer = device->SamplesToBuffer();
	// Use the lower value.
	int samplesToSend = min(defaultSamplesToSend, driverSamplesToBuffer);
	if (samplesToSend <= 0)
	{
		SleepThread(5);
		return;
	}
	// Calc volume.
	float currentVolume = volume;
	if (muted)
		currentVolume = 0.f;
	/// Apply master volume RIGHT before sending to device! o.o
	for (int i = 0; i < samplesToSend; ++i)
	{
		pcmQueueF[i] *= currentVolume;
	}
	int bytesBuffered = device->BufferData((char*)pcmQueueF, samplesToSend * sizeof(float));
	int bytesPerSample = sizeof(float);
	int samplesBuffered = bytesBuffered / bytesPerSample;

	/// Move back existing data.
	int samplesRemainingInQueue = queueSampleTotal - samplesBuffered;
	if (debug == -22)
		PrintQueue("End of samples buffered, this part should be moved back to 0", samplesBuffered, samplesBuffered + 10);
	memmove(pcmQueueF, pcmQueueF + samplesBuffered, samplesRemainingInQueue * bytesPerSample);
	if (debug == -22)
		PrintQueue("From 0, post movement, should be the same as the above", 0, 10);
	// Nullify the newly added end part of the buffer
	memset(pcmQueueF + samplesRemainingInQueue, 0, samplesBuffered * bytesPerSample);
	if (debug == -22)
		PrintQueue("From after samplesBuffered, should be 0s", samplesBuffered, samplesBuffered + 10);

	// Move back all markers too.
	for (int i = 0; i < markers.Size(); ++i)
	{
		ABM * abm = markers[i];
		abm->pcmQueueIndex -= samplesBuffered;
		if (abm->pcmQueueIndex < 0)
		{
			markers.RemoveIndex(i);
			--i;
			delete abm;
		}
	}
#endif
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

void AudioMixer::PrintQueue(String text, int fromIndex, int toIndex)
{
	std::cout<<"\n"<<text;
	for (int i = fromIndex; i < toIndex; ++i)
	{
		std::cout<<"\nAQ "<<i<<": "<<pcmQueueF[i];
	}
}
