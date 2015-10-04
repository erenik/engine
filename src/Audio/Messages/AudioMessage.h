/// Emil Hedemalm
/// 2014-07-24
/// Similar to graphics and physics, a dedicated messaging system for queueing playback and changes with audio, 
/// As it may be handled in a different thread, which otherwise would cause race conditions.

#ifndef AUDIO_MESSAGE_H
#define AUDIO_MESSAGE_H

#include "String/AEString.h"
#include "MathLib/Vector3f.h"

enum audioMessageTypes
{
	AM_SHUTDOWN,
	
	AM_PAUSE_PLAYBACK,
	AM_RESUME_PLAYBACK,
	// 
	AM_REGISTER,
	AM_DISABLE_AUDIO,
	AM_MUTE_SFX,
	AM_SET, // general setter.
	AM_SET_BOOLEAN,
	AM_SET_AUDIO, // audio setter
	AM_PLAY,
	AM_FADE,
	AM_QUEUE,
	AM_PAUSE,
	AM_STOP,
	AM_STOP_ALL,
	AM_TOGGLE_MUTE,
};

// All prefixed AT_
enum audioTargets 
{
	AT_MASTER_VOLUME,
	AT_BGM_VOLUME,
	AT_SFX_VOLUME,
	AT_LISTENER, // For setting the camera or entity which si the listener for calculating positional audio volumes.
	AT_CATEGORY_VOLUME,
	AT_VOLUME,
	// Overlaying settings.
	AT_BGM_ENABLED, 
};

class Audio;
class Camera;

class AudioMessage 
{
public:
	AudioMessage(int type);
	virtual ~AudioMessage();
	virtual void Process();
private:
	int type;
	float fValue;
};

class AMSet : public AudioMessage 
{
public:
	/// E.g. MASTER_VOLUME, and the volume.
	AMSet(int target, float fValue);
	AMSet(int target, Camera * listener);
	/// E.g. setting categpry volumes, specify CATEGORY, the id of the category (e.g. AudioType::BGM), and then the volume.
	AMSet(int target, int id, float fValue);
	virtual void Process();
private:
	int target;
	Camera * listener;
	int id;
	float fValue;
};

class AMSetb : public AudioMessage 
{
public:
	AMSetb(int target, bool bValue);
	virtual void Process();
private:
	int target;
	bool value;
};

class AMSetAudio : public AudioMessage 
{
public:
	AMSetAudio(Audio * audio, int target, float fValue);
	virtual void Process();
private:
	Audio * audio;
	int target;
	float fValue;
};

class AMFade : public AudioMessage 
{
public:
	AMFade(Audio * audio, float targetVolume, float fadeTime);
	virtual void Process();
private:
	Audio * audio;
	float volume;
	float fadeTime;
};

/// Registers and audio to be rendered, but does not actually play it yet.
class AMRegister : public AudioMessage 
{
public:
	AMRegister(Audio * audio);
	virtual void Process();
protected:
	Audio * audio;
};

class AMPlay : public AudioMessage 
{
public: 
	AMPlay(Audio * existingAudio);
	AMPlay(int audioType, String nameOrSource, float volume);
	virtual void Process();
protected:
	bool positional;
	Vector3f position;
	Audio * audio;
	int audioType;
	String nameOrSource;
	float volume;
};

class AMPlaySFX : public AMPlay
{
public:
	/// Non positional
	AMPlaySFX(String nameOrSource, float volume = 1.f);
	// Positional
	AMPlaySFX(String nameOrSource, float volume, ConstVec3fr position);
private:
};

/// By default tries to play a BGM from start. Flag boolean 'resumePaused' if you want them to resume a paused BGM (default pauses before switching).
class AMPlayBGM : public AMPlay 
{
public:
	AMPlayBGM(String nameOrSource, float volume = 1.f);
	virtual void Process();
	bool resumePaused;
private:
};

class AMPause : public AudioMessage 
{
public:
	AMPause(Audio * audio, bool pauseLocally = true);
	virtual void Process();
private:
	Audio * audio;
	bool pauseLocally;
};

class AMStop : public AudioMessage 
{
public: 
	AMStop(int audioType);
	virtual void Process();
private:
	int audioType;
};

class AMStopBGM : public AMStop
{
public:
	AMStopBGM();
private:
};

#endif
