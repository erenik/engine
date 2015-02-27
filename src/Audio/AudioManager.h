// Emil Hedemalm
// 2013-03-22

#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#define AUDIO	1

#include "OS/Thread.h"
#include <stdexcept>
#include <Util.h>
#include "AudioTypes.h"
#include "Messages/AudioMessage.h"
#include "MathLib/Vector3f.h"

/// Macro for accessing the audio manager singleton!
#define AudioMan (*AudioManager::Instance())

// Forward declarations to avoid includes until neededs
typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;

extern String lastAudioInfo;

class Camera;
class Message;
class Audio;
class MultimediaStream;

namespace AudioDriver {
	enum 
	{
		BAD_DRIVER = -1,
		OpenAL,
		WindowsCoreAudio,
		DRIVERS,
	};
};

/// See enum above.
extern int audioDriver;
/// Updated each audio-loop.
extern int64 audioNowMs;

/** A general utility manager for handling sounds and music pieces with varying degrees of complexity.
	It is designed to be able to work with OpenAL mainly.
*/
class AudioManager 
{
	friend class AudioMessage;
	friend class MultimediaStream;
	friend class AMPlay;
	friend class AMSet;
	friend class AMStop;
	friend class Audio;
private:
	AudioManager();
	static AudioManager * audioManager;
public:
	~AudioManager();
	static AudioManager * Instance() { return audioManager; };
	
	/// Controls the main audio processing thread.
	bool shouldLive;
	/// Main audio processing thread
	PROCESSOR_THREAD_DEC;

	/// If true, you may queue messages.
	static bool AudioProcessingActive();
	static void SetDefaultAudioDriver(String fromString);

	/** Called once in the initializer thread after allocation but before the engine gets started. 
		If false, audio was failed to set up, and program should be quit.
	*/
	bool Initialize();
	bool InitializeDriver(int driverID);
	/// Called once in the deallocator thread when stop procedures have begun but before deallocation occurs.
	void Shutdown();
	static void Allocate();
	static void Deallocate();

	/// Loads preferences from the PreferencesManager if possible. Should be called from the application's global state on enter.
	void LoadPreferences();
	/// Saves preferences to the PreferencesManager. Should be called from the application's global state on exit.
	void SavePreferences();

	/// Yer.
	void QueueMessage(AudioMessage* message);
	void QueueMessages(const List<AudioMessage*> & messageList);

	/// o.o
	void RegisterAudio(Audio * audio);

	/// Updates the streams and volumes, also processes any queued messages ^^
	void Update();
	Vector3f ListenerPosition();
	
	/// Getters
	float MasterVolume(){ return masterVolume; };
	/// Based on master and mute.
	float ActiveMasterVolume(){ if(mute) return 0; return masterVolume;};
	/// Returns the current volume for target category (0.0 to 1.0)
	float CategoryVolume(int category);
	

	// Yup.
	void StopAndRemoveAll();

private:

	Audio * GetAudioByName(String name);
	void ToggleMute();
	/** Attempts to play audio from given source. Optional arguments control loop-mode and relative volume.
		Returns the relevant Audio object upon success.
	*/
	Audio * PlayFromSource(char type, String fromSource, bool repeat = false, float volume = 1.0f);
	/// Type? Defined where..?
	Audio * Play(char type, String name, bool repeat = false, float volume = 1.0);
	/// Name is should correspond to filename or path, expected locatoin in ./sound/sfx/
	void PlaySFX(String name, float volume = 1.f);
	/// Plays target BGM, pausing all others. Default sets to repeat.
	Audio * PlayBGM(String name, float volume = 1.f);

	// Halting playback
	void Pause(String name);
	void Stop(String name);

	/// Creates an audio stream which will continually try and update by grabbing PCM data from the stream.
	Audio * CreateAudioStream(MultimediaStream * stream);

	/// Getter/checker
	bool AudioEnabled() { return audioEnabled; };
	/// Enables audio and unpauses all paused media.
	void EnableAudio();
	/// Disables audio and pauses all current playback indefinitely.
	void DisableAudio();

	/// Pause o.O
	void PauseAllOfType(char type);
	void StopAllOfType(char type);
	void FadeOutAllOfType(char type, float seconds);

	/// Sets master volume, from 0.0 to 1.0
	void SetMasterVolume(float level);
	/// Calls update volume for all audio
	void UpdateVolume();	

	/// Flag if we should pause updates
	bool pauseUpdates;	
	/// If we're currently udpating
	bool updating;		
	

	/// For processing general messages.
	void ProcessMessage(Message * mes);
	
private:

	bool ClaimAudioMutex();
	bool ReleaseAudioMutex();

	// yer.
	void ProcessAudioMessages();
	/// o=o
	List<AudioMessage*> messageQueue;

	/// Volumes for the various audio categories (BGM, SFX, etc.)
	List<float> categoryVolumes;

	/// listener o.o
	Camera * listener;
	Vector3f listenerPosition;
	/// Value applied to all audio.
	float masterVolume;
	// Default false.
	bool mute;
	/// To toggle it in run-time.
	bool audioEnabled;
	/// Pauses target audio.
	void Pause(Audio * audio);
	/// List of active audio
	List<Audio*> audioList;

	bool initialized;	// Consider using other relevant variables to test if initialization succeeded.
};

#endif
