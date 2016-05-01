/// Emil Hedemalm
/// 2014-01-07
/// Overlay class for the handling audio, abstracted to the user's level.

#include "Track.h"
#include "Audio/Audio.h"
#include "Audio/AudioManager.h"

class SRTrack * track = 0;

Track::Track()
{
	audio = NULL;
	loop = false;
}

void Track::Play()
{
#ifdef AUDIO
	if (!audio)
	{
	
	}
	if (audio)
	{
		assert(false);
		/// Pause all other BGMs?
	//	AudioMan.PauseAllOfType(AudioType::BGM);
		audio->Play();
	}
#endif
}


bool Track::IsPlaying()
{
	if (audio)
		return audio->IsPlaying();
	return false;
}

void Track::Pause(bool pauseLocally)
{
	if (!audio)
		return;
	audio->Pause(true);
}
void Track::Resume()
{
	if (audio){
		audio->Resume();
	}
	else 
		this->Play();
}

void Track::Stop()
{
	if (audio)
	{
		audio->Stop(true);
	}
}

/// Sets the track to start/stop looping.
void Track::Loop(bool value)
{
	loop = value;
	if (audio)
		audio->repeat = true;
}

void Track::SetVolume(float relative)
{
	if (relative < 0)
		relative = 0;
	if (audio)
		audio->volume = relative;
}
