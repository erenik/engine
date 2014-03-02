// Emil Hedemalm
// 2013-03-22

#include "AudioSettings.h"

#ifndef USE_AUDIO
	#define AudioMan		NULL //Network disabled.
#else

#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#define AUDIO	1

#include <stdexcept>
#include <Util.h>
#include "AudioTypes.h"

/// Macro for accessing the audio manager singleton!
#define AudioMan (*AudioManager::Instance())

// Forward declarations to avoid includes until neededs
typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;

class Audio;
class MultimediaStream;

/** A general utility manager for handling sounds and music pieces with varying degrees of complexity.
	It is designed to be able to work with OpenAL mainly.
*/
class AudioManager {
private:
	AudioManager();
	static AudioManager * audioManager;
public:
	~AudioManager();
	static AudioManager * Instance() { return audioManager; };
	/// Called once in the initializer thread after allocation but before the engine gets started.
	void Initialize();
	/// Called once in the deallocator thread when stop procedures have begun but before deallocation occurs.
	void Shutdown();
	static void Allocate();
	static void Deallocate();

	/// Loads preferences from the PreferencesManager if possible. Should be called from the application's global state on enter.
	void LoadPreferences();
	/// Saves preferences to the PreferencesManager. Should be called from the application's global state on exit.
	void SavePreferences();


	/** Attempts to play audio from given source. Optional arguments control loop-mode and relative volume.
		Returns the relevant Audio object upon success.
	*/
	Audio * PlayFromSource(char type, String fromSource, bool repeat = false, float volume = 1.0f);
	void Play(char type, String name, bool repeat = false, float volume = 1.0);
	void Pause(String name);
	void Stop(String name);
	void Stop(int index);

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

	/// Updates the streams and volumes ^^
	void Update();
	float MasterVolume(){ return masterVolume; };
	/// Sets master volume, from 0.0 to 1.0
	void SetMasterVolume(float level);
	/// Calls update volume for all audio
	void UpdateVolume();	

	/// Flag if we should pause updates
	bool pauseUpdates;	
	/// If we're currently udpating
	bool updating;		

private:
	/// Value applied to all audio.
	float masterVolume;
	/// To toggle it in run-time.
	bool audioEnabled;
	/// Pauses target audio.
	void Pause(Audio * audio);
	/// List of active audio
	List<Audio*> audioList;

#ifdef USE_OPEN_AL
	// Open AL device and context, similar to OpenGL device and context!
	ALCdevice * alcDevice;		// Device
	ALCcontext * alcContext;	// Rendering audio context
#endif

	bool initialized;	// Consider using other relevant variables to test if initialization succeeded.
};

#endif

#endif // USE_AUDIO