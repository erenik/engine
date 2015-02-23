/// Emil Hedemalm
/// 2014-07-24
/// Similar to graphics and physics, a dedicated messaging system for queueing playback and changes with audio, 
/// As it may be handled in a different thread, which otherwise would cause race conditions.

#include "AudioMessage.h"
#include "Audio/AudioManager.h"

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
		case MASTER_VOLUME:
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
		case MASTER_VOLUME:
			AudioMan.SetMasterVolume(fValue);
			break;
		default:
			assert(false);
	}
}


AMPlay::AMPlay(int audioType, String nameOrSource, float volume)
	: AudioMessage(AM_PLAY), audioType(audioType), nameOrSource(nameOrSource), volume(volume)
{
}

void AMPlay::Process()
{
//	std::cout<<"\nAMPlay called with source: "<<nameOrSource;
	switch(audioType)
	{
		case AudioType::SFX:
			AudioMan.PlaySFX(nameOrSource, volume);
			break;
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

AMPlayBGM::AMPlayBGM(String nameOrSource, float volume)
	: AMPlay(AudioType::BGM, nameOrSource, volume)
{
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



