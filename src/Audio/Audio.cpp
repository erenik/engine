// Emil Hedemalm
// 2013-03-22

#include "AudioSettings.h"

#ifdef USE_AUDIO

#include "Audio.h"
#include "ALDebug.h"
#include "AudioManager.h"
#include "Multimedia/MultimediaStream.h"
#include "Multimedia/MultimediaTypes.h"
#include "Multimedia/Ogg/OggStream.h"

#define BUILD_AL
#ifdef BUILD_AL
#include <AL/al.h>
#include <AL/alc.h>
// #include <AL/alut.h>
#endif


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



AudioBuffer::AudioBuffer()
{
	alBuffer = 0;
	buffered = false;
}

bool Audio::audioEnabled = true;
int Audio::defaultBufferSize = 4096 * 4;

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
	assert(defaultBufferSize > 0);
	bufferData = new char[defaultBufferSize];

	alSource = 0;
	audioBuffers.ClearAndDelete();
	audioBuffers.Add(new AudioBuffer());
	audioBuffers.Add(new AudioBuffer());

	playbackEnded = false;
}

/// Generate audio source if not existing.
void Audio::CreateALObjects()
{
	/// Skip is source is valid already.
	if (alSource)
		return;
	for (int i = 0; i < audioBuffers.Size(); ++i)
	{
		AudioBuffer * audioBuffer = audioBuffers[i];
		alGenBuffers(1, &audioBuffer->alBuffer);
	}
	alGenSources(1, &alSource);
	if (alSource == 0)		// Throw an error if we can't generate a source.
		throw 5;
	std::cout<<"\nALSource generated: "<<alSource;
	assert(alSource > 0);
	/// Set some basic properties for the audio.
	alSource3f(alSource, AL_POSITION,        0.0, 0.0, 0.0);
	alSource3f(alSource, AL_VELOCITY,        0.0, 0.0, 0.0);
	alSource3f(alSource, AL_DIRECTION,       0.0, 0.0, 0.0);
	alSourcef (alSource, AL_ROLLOFF_FACTOR,  0.0          );
	alSourcei (alSource, AL_SOURCE_RELATIVE, AL_TRUE      );
}

/// Destructor that deletes AL resources as well as other thingies!
Audio::~Audio()
{
	/// Deallocate al IDs.
    if (alSource != 0){
        alSourceStop(alSource);
        alDeleteSources(1, &alSource);

		/// Delete alBuffers.
		while(audioBuffers.Size())
		{
			AudioBuffer * audioBuffer = audioBuffers[0];
			alDeleteBuffers(1, &audioBuffer->alBuffer);
			audioBuffers.Remove(audioBuffer);
			delete audioBuffer;
		}
	}
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
Audio::Audio(char i_type, String i_name, bool i_repeat, float audioVolume)
{
    Nullify();
	type = i_type;
	name = i_name;
	String newPath;
	try {
		newPath = "sound";
		switch(type){
			case AudioType::BGM: newPath += "/bgm/"; break;
			case AudioType::BGS: newPath += "/bgs/"; break;
			case AudioType::SFX: newPath += "/sfx/"; break;
			case AudioType::UIS: newPath += "/uis/"; break;
			case AudioType::SPEECH: newPath += "/spc/"; break;
		}
		newPath += name;

		// Check that the file exists maybe?
		state = AudioState::AUDIO_ERROR;

		path = newPath;

	}
	catch(String error) {
		std::cout<< error;
		state = AudioState::AUDIO_ERROR;
	}
	repeat = i_repeat;
	volume = audioVolume;
}

bool Audio::Load(){
    assert(!loaded);
    assert(audioStream == NULL);
    std::cout<<"\nAudio::Load for "<<name<<" assumed to be located at "<<path;

    std::fstream file;
    file.open(path.c_str(), std::ios_base::in);
    if (!file.is_open()){
        file.close();
        std::cout<<"\nERROR: File does not exist. Returning.";
        return false;
    }
    file.close();

    // Stream
	if (name.Contains(".ogg")){
        std::cout<<"\nFile deemed of type Ogg Vorbis, trying to load.";
		audioStream = new OggStream();
		loaded = audioStream->Open(path.c_str());
	}
	else if (name.Contains(".wav")){
	    std::cout<<"\nFile deemd of type Waveform Audio File Format, trying to load.";
        // Load wavvy
        assert(false && "Add a Wav-stream?");
      //  audioStream = new AudioStream();
      //  loaded = audioStream->Open(path.c_str());
	}
	else {
        assert(false && "Unsupported audio-format I'm afraid!");
	}
	audioStream->loop = repeat;

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
		CreateALObjects();
	}

    std::cout<<"\nPlaying audio "<<name;
//	assert(audioStream->Type() != MultimediaType::UNKNOWN);
	/// Check type
	switch(this->type){
		case AudioType::BGM:
		case AudioType::BGS: {
			// Pause all others.
			AudioMan.StopAllOfType(this->type);
			break;
		}
		default:
			break;
	}

	int channels = audioStream->AudioChannels();
    ALuint format;
	if(channels == 1)
        format = AL_FORMAT_MONO16;
    else
        format = AL_FORMAT_STEREO16;


	/// Stream source is reloaded below anyway, so skip this check.
//	assert(audioStream->source > 0);



	/// Set volume..?
	UpdateVolume();
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
	playbackEnded = false;
}

