/// Emil Hedemalm
/// 2014-01-07
/// Overlay class for the handling audio, abstracted to the user's level.

#include "Track.h"
#include "Audio/Audio.h"
#include "Audio/AudioManager.h"

Track::Track()
{
	audio = NULL;
	loop = false;
}

void Track::Play(){
#ifdef AUDIO
	if (audio)
	{
		/// Pause all other BGMs?
		AudioMan.PauseAllOfType(AudioType::BGM);
		audio->Play();
	}
	else {
		audio = AudioMan.PlayFromSource(AudioType::BGM, source, loop);
//	AudioMan.PlayFromSource(BGM, source, true);
	}
#endif
}

bool Track::IsPlaying()
{
	if (audio)
		return audio->IsPlaying();
	return false;
}

void Track::Pause()
{
	if (!audio)
		return;
	audio->Pause();
}
void Track::Resume()
{
	if (audio){
		audio->Resume();
	}
	else 
		this->Play();
}

void Track::Stop(){
	if (audio){
		audio->Stop();
	}
}

/// Sets the track to start/stop looping.
void Track::Loop(bool value)
{
	loop = value;
	if (audio)
		audio->repeat = true;
}