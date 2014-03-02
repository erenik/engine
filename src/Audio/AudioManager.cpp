// Emil Hedemalm
// 2013-03-22

#include "AudioSettings.h"

#ifdef USE_AUDIO

#include "AudioManager.h"
#include "TrackManager.h"
#include "System/PreferencesManager.h"
// #include "../globals.h"

#ifdef USE_OPEN_AL
#include <AL/al.h>
#include <AL/alc.h>
// #include <alut.h>
#endif

#include "Audio.h"

#include <iostream>


AudioManager * AudioManager::audioManager = NULL;

// Short stringstream for debugging using dlog();
// extern stringstream debugs;

// Default constructor
AudioManager::AudioManager(){

	// Set the global audio to this upon finishing construction.
	initialized = false;
	pauseUpdates = false;
	audioEnabled = true;

	masterVolume = 0.1f;
}

AudioManager::~AudioManager(){
	audioList.ClearAndDelete();
#ifdef USE_OPEN_AL
	alcMakeContextCurrent( NULL );
	alcDestroyContext( alcContext );
	alcCloseDevice( alcDevice );
//	alutExit();
#endif
}

void AudioManager::Initialize(){
	std::cout<<"\nStarting AudioManager...";

#ifdef USE_OPEN_AL

	std::cout<<"\nInitializing OpenAL Utilities...";
	std::cout<<"\nChecking for available devices...";
	if (alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") == AL_TRUE){
		std::cout<<"\nEnumeration extension found!";
		const char * deviceSpecifier = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		const char * defaultDevice = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	}
	else
		std::cout<<" Enumeration extension not available!";

	// TODO: Proper initialization
	alcDevice = alcOpenDevice(NULL); // Open default alcDevice
	if (alcDevice == 0){
		std::cout<<"\nERROR: Unable to open AL device.";
		//	assert(alcDevice && "Unable to open AL device in AudioManager::Initialize");
		//audio = this;
		return;
	}
	alcContext = alcCreateContext(alcDevice, NULL);
	int result = alcMakeContextCurrent(alcContext);
	if (result == ALC_FALSE){
		std::cout<<"\nERROR: Unable to make AL context active.";
		assert(alcDevice && "Unable to make AL context active in AudioManager::Initialize");
		return;
	}

	/// Set initial values, like the listener position?

//	std::cout<<"Queueing initial playback";

	// If something went wrong, throw an error.
	if (false){
		throw std::runtime_error("AudioManager failed to initialize!");
	}
	// Set initialized to true after all initialization has been completed correctly.
	initialized = true;
#else
	std::cout<<"\nINTO: Open AL is currently disabled. Enable it in AudioSettings.h and recompile!";
#endif // USE_OPEN_AL
}

/// Called once in the deallocator thread when stop procedures have begun but before deallocation occurs.
void AudioManager::Shutdown()
{
	/// Unbind audio from their streams. Streams should be deleted by the multimedia-manager.
	for (int i = 0; i < audioList.Size(); ++i){
		Audio * audio = audioList[i];
		audio->audioStream = NULL;
	}
}

void AudioManager::Allocate(){
	assert(audioManager == NULL);
	audioManager = new AudioManager();
}
void AudioManager::Deallocate(){
	assert(audioManager);
	delete audioManager;
}

/// Loads preferences from the PreferencesManager if possible. Should be called from the application's global state on enter.
void AudioManager::LoadPreferences()
{
	float masterVol;
	bool boolVal;
	bool success = Preferences.GetFloat("MasterVolume", &masterVol);
	if (success)
		this->SetMasterVolume(masterVol);
	success = Preferences.GetBool("AudioEnabled", &boolVal);
	if (success)
	{
		if (boolVal)
			EnableAudio();
		else
			DisableAudio();
	}


}
/// Saves preferences to the PreferencesManager. Should be called from the application's global state on exit.
void AudioManager::SavePreferences()
{
	Preferences.SetFloat("MasterVolume", masterVolume);
	Preferences.SetBool("AudioEnabled", audioEnabled);
}

