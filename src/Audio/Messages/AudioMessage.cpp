/// Emil Hedemalm
/// 2014-07-24
/// Similar to graphics and physics, a dedicated messaging system for queueing playback and changes with audio, 
/// As it may be handled in a different thread, which otherwise would cause race conditions.

#include "AudioMessage.h"
#include "Audio/AudioManager.h"
#include "Audio/Audio.h"

AudioMessage::AudioMessage(int type)
	: type(type)
{
}

AudioMessage::~AudioMessage()
{
}

void AudioMessage::Process()
{
	switch(type)
	{
		case AM_SHUTDOWN:
			AudioMan.Shutdown();
			break;
		case AM_DISABLE_AUDIO:
			AudioMan.DisableAudio();
			break;
		case AM_STOP_ALL:
			AudioMan.StopAndRemoveAll();
			break;
		case AM_TOGGLE_MUTE:
			AudioMan.ToggleMute();
			break;
		default:
			assert(false);
	}
}

/// E.g. MASTER_VOLUME, and the volume.
AMSet::AMSet(int target, float fValue)
	: AudioMessage(AM_SET), target(target), fValue(fValue)
{
	switch(target)
	{
		case AT_MASTER_VOLUME:
			break;
		default:
			assert(false);
	}
}

AMSet::AMSet(int target, Camera * listener)
: AudioMessage(AM_SET), target(target), listener(listener)
{
	switch(target)
	{
	case AT_LISTENER:
		break;
	default:
		assert(false);
	}
}

/// E.g. setting categpry volumes, specify CATEGORY, the id of the category (e.g. AudioType::BGM), and then the volume.
AMSet::AMSet(int target, int id, float fValue)
	: AudioMessage(AM_SET), target(target), id(id), fValue(fValue)
{
	switch(target)
	{
		default:
			assert(false);
	}
}
void AMSet::Process()
{
	switch(target)
	{
		case AT_MASTER_VOLUME:
			AudioMan.SetMasterVolume(fValue);
			break;
		case AT_LISTENER:
			AudioMan.listener = listener;
			break;
		default:
			assert(false);
	}
}

AMSetAudio::AMSetAudio(Audio * audio, int target, float fValue)
: AudioMessage(AM_SET_AUDIO), audio(audio), target(target), fValue(fValue)
{
	switch(target)
	{
		case AT_VOLUME:
			break;
		default:
			assert(false);
	}
}
void AMSetAudio::Process()
{
	if (!audio)
		return;
	switch(target)
	{
		case AT_VOLUME:
			audio->volume = fValue;
			break;
		default:
			assert(false);
	}
}

AMFade::AMFade(Audio * audio, float targetVolume, float fadeTime)
: AudioMessage(AM_FADE), audio(audio), volume(targetVolume), fadeTime(fadeTime)
{
}
void AMFade::Process()
{
	audio->FadeTo(volume, fadeTime);
}


AMRegister::AMRegister(Audio * audio)
: AudioMessage(AM_REGISTER), audio(audio)
{
	assert(audio);
}
void AMRegister::Process()
{
	AudioMan.RegisterAudio(audio);
}

AMPlay::AMPlay(Audio * existingAudio)
: AudioMessage(AM_PLAY), audio(existingAudio)
{
}

AMPlay::AMPlay(int audioType, String nameOrSource, float volume)
	: AudioMessage(AM_PLAY), audioType(audioType), nameOrSource(nameOrSource), volume(volume)
{
	audio = NULL;
	positional = false;
}

void AMPlay::Process()
{
	if (audio)
	{
		if (!audio->registeredForRendering)
			AudioMan.RegisterAudio(audio);
		audio->Play();
		return;
	}
//	std::cout<<"\nAMPlay called with source: "<<nameOrSource;
	switch(audioType)
	{
		case AudioType::SFX:
		{
			AudioMan.PlaySFX(nameOrSource, volume);
			break;
		}
		case AudioType::BGM:
			AudioMan.PlayBGM(nameOrSource, volume);
			break;
		default:
			assert(false);
	}
}


AMPlaySFX::AMPlaySFX(String nameOrSource, float volume)
	: AMPlay(AudioType::SFX, nameOrSource, volume)
{
}
// Positional
AMPlaySFX::AMPlaySFX(String nameOrSource, float volume, ConstVec3fr atPosition)
: AMPlay(AudioType::SFX, nameOrSource, volume)
{
	positional = true;
	position = atPosition;
}

AMPlayBGM::AMPlayBGM(String nameOrSource, float volume)
	: AMPlay(AudioType::BGM, nameOrSource, volume)
{
}

AMPause::AMPause(Audio * audio)
: AudioMessage(AM_PAUSE), audio(audio)
{
}
void AMPause::Process()
{
	audio->Pause();
}


AMStop::AMStop(int audioType)
	: AudioMessage(AM_STOP), audioType(audioType)
{
}

void AMStop::Process()
{
	AudioMan.StopAllOfType(audioType);
}

AMStopBGM::AMStopBGM()
	: AMStop(AudioType::BGM)
{
}



