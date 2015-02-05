// Emil Hedemalm
// 2013-03-22

#include "Audio.h"
#include "ALDebug.h"
#include "AudioManager.h"

#include "Multimedia/MultimediaStream.h"
#include "Multimedia/MultimediaTypes.h"
#include "Multimedia/Ogg/OggStream.h"
#include "Multimedia/Wav/WavStream.h"

#include "File/FileUtil.h"

#include <cmath>

#include "OpenAL.h"

#include "File/LogFile.h"


//#include "../globals.h"
#include <iostream>
#include <fstream>

//
//enum audioStatus {
//	AUDIO_NULL,
//	AUDIO_PLAYING,
//	AUDIO_PAUSED,
//	AUDIO_STOPPED,
//	AUDIO_ERROR,
//};

#define audioEnabled (AudioMan.AudioEnabled())

String Audio::audioDirectory = "sound";

// Default constructor
Audio::Audio()
{
    Nullify();
}

void Audio::Nullify()
{
	volume = 1.0f;
    loaded = false;
	type = AudioType::NONE;
	state = AudioState::NOT_INITIALIZED;
	audioStream = NULL;
	absoluteVolume = 1.0f;

#ifdef OPENAL
	alSource = 0;
#endif
	audioBuffers.ClearAndDelete();

	playbackEnded = false;

	playbackTime = 0;
	bufferDuration = 0;
	buffersPassed = 0;
	totalDurationBuffered = 0;

	deleteOnEnd = false;

	firstBufferDone = false;
}

/// Generate audio source if not existing.
bool Audio::CreateALObjects()
{
	lastAudioInfo = "Audio::CreateALObjects";
#ifdef OPENAL
	/// Skip is source is valid already.
	if (alSource)
	{
		std::cout<<"\nalSource already created.";
		return true;
	}
	alSource = ALSource::New();
	// Throw an error if we can't generate a source.
	if (alSource == 0)		
	{
		LogAudio("Unable to create ALSource for audio: "+this->path, ERROR);
		return false;
	}
	// Create audio buffers.
	assert(audioBuffers.Size() == 0);
	for (int i = 0; i < 2; ++i)
	{
		AudioBuffer * audioBuffer = AudioBuffer::New();
		audioBuffers.Add(audioBuffer);
	}
//	std::cout<<"\nALSource generated: "<<alSource;
	assert(alSource > 0);
	/// Set some basic properties for the audio.
	alSource3f(alSource, AL_POSITION,        0.0, 0.0, 0.0);
	alSource3f(alSource, AL_VELOCITY,        0.0, 0.0, 0.0);
	alSource3f(alSource, AL_DIRECTION,       0.0, 0.0, 0.0);
	alSourcef (alSource, AL_ROLLOFF_FACTOR,  0.0          );
	alSourcei (alSource, AL_SOURCE_RELATIVE, AL_TRUE      );
#endif
	lastAudioInfo = "Audio::CreateALObjects - success";
	return true;
}

/// Destructor that deletes AL resources as well as other thingies!
Audio::~Audio()
{
#ifdef OPENAL
	int error = 0;
	/// Deallocate al IDs.
    if (alSource != 0)
	{
		// Stop!
        alSourceStop(alSource);
		// set 0 buffer so that they may be deleted. <- wat...
//		alSourcei(alSource, AL_BUFFER, 0);
		AssertALError();
			
		// Unqueue and delete all audio buffers buffers.
		while(audioBuffers.Size())
		{
			AudioBuffer * audioBuffer = audioBuffers[0];
			// Unqueue it so that it may be freed later.
			if (audioBuffer->attached)
				UnqueueBuffer(audioBuffer);
			// Free the buffers.
			audioBuffer->Free();
			AssertALError();
			audioBuffers.Remove(audioBuffer);
//			delete audioBuffer;
		}

		ALSource::Free(alSource);
		AssertALError();
	}
#endif
	
    if (audioStream){
        audioStream->Close();
        delete audioStream;
        audioStream = NULL;
    }
}


	/// Creates a new audio object using given path.
Audio::Audio(String i_path)
{
	Nullify();
	name = path = i_path;
}

// Regular constructor
Audio::Audio(char i_type, String source, bool i_repeat, float audioVolume)
{
    Nullify();
	type = i_type;
	name = source;
	
	path = source;
	/*
		if (!path.Contains(Audio::audioDirectory))
		{
			String newPath;
			newPath = "sound";
			switch(type){
				case AudioType::BGM: newPath += "/bgm/"; break;
				case AudioType::BGS: newPath += "/bgs/"; break;
				case AudioType::SFX: newPath += "/sfx/"; break;
				case AudioType::UIS: newPath += "/uis/"; break;
				case AudioType::SPEECH: newPath += "/spc/"; break;
			}
			newPath += name;
			path = newPath;
		}
	}
	*/
	// Check that the file exists maybe?
	state = AudioState::AUDIO_ERROR;
	repeat = i_repeat;
	volume = audioVolume;
}

bool Audio::Load()
{
    assert(!loaded);
    assert(audioStream == NULL);
//    std::cout<<"\nAudio::Load for "<<name<<" assumed to be located at "<<path;

	if (!FileExists(path))
	{
		std::cout<<"\nERROR: File does not exist. Returning.";    
		return false;
	}

    // Stream
	if (name.Contains(".ogg") ||
		name.Contains(".opus"))
	{
//        std::cout<<"\nFile deemed of type Ogg Vorbis, trying to load.";
#ifdef BUILD_OGG
		audioStream = new OggStream();
		loaded = audioStream->Open(path.c_str());
		assert(loaded);	
#endif
	}
	else if (name.Contains(".wav"))
	{
	  //  std::cout<<"\nFile deemed of type Waveform Audio File Format, trying to load.";
        // Load wavvy
		audioStream = new WavStream();
		loaded = audioStream->Open(path);
		assert(loaded);
	}
	else {
        assert(false && "Unsupported audio-format I'm afraid!");
	}
	audioStream->loop = repeat;
	lastAudioInfo = "Audio::Load for path: "+path; 
    return loaded;
}


/** Begins playing ze sound! For BGMs, mute the others by default (might make this togglable)
*/
void Audio::Play()
{
	if (!audioEnabled)
		return;

	/// Generate buffers, sources, etc. if not already done so!
	if (alSource == 0){
		bool ok = CreateALObjects();
		if (!ok)
			return;
	}

//    std::cout<<"\nPlaying audio "<<name;
//	assert(audioStream->Type() != MultimediaType::UNKNOWN);
	/// Check type
	switch(this->type)
	{
		case AudioType::BGM:
		case AudioType::BGS: 
		{
			// Pause all others.
			AudioMan.StopAllOfType(this->type);
			break;
		}
		default:
			break;
	}

#ifdef OPENAL
	int channels = audioStream->AudioChannels();
    ALuint format;
	if(channels == 1)
        format = AL_FORMAT_MONO16;
    else
        format = AL_FORMAT_STEREO16;

	/// Stream source is reloaded below anyway, so skip this check.
//	assert(audioStream->source > 0);

	/// Set volume..?
	UpdateVolume(AudioMan.MasterVolume());
	/// Play it!
	alSourcePlay(alSource);
	int error = alGetError();
	switch(error){
		case AL_NO_ERROR:
			state = AudioState::PLAYING;
			break;
		default:
			std::cout<<"Error playing audio.";

	}
#endif
	playbackEnded = false;
}

bool Audio::IsPlaying()
{
	return state == AudioState::PLAYING;
}

// Resumes ONLY if the audio was currently paused.
void Audio::Resume()
{
	/// Set volume..?
	UpdateVolume(AudioMan.MasterVolume());
#ifdef OPENAL
	/// Play it!
	alSourcePlay(alSource);
	int error = alGetError();
	switch(error){
		case AL_NO_ERROR:
			state = AudioState::PLAYING;
			break;
		default:
			std::cout<<"Error playing audio.";

	}
#endif
	playbackEnded = false;
}