#ifdef USE_AUDIO


void testPlaybackLoop(){

#ifdef USE_OGG_VORBIS
	OggStream ogg;

//	alutInit(NULL, NULL);

	try
    {
     //   if(argc < 2)
      //      throw string("oggplayer *.ogg");

		String path = "audio\\bgm\\tileatron.ogg";
        ogg.Open(path);

        ogg.Display();

		if(!ogg.Playback())
            throw string("Ogg refused to Play.");

        while(ogg.Update())
        {
	        if(!ogg.IsPlaying())
            {
                if(!ogg.Playback())
                    throw String("Ogg abruptly stopped.");
                else
					std::cout<< "Ogg stream was interrupted.\n";
            }
        }

        std::cout<<"Program normal termination.";

    }
	catch(String error)
    {
		std::cout << "Error: "<<error;
    }
    #endif // USE_OGG_VORBIS
}
#endif // USE_AUDIO

void AudioManager::PauseAllOfType(char type){
	if (!initialized)
		return;
	ALCboolean result = alcMakeContextCurrent(alcContext);
	Audio * audio;
	for (int i = 0; i < audioList.Size(); ++i){
		audio = audioList[i];
		if (audio->type == type){
			Pause(audio);
		}
	}
}

void AudioManager::StopAllOfType(char type){
	Audio * audio;
	for (int i = 0; i < audioList.Size(); ++i){
		audio = audioList[i];
		if (audio->type == type && audio->type){
			audio->Stop();
		}
	}
}

/** Attempts to play audio from given source. Optional arguments control loop-mode and relative volume.
	Returns the relevant Audio object upon success.
*/
Audio * AudioManager::PlayFromSource(char type, String fromSource, bool repeat /*= false*/, float volume /*= 1.0f*/)
{
#ifndef USE_AUDIO
    std::cout<<"\nAudio disabled. Returning.";
	return;
#endif
	switch(type){
		case AudioType::BGM:
		{
			PauseAllOfType(type);
			break;
		}
	}

    std::cout<<"Not initialized! o.o";

	if (!audioEnabled)
		return NULL;
    if (!initialized){
        std::cout<<"\nAudioManager not initialized correctly. Unable to continue.";
        return NULL;
    }
	// Does what?
	pauseUpdates = true;
	ALCboolean result = alcMakeContextCurrent(alcContext);
	assert(result && "Unable to set alcContext in AudioManager::Play");

	// Create audio
	std::cout<<"\nAudio not found, tryng to create and load it.";
	Audio * audio = new Audio(fromSource);
	audio->type = type;
	audio->repeat = repeat;
	audio->volume = volume;

	bool loadResult = audio->Load();
	if (!loadResult && !fromSource.Contains("bgm/")){
		audio->path = "bgm/" + audio->path;
		loadResult = audio->Load();
	}
	if (!loadResult && !fromSource.Contains("sound/")){
		audio->path = "sound/" + audio->path;
		loadResult = audio->Load();
	}
	std::cout<<"\nLoad result: "<<loadResult;
	if (loadResult == false){
        delete audio;
        std::cout<<"\nERROR: Unable to load audio \""<<audio->name<<"\". Aborting playback.";
        return NULL;
	}
//	assert(audio->audioStream->source > 0);
	audioList.Add(audio);
	/// Generate audio source if not existing.
	audio->CreateALObjects();
	audio->UpdateVolume(masterVolume);
	audio->Play();
	pauseUpdates = false;
	return audio;
}



