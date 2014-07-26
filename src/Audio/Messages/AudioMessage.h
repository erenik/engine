/// Emil Hedemalm
/// 2014-07-24
/// Similar to graphics and physics, a dedicated messaging system for queueing playback and changes with audio, 
/// As it may be handled in a different thread, which otherwise would cause race conditions.

#ifndef AUDIO_MESSAGE_H
#define AUDIO_MESSAGE_H

#include "String/AEString.h"

enum audioMessageTypes
{
	AM_DISABLE_AUDIO,
	AM_SET, // general setter.
	AM_PLAY,
	AM_QUEUE,
	AM_PAUSE,
	AM_STOP,
};

enum audioTargets 
{
	MASTER_VOLUME,
	CATEGORY_VOLUME,
	VOLUME,
};

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
	/// E.g. setting categpry volumes, specify CATEGORY, the id of the category (e.g. AudioType::BGM), and then the volume.
	AMSet(int target, int id, float fValue);
	virtual void Process();
private:
	int target;
	int id;
	float fValue;
};

class AMPlay : public AudioMessage 
{
public: 
	AMPlay(int audioType, String nameOrSource, float volume);
	virtual void Process();
protected:
	int audioType;
	String nameOrSource;
	float volume;
};

class AMPlaySFX : public AMPlay
{
public:
	AMPlaySFX(String nameOrSource, float volume = 1.f);
private:
};

class AMPlayBGM : public AMPlay 
{
public:
	AMPlayBGM(String nameOrSource, float volume = 1.f);
private:
};

#endif