void Audio::Pause()
{
#ifdef OPENAL
	alSourcePause(alSource);
#endif
	state = AudioState::PAUSED;
	// TODO: save time
//	currentTime = stream.
}

void Audio::Stop(bool andSeekToStart)
{
#ifdef OPENAL
	// Stahp!
	alSourceStop(alSource);
	if (andSeekToStart)
	{
		// Seek to start.
		audioStream->Seek(0);
	}
	// Clear current audio buffers.
	UnqueueBuffers();
#endif
	// And mark our new state.
	state = AudioState::READY;
}

void Audio::QueueBuffer(AudioBuffer * buf)
{
	assert(!buf->attached);
	bool ok = buf->AttachTo(alSource);
	assert(ok);
	queuedBuffers.Add(buf);
}

void Audio::UnqueueBuffer(AudioBuffer * buf)
{
	assert(buf->attached);
	bool ok = buf->DetachFrom(alSource);
	assert(ok);
	queuedBuffers.Remove(buf, ListOption::RETAIN_ORDER);
	/// Assume the buffer should get new data after unqueuing it.
	buf->buffered = false;
}

// Buffers new data from underlying streams and pushes it into AL for playback.
void Audio::Update()
{
	if (!audioEnabled)
		return;

#ifdef OPENAL
	// If ending, just wait until the al State reaches STOPPED.
	if (state == AudioState::ENDING)
	{
		ALint alSourceState = 0;
		alGetSourcei(alSource, AL_SOURCE_STATE, &alSourceState);
		switch(alSourceState){
			case AL_INITIAL: 
			case AL_PLAYING: 
				return;
			case AL_PAUSED:
			case AL_STOPPED:
			{
				state = AudioState::ENDED;
				playbackEnded = true;
				return;
			}
		}
	}

	// Check if playing?
	if (state != AudioState::PLAYING)
		return;

	// Check if first buffering is done yet.
	if (!firstBufferDone)
	{
		for (int i = 0; i < audioBuffers.Size(); ++i)
		{
			AudioBuffer * audioBuffer = audioBuffers[i];
			BufferData(audioStream, audioBuffer);
			if (audioBuffer->buffered)
			{
				// Queue it.
				QueueBuffer(audioBuffer);
//				alSourceQueueBuffers(alSource, 1, &audioBuffer->alBuffer);
			}
		}
		firstBufferDone = true;
		// Play it!
		alSourcePlay(alSource);
		return;
	}

	// Update time.
	playbackTime;
	float currentBufferTime;
	alGetSourcef(alSource, AL_SEC_OFFSET, &currentBufferTime);
	static float maxBufferTime = 0;
	if (maxBufferTime < currentBufferTime)
		maxBufferTime = currentBufferTime;
	playbackTime = buffersPassed * bufferDuration + currentBufferTime;
	
//	playbackTime = audioStream->AudioTime() + currentBufferTime;
//	playbackTime = totalDurationBuffered;

	ALint processed = 0, queued = 0;
    bool success = true;

	ALint alSourceState = 0;
	alGetSourcei(alSource, AL_SOURCE_STATE, &alSourceState);
	switch(alSourceState){
		case AL_INITIAL: break;
		case AL_PLAYING: break;
		case AL_PAUSED:
			/// Set state to paused?
			state = AudioState::PAUSED;
			return;
		case AL_STOPPED:
		{
			/// It has played decently before, so its at the end now.
			if (playbackEnded){
				state = AudioState::ENDED;
				alSourceStop(alSource);
				return;
			}

			// Check playback time
		//	std::cout<<"\nAudio stopped! Maybe it ran out of data to play back? Playing it again unless stream we depend on has ended!";
			alSourcePlay(alSource);
			CheckALError();
		}
	}

	/// Set volume
//	alSourcef(alSource, AL_GAIN, 1.0f);

	/// Check queued buffers
	alGetSourcei(alSource, AL_BUFFERS_QUEUED, &queued);
	CheckALError("Error getting source statistics for alSource: " + String::ToString((int)alSource)+ " and queued: "+String::ToString(queued));
	/// Check processed buffers.
	alGetSourcei(alSource, AL_BUFFERS_PROCESSED, &processed);
	CheckALError("Error getting source statistics for alSource: " + String::ToString((int)alSource)+ " and processed: "+String::ToString(processed));
//	assert(queuedBuffers.Size() >= processed + queued);
		
	/// For each processed buffer, unqueue it
	while(processed)
	{
		assert(queuedBuffers.Size());
		// Fetch the index 0 buffer, should be the one.
		AudioBuffer * buf = queuedBuffers[0];
		UnqueueBuffer(buf);
		/*
		ALuint alBuffer;
		/// Check if buffer is still queued?
//		alGetBufferi(alBuffer, AL_BUFFER_

        alSourceUnqueueBuffers(alSource, 1, &alBuffer);
	//	std::cout<<"\nBuffer unqueued: "<<alBuffer;
	    CheckALError("Error Unquequeing bufferrs");
		for (int i = 0; i < this->queuedBuffers.Size(); ++i){
			AudioBuffer * queuedBuffer = queuedBuffers[i];
			if (alBuffer == queuedBuffer->alBuffer){
				// Unqueue it
				queuedBuffers.Remove(queuedBuffer);
				queuedBuffer->buffered = false;
			}
		}*/
		--processed;
		/// Increment known buffers passed so we know how far we have played the track.
		buffersPassed++;
	}
	/// For each unqueued buffer, stream more data and queue it.
	List<AudioBuffer*> unqueuedBuffers = audioBuffers - queuedBuffers;
	for (int i = 0; i < unqueuedBuffers.Size(); ++i)
	{
		AudioBuffer * audioBuffer = unqueuedBuffers[i];
		BufferData(audioStream, audioBuffer);

		// Don't queue the buffer if streaming failed, since it's old data, yo.
		if (audioBuffer->buffered)
		{
			QueueBuffer(audioBuffer);
		}
	}
#endif
    return;
}