void AudioManager::Play(char type, String name, bool repeat, float volume){
#ifndef USE_AUDIO
    std::cout<<"\nAudio disabled. Returning.";
	return;
#endif
    if (!initialized)
        return;
	if (!audioEnabled)
		return;
   // assert(initialized);

	pauseUpdates = true;
	ALCboolean result = alcMakeContextCurrent(alcContext);
	assert(result && "Unable to set alcContext in AudioManager::Play");

	// Look if we already have an element with same name
	Audio * audio;
	for (int i = 0; i < audioList.Size(); ++i){
		audio = audioList[i];
		// If we find a similar audio already in the array: Play it
		if (name == audio->name){
			/// Update the audio o-o
			audio->Play();
			// Update volume
			audio->UpdateVolume(masterVolume);
			return;
		}
	}

	// Create audio
	std::cout<<"\nAudio not found, tryng to create and load it.";
	audio = new Audio(type, name, repeat, volume);
	bool loadResult = audio->Load();
	std::cout<<"\nLoad result: "<<loadResult;
//	assert(loadResult && "Could not load audio data?");
	if (loadResult == false){
        delete audio;
        std::cout<<"\nERROR: Unable to load audio \""<<name<<"\". Aborting playback.";
        return;
	}
	/// Generate audio source if not existing.
	audio->CreateALObjects();
	audio->UpdateVolume(masterVolume);
//	assert(audio->audioStream->source > 0);
	audioList.Add(audio);
	audio->Play();
	pauseUpdates = false;
}

// Pause
void AudioManager::Pause(String name){
	// Look if we already have an element with same name
	Audio * audio;
	for (int i = 0; i < audioList.Size(); ++i){
		audio = audioList[i];
		// If we find a similar audio already in the array: Pause it
		if (audio->name == name){
			Pause(audio);
			return;
		}
	}
}

void AudioManager::Pause(Audio * audio){
	if (!initialized)
		return;
	ALCboolean result = alcMakeContextCurrent(alcContext);
	audio->Pause();
}

void AudioManager::Stop(String name){
	ALCboolean result = alcMakeContextCurrent(alcContext);
	assert(result && "Unable to make alc context current");
	// Look if we already have an element with same name
	Audio * audio;
	for (int i = 0; i < audioList.Size(); ++i){
		audio = audioList[i];
		// If we find a similar audio already in the array: Pause it
		if (name == audio->name){
			Stop(i);
			return;
		}
	}
}

void AudioManager::Stop(int index){
	ALCboolean result = alcMakeContextCurrent(alcContext);
	audioList[index]->Stop();
	delete audioList[index];
	audioList[index] = NULL;
}

/// Creates an audio stream which will continually try and update by grabbing PCM data from the stream.
Audio * AudioManager::CreateAudioStream(MultimediaStream * stream)
{
	Audio * audio = new Audio();
	audio->audioStream = stream;
	this->audioList.Add(audio);
	return audio;
}


/// Enables audio and unpauses all paused media.
void AudioManager::EnableAudio(){
	audioEnabled = true;
	Audio::audioEnabled = true;
	for (int i = 0; i < audioList.Size(); ++i){
		Audio * audio = audioList[i];
		audio->Resume();
	}
}

/// Disables audio and pauses all current playback indefinitely.
void AudioManager::DisableAudio(){
	audioEnabled = false;
	Audio::audioEnabled = false;
	for (int i = 0; i < audioList.Size(); ++i){
		Audio * audio = audioList[i];
		audio->Pause();
	}
}

void AudioManager::Update(){
	/// Get context, always
	ALCboolean result = alcMakeContextCurrent(alcContext);
	assert(result && "Unable to make alc context current");
	for (int i = 0; i < audioList.Size(); ++i){
		Audio * audio = audioList[i];
		audio->UpdateVolume(masterVolume);
		audio->Update();
	}
}


/// Sets master volume, from 0.0 to 1.0
void AudioManager::SetMasterVolume(float level){
	masterVolume = level;
	this->UpdateVolume();
}

/// Calls update volume for all audio
void AudioManager::UpdateVolume(){
	for (int i = 0; i < audioList.Size(); ++i){
		audioList[i]->UpdateVolume(masterVolume);
	}
}

#endif // USE_AUDIO