bool Audio::IsPlaying()
{
	return state == AudioState::PLAYING;
}

// Resumes ONLY if the audio was currently paused.
void Audio::Resume(){
	/*
	if (!audioEnabled)
		return;
	if (status != AUDIO_PAUSED)
		return;
	assert(false && "fix");
	// Resume playback
	/// Check for al error.
//	audioStream->Check("Error playing source");
	status = AL_PLAYING;
	if(!audioStream->Playback()){
		assert(false && "Audio stream refused to play!");
	}
	*/
}

void Audio::Pause()
{
	alSourcePause(alSource);
	state = AudioState::PAUSED;
	// TODO: save time
//	currentTime = stream.
}

void Audio::Stop()
{
	// Stahp!
	alSourceStop(alSource);
	// Seek to start.
	audioStream->Seek(0);
	// Clear current audio buffers.
	UnqueueBuffers();
	// And mark our new state.
	state = AudioState::READY;
}

// Buffers new data from underlying streams and pushes it into AL for playback.
void Audio::Update()
{
	if (!audioEnabled)
		return;

	// Check if playing?
	if (state != AudioState::PLAYING)
		return;

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
			std::cout<<"\nAudio stopped! Maybe it ran out of data to play back? Playing it again unless stream we depend on has ended!";
			alSourcePlay(alSource);
			int error = alGetError();
			switch(error){
				case AL_NO_ERROR:
					break;
				default:
					std::cout<<"\nERRRORROR";
					break;
			}
			break;
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

	/// For each processed buffer, unqueue it
	while(processed){
		/// wat.
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
		}
		--processed;
	}
	/// For each unqueued buffer, stream more data and queue it.
	List<AudioBuffer*> unqueuedBuffers = audioBuffers - queuedBuffers;
	for (int i = 0; i < unqueuedBuffers.Size(); ++i){
		AudioBuffer * audioBuffer = unqueuedBuffers[i];
		try {
			/// Replace with a call to stream audio data from the stream.
			assert(audioStream);

			#define FOURKB		4096
			#define BUFFER_SIZE (FOURKB * 64)

			char buf[BUFFER_SIZE];
			int channels = audioStream->AudioChannels();
			int format = 0;
			switch(channels){
				case 1: format = AL_FORMAT_MONO16; break;
				case 2: format = AL_FORMAT_STEREO16; break;
			}
			int frequency = audioStream->AudioFrequency();
			int bytesBuffered = audioStream->BufferAudio(buf, BUFFER_SIZE, repeat);

/*			// If unable to buffer data, assume we are at the end of the file. If so, check if we can re-start the file (assuming looping is enabled)
			if (bytesBuffered <= 0 && repeat){
				audioStream->Seek(0);
				bytesBuffered = audioStream->BufferAudio(buf, BUFFER_SIZE);
			}
*/
			if (bytesBuffered <= 0){
				playbackEnded = true;
				std::cout<<"\nUnable to buffer audio data! Assuming at end of stream.";
				state = AudioState::ENDED;
				continue;
			}
	//		std::cout<<"\nBuffer more PCM data: "<<bytesBuffered;
			/// Samples per second yo....
			alBufferData(audioBuffer->alBuffer, format, buf, bytesBuffered, frequency);
			int error = alGetError();
			switch(error){
				case AL_NO_ERROR:
					/// Mark the buffer as buffered.
					audioBuffer->buffered = true;
					break;
				default:
					std::cout<<"\nError buffering AL Data";
					break;
			}
		}
		catch (...){
			std::cout<<"\nStreaming failed, aborting.";
			return;
		}
		// Don't queue the buffer if streaming failed, since it's old data, yo.
		if (audioBuffer->buffered)
		{
			alSourceQueueBuffers(alSource, 1, &audioBuffer->alBuffer);
			ALuint error = alGetError();
	//		std::cout<<"\nBuffer queued: "<<audioBuffer->alBuffer;
			switch(error){
				case AL_NO_ERROR:
					queuedBuffers.Add(audioBuffer);
					break;
				default:
					std::cout<<"\nError queueing AL buffer.";
					break;
			}
		}
	}

    return;
}

/// Unqueues all AL buffers. This is wanted when stopping or changing locations,... probably.
void Audio::UnqueueBuffers()
{
	while(queuedBuffers.Size()){
        ALuint alBuffer;
		alSourceUnqueueBuffers(alSource, 1, &alBuffer);
		std::cout<<"\nBuffer unqueued: "<<alBuffer;
	    CheckALError("Error Unquequeing bufferrs");
		for (int i = 0; i < this->queuedBuffers.Size(); ++i){
			AudioBuffer * queuedBuffer = queuedBuffers[i];
			if (alBuffer == queuedBuffer->alBuffer){
				// Unqueue it
				queuedBuffers.Remove(queuedBuffer);
				queuedBuffer->buffered = false;
			}
		}
	}
}

/// Updates playback volume.
void Audio::UpdateVolume(float masterVolume){
	absoluteVolume = this->volume * masterVolume;
	/// Set volume in AL.
	alSourcef(alSource, AL_GAIN, absoluteVolume);
	int error = alGetError();
	switch(error){
		case AL_NO_ERROR:
			break;
		default:
			//std::cout<<"\nError o-o";
			break;
	}
}



#endif // USE_AUDIO
