// Emil Hedemalm
// 2013-03-22

#include "MultimediaStream.h"
#include "Audio/Audio.h"
#include "Audio/AudioManager.h"

// Default constructor, See types in MultimediaTypes.h
MultimediaStream::MultimediaStream(int type)
: type(type)
{
	startTime = 0;
	volume = 1.0f;
	hasAudio = hasVideo = false;
	lastUpdate = 0;
	streamState = StreamState::NOT_INITIALIZED;
	currentFrame = 0;
	mediaTime = 0;
	audio = NULL;
}

/// Attempts to open target file. Returns false upon failure.
bool MultimediaStream::Open(String path)
{
	assert(false && "Subclass");
	return false;
} 
/// Closes target file and stream.
void MultimediaStream::Close()
{
	assert(false && "Subclass");
}

/// Current frame time of the media in milliseconds.
int MultimediaStream::CurrentFrameTime()
{
	int denom = fpsDenom;
	int numbersInDenom = 0;
	int divisor = 1;
	while(denom > 0){
		denom /= 10;
		numbersInDenom++;
		divisor *= 10;
	}
	return currentFrame * fpsNom + (currentFrame * fpsDenom) / divisor;
}

/// Starts buffering of the stream. Nothing is done until this command has been executed successfully, following a call to Open.
bool MultimediaStream::Play()
{

	if (streamState == StreamState::PAUSED ||
		streamState == StreamState::READY)
	{
		streamState = StreamState::PLAYING;
		/// Start audio also if not already schtarted?
		if (!audio)
			audio = AudioMan.CreateAudioStream(this);
		audio->Play();
		return true;
	}
	return false;
}
/// Pauses the stream.
bool MultimediaStream::Pause()
{
	if (streamState == StreamState::PLAYING){
		streamState = StreamState::PAUSED;
		if (audio)
			audio->Pause();
		return true;
	}
	return false;
}

/// Seeks to target time in milliseconds.
bool MultimediaStream::Seek(int toTime)
{
	assert(false && "Subclass");
	return false;
}

/// How many milliseconds each frame should take on average.
float MultimediaStream::TimePerFrame(){
	return 1.0f / fps;
}

/// Streams the next frame. Does not perform any type of check that the frame is beyond current time, meaning the MultimediaManager has this responsibility!
bool MultimediaStream::StreamNextFrame(int framesToPass)
{
	assert(false && "Subclass");
	return false;
}

/** Buffers audio data into buf, up to maximum of maxBytes. Returns amount of bytes buffered. 
	Amount of bytes may reach 0 even if the media has not ended. Returns -1 on error.
	If loop is set to true it will try to automatically seek to the beginning when it reaches the end of the file.
*/
int MultimediaStream::BufferAudio(char * buf, int maxBytes, bool loop)
{
	assert(false && "Subclass");
	return -1;
}

/** Returns a texture which will be updated as the stream progresses. 
	If the stream lacks graphics NULL will be returned.
	If the texture has the rebufferize flag set it has to be re-buffered to gain new frame data.
*/
Texture * MultimediaStream::GetFrameTexture(){
	return NULL;
}



/// Returns the type of the stream. See MultimediaTypes.h. 
int MultimediaStream::Type()
{
	return type;
}

/// Sets relative level.
void MultimediaStream::SetVolume(float level)
{
	volume = level;

}

/// Returns amount of channels present in the audio stream.
int MultimediaStream::AudioChannels()
{
	assert(false && "Subclass");
	return 0;
}

/// Gets frequency of the audio. This is typically 48000 or similar?
int MultimediaStream::AudioFrequency()
{
	assert(false && "Subclass");
	return 0;
}
