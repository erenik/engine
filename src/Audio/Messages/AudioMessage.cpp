/// Emil Hedemalm
/// 2014-07-24
/// Similar to graphics and physics, a dedicated messaging system for queueing playback and changes with audio, 
/// As it may be handled in a different thread, which otherwise would cause race conditions.

#include "AudioMessage.h"
#include "Audio/AudioManager.h"
#include "Audio/Audio.h"
#include "File/LogFile.h"

AudioMessage::AudioMessage(int type)
	: type(type)
{
}

AudioMessage::~AudioMessage()
{
}

AMGlobal::AMGlobal(int type)
	: AudioMessage(type)
{
	switch (type)
	{
		case AM_SHUTDOWN:
		case AM_PAUSE_PLAYBACK:
		case AM_RESUME_PLAYBACK:
		case AM_DISABLE_AUDIO:
		case AM_STOP_ALL:
		case AM_TOGGLE_MUTE:
		case AM_MUTE_SFX:
			break;
		default:
			LogMain("Unknown audio message: "+String(type), DEBUG);
			assert(false && "See audio log file");
	}
}
AMGlobal::~AMGlobal()
{
}

void AMGlobal::Process()
{
	switch(type)
	{
		case AM_SHUTDOWN:
			AudioMan.Shutdown();
			break;
		case AM_PAUSE_PLAYBACK:
			AudioMan.GlobalPause(true);
			break;
		case AM_RESUME_PLAYBACK:
			AudioMan.GlobalPause(false);
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
		case AM_MUTE_SFX:
			AudioMan.muteType[AudioType::SFX] = true;
			AudioMan.UpdateVolume();
		//	AudioMan.ToggleMute(AudioType::SFX);
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
		case AT_BGM_VOLUME:
		case AT_SFX_VOLUME:
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
		case AT_BGM_VOLUME:
			AudioMan.bgmVolume = fValue;
			AudioMan.UpdateVolume();
			break;
		case AT_SFX_VOLUME:
			AudioMan.sfxVolume = fValue;
			AudioMan.UpdateVolume();
			break;
		case AT_LISTENER:
			AudioMan.listener = listener;
			break;
		default:
			assert(false);
	}
}

AMSetb::AMSetb(int target, bool bValue)
: AudioMessage(AM_SET_BOOLEAN), target(target), value(bValue)
{
	switch(target)
	{
		case AT_BGM_ENABLED:
			break;
		default:
			assert(false && "Bad target");
	}
}
void AMSetb::Process()
{
	switch(target)
	{
		case AT_BGM_ENABLED:
		{
			// Resume any paused ones?
			if (value)
			{
				AudioMan.ResumeAllOfType(AudioType::BGM);
			}
			// Pause all BGMs and prevent new ones from playing?
			else {
				AudioMan.PauseAllOfType(AudioType::BGM);
			}
			AudioMan.bgmEnabled = value;
		}
		
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
	resumePaused = false;
}

void AMPlayBGM::Process()
{
	AudioMan.PlayBGM(nameOrSource, volume);	
}


AMPause::AMPause(Audio * audio, bool pauseLocally)
: AudioMessage(AM_PAUSE), audio(audio), pauseLocally(pauseLocally)
{
}
void AMPause::Process()
{
	audio->Pause(pauseLocally);
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