/// Playback time in milliseconds.
long long Audio::PlaybackTimeMs()
{
	return playbackTime * 1000;
}

/// Unqueues all AL buffers. This is wanted when stopping or changing locations,... probably.
void Audio::UnqueueBuffers()
{
	while(queuedBuffers.Size())
	{
		AudioBuffer * buf = queuedBuffers[0];
		UnqueueBuffer(buf);
	}
}

/// Updates playback volume.
void Audio::UpdateVolume(float masterVolume)
{
	absoluteVolume = this->volume * masterVolume;
//	std::cout<<"\nUpdating volume: "<<absoluteVolume<<" for source "<<alSource;
	/// Set volume in AL.
//	std::cout<<"\nVolume updated: "<<absoluteVolume;
	
#ifdef OPENAL
	assert(alSource != 0);
	alSourcef(alSource, AL_GAIN, absoluteVolume);
	int error = alGetError();
	switch(error){
		case AL_NO_ERROR:
			break;
		default:
			std::cout<<"\nALError in Audio::UpdateVolume";
			break;
	}
#endif
}


void Audio::BufferData(MultimediaStream * fromStream, AudioBuffer * intoBuffer)
{
	try {
		/// Replace with a call to stream audio data from the stream.
		assert(audioStream);

		#define FOURKB		4096
		#define BUFFER_SIZE (FOURKB * 64)

		char buf[BUFFER_SIZE];
		int channels = audioStream->AudioChannels();
		int format = 0;
		int frequency = audioStream->AudioFrequency();
		if (frequency < 0 || frequency > 1000000)
		{
			std::cout<<"\nAudio sampling rate/frequency outside allowed range.";
			assert(frequency > 0 && frequency < 1000000);
			return;
		}
		int bytesBuffered = audioStream->BufferAudio(buf, BUFFER_SIZE, repeat);
//		std::cout<<"\nBytes buffered: "<<bytesBuffered;
			
		float durationBuffered;
		int numberOfSampleFrames;
		int sampleRate = frequency;
#ifdef OPENAL
		switch(channels)
		{
			case 1: 
				format = AL_FORMAT_MONO16; 
				numberOfSampleFrames = bytesBuffered * 0.5;
				break;
			case 2: 
				format = AL_FORMAT_STEREO16; 
				numberOfSampleFrames = bytesBuffered * 0.25;
				break;
		}
#endif
		durationBuffered = numberOfSampleFrames / (float)sampleRate;
		bufferDuration = durationBuffered;
		totalDurationBuffered += durationBuffered;

		int bytesPerSample = 2;
		int divisible = bytesPerSample * channels;
		int rest = bytesBuffered % divisible;
		assert(rest == 0);

	/*			// If unable to buffer data, assume we are at the end of the file. If so, check if we can re-start the file (assuming looping is enabled)
		if (bytesBuffered <= 0 && repeat){
			audioStream->Seek(0);
			bytesBuffered = audioStream->BufferAudio(buf, BUFFER_SIZE);
		}
	*/
		if (bytesBuffered <= 0)
		{
		//	std::cout<<"\nUnable to buffer audio data! Assuming at end of stream.";
			state = AudioState::ENDING;
			intoBuffer->buffered = false;
			return;
		}
	//		std::cout<<"\nBuffer more PCM data: "<<bytesBuffered;
//		std::cout<<" Format: "<<format<<" frequency: "<<frequency;
		/// Samples per second yo....
	//	bytesBuffered = 262144;

		int bytesPerSampleTimesChannels = bytesPerSample * channels;
		int samples = bytesBuffered / bytesPerSample;

		// Flip the bytes?
		bool flipBytes = false;
		if (flipBytes)
		{
			for (int i = 0; i < bytesBuffered; i += bytesPerSample)
			{
				uchar tmp = buf[i];
				buf[i] = buf[i+1];
				buf[i+1] = tmp;
			}
		}
		bool nullifyLeft = false;
		if (nullifyLeft)
		{
			for (int i = 0; i < bytesBuffered; i += bytesPerSampleTimesChannels)
			{
				buf[i] = 0;
				buf[i+1] = 0;
			}
		}
		bool nullifyRight = false;
		if (nullifyRight)
		{
			for (int i = bytesPerSample; i < bytesBuffered; i += bytesPerSampleTimesChannels)
			{
				buf[i] = 0;
				buf[i+1] = 0;
			}
		}

		/// Sine-wave, please.
		bool sine = false;
		short * shortBuf = (short*) buf;
		static int sines = 0;
		if (sine)
		{
			for (int i = 0; i < samples; i += channels)
			{
				sines++;
				int sinV = sin(sines * 0.1f) * SHRT_MAX;
				shortBuf[i] = sinV;
//				int sinV2 = sin(sines * 0.2f) * SHRT_MAX;
	//			shortBuf[i+1] = sinV2;
//				std::cout<<"\nSin value: "<<sinV;
			}
		}
#ifdef OPENAL
		CheckALError("Before buffering");
//		frequency *= 0.5f;
		alBufferData(intoBuffer->alBuffer, format, buf, bytesBuffered, frequency);
		int error = alGetError();
		switch(error){
			case AL_NO_ERROR:
				/// Mark the buffer as buffered.
				intoBuffer->buffered = true;
				break;
			default:
				std::cout<<"\nError buffering AL Data";
				PrintALError(error);
				intoBuffer->buffered = false;
				break;
		}
#endif
	}
	catch (...){
		std::cout<<"\nStreaming failed, aborting.";
		return;
	}
}
