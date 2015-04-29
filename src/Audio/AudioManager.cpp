// Emil Hedemalm
// 2013-03-22

#include "AudioManager.h"
#include "TrackManager.h"
#include "System/PreferencesManager.h"
// #include "../globals.h"

#include "OpenAL.h"

#include "Graphics/Camera/Camera.h"
#include "AudioMixer.h"
#include "Audio.h"
#include "Mutex/Mutex.h"

#include "Multimedia/Ogg/OggStream.h"

#include "File/LogFile.h"

#include <iostream>

#include "Windows/WindowsCoreAudio.h"

#include "OS/Sleep.h"

#ifdef USE_FMOD
#include <fmod_studio.hpp>
#include <fmod_studio_common.h>
#endif
//#include <fmod.hpp>

int64 audioNowMs = 0;
AudioManager * AudioManager::audioManager = NULL;
Mutex audioMessageQueueMutex;
int audioDriver;
int defaultAudioDriver = AudioDriver::BAD_DRIVER;
// Short stringstream for debugging using dlog();
// extern stringstream debugs;

String lastAudioInfo;

bool usingMasterMixer = false;

// Default constructor
AudioManager::AudioManager()
{
	// Set the global audio to this upon finishing construction.
	listener = NULL;
	initialized = false;
	pauseUpdates = false;
	audioEnabled = true;
	mute = false;
	masterVolume = 0.1f;
	shouldLive = true;
	audioDriver = AudioDriver::BAD_DRIVER;
	AudioMixer::AllocateMaster();
	bgmEnabled = true;

	// Create mutex for handling race-conditions/threading
	audioMessageQueueMutex.Create("audioMessageQueueMutex");
}

AudioManager::~AudioManager()
{
	audioList.ClearAndDelete();
	// Shutdown if not already done so!
	Shutdown();
	messageQueue.ClearAndDelete();
	AudioMixer::DeallocateMaster();
}

#include "Graphics/GraphicsManager.h"

/// If true, you may queue messages.
bool AudioManager::AudioProcessingActive()
{
	return GraphicsManager::GraphicsProcessingActive();
}

void AudioManager::SetDefaultAudioDriver(String fromString)
{
	String driverName = fromString.Tokenize(" \t")[1];
	if (driverName.Contains("WindowsCoreAudio"))
		defaultAudioDriver = AudioDriver::WindowsCoreAudio;
}


bool AudioManager::Initialize()
{
	LogAudio("Starting AudioManager...", INFO);

	/// Fill the category thingy with volumes
	categoryVolumes.Clear();
	for (int i = 0; i < AudioType::NUM_TYPES; ++i)
	{
		categoryVolumes.Add(1.f);
	}
	
	if (defaultAudioDriver != AudioDriver::BAD_DRIVER)
	{
		if (InitializeDriver(defaultAudioDriver))
		{
			audioDriver = defaultAudioDriver;
			initialized = true;
			return true;
		}
	}
	// Try the other drivers?
	for (int i = 0; i < AudioDriver::DRIVERS; ++i)
	{
		bool ok = InitializeDriver(i);
		if (ok)
		{
			audioDriver = i;
			initialized = true;
			return true;
		}
	}

	initialized = false;
	CheckALError("AudioMan - failed to initialize.");
	return false;
}

bool AudioManager::InitializeDriver(int driverID)
{
	switch(driverID)
	{
		case AudioDriver::OpenAL: return OpenAL::Initialize(); break;
	#ifdef WINDOWS
		case AudioDriver::WindowsCoreAudio:
			if (WMMDevice::Initialize())
			{
				usingMasterMixer = true;
				return true;
			} 
			return false;
			break;
	#endif
	}	
#ifdef USE_FMOD
	// Initialize FMOD
	result = 0;
	FMOD::Studio::System * system;
	result = FMOD::Studio::System::create(&system); // Create the Studio System object.
	if (result == FMOD_OK)
	{
		result = system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0);
	}
	while(true)
	{
		system->loadBankFile("filename", 0, &bank);
		system->update();
		system->playSound(sound, channelGroup, false, &channel);
	}	
#endif	
}


/// Called once in the deallocator thread when stop procedures have begun but before deallocation occurs.
void AudioManager::Shutdown()
{
	if (!initialized)
		return;
	shouldLive = false;

	/// Open AL cleanup.
	if (audioDriver == AudioDriver::OpenAL)
	{
		/// Free all OpenAL resources.
		AL_FREE_ALL
		/// Deallocate audio stufs, since that loop is here still..
		AudioBuffer::FreeAll();
	}

#ifdef WINDOWS
	// Try windows
	bool ok = WMMDevice::Deallocate();
	assert(ok);
#endif
	

#ifdef OPENAL
	ALSource::FreeAll();
	if (alcContext)
	{
		ALCboolean result = alcMakeContextCurrent( NULL );
		assert(result == ALC_TRUE);
		alcDestroyContext( alcContext );
		alcContext = NULL;
		int error = alcGetError(alcDevice);
		if (error != ALC_NO_ERROR)
		{
			LogAudio("ALC error"+String(error), ERROR);
		}
	}
	if (alcDevice)
	{
		ALCboolean result = alcCloseDevice( alcDevice );
		assert(result == ALC_TRUE);
		alcDevice = NULL;
	}
#endif
}

void AudioManager::Allocate()
{
	assert(audioManager == NULL);
	audioManager = new AudioManager();
}
void AudioManager::Deallocate()
{
	assert(audioManager);
	delete audioManager;
	audioManager = NULL;
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

List<Audio*> AudioManager::AllOfType(char type)
{
	List<Audio*> result;
	Audio * audio;
	for (int i = 0; i < audioList.Size(); ++i){
		audio = audioList[i];
		if (audio->type == type)
		{
			result.AddItem(audio);
		}
	}
	return result;
}

void AudioManager::ResumeAllOfType(char type)
{
	List<Audio*> all = AllOfType(type);	
	for (int i = 0; i < all.Size(); ++i)
	{
		Audio * audio = all[i];
		if (audio->IsPaused())
			audio->Resume();
	}
}

void AudioManager::PauseAllOfType(char type)
{
	if (!initialized)
		return;

	Audio * audio;
	for (int i = 0; i < audioList.Size(); ++i){
		audio = audioList[i];
		if (audio->type == type){
			Pause(audio);
		}
	}
}

void AudioManager::StopAllOfType(char type)
{
	Audio * audio;
	for (int i = 0; i < audioList.Size(); ++i)
	{
		audio = audioList[i];
		if (audio->type == type && audio->type)
		{
			audio->Stop(true);
		}
	}
}

void AudioManager::FadeOutAllOfType(char type, float seconds)
{
	Audio * audio;
	for (int i = 0; i < audioList.Size(); ++i)
	{
		audio = audioList[i];
		if (audio->type == type && audio->type)
		{
			audio->FadeOut(seconds);
		}
	}
}

void AudioManager::ToggleMute()
{
	this->mute = !mute;
	/// Set master mixer mute status?
	masterMixer->muted = mute;
	this->UpdateVolume();
}


/** Attempts to play audio from given source. Optional arguments control loop-mode and relative volume.
	Returns the relevant Audio object upon success.
*/
Audio * AudioManager::PlayFromSource(char type, String fromSource, bool repeat /*= false*/, float volume /*= 1.0f*/)
{
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
	lastAudioInfo = "AudioManager::PlayFromSource - audio loaded";
//	assert(audio->audioStream->source > 0);
	/// Generate audio source if not existing.
	if (!audio->CreateALObjects())
	{
		LogAudio("Unable to create AL objects for audio: "+audio->name, ERROR);
		delete audio;
		return NULL;
	}
//	audio->UpdateVolume(masterVolume);
	audio->Play();
	pauseUpdates = false;
	audioList.Add(audio);
	return audio;
}



Audio * AudioManager::Play(char type, String name, bool repeat, float volume)
{
    if (!initialized)
        return 0;
	if (!audioEnabled)
		return 0;
   // assert(initialized);

	pauseUpdates = true;

	// Look if we already have an element with same name
	Audio * audio;
	for (int i = 0; i < audioList.Size(); ++i){
		audio = audioList[i];
		// If we find a similar audio already in the array: Play it
		if (name == audio->name){
			/// Update the audio o-o
			audio->Play();
			audio->volume = volume;
			// Update volume
//			audio->UpdateVolume(masterVolume);
			return audio;
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
        return 0;
	}
	/// Generate audio source if not existing.
	if (audioDriver == AudioDriver::OpenAL)
	{
		if (!audio->CreateALObjects())
		{
			LogAudio("Unable to create AL objects for audio: "+audio->name, ERROR);
			delete audio;
			return NULL;	
		}
	}
//	audio->UpdateVolume(masterVolume);
//	assert(audio->audioStream->source > 0);
	audioList.Add(audio);
	audio->Play();
	pauseUpdates = false;
	return audio;
}

#define CHECK_INITIALIZED {if (!initialized){std::cout<<"\nAudio manager not initialized. Returning"; return;}}
#define CHECK_AUDIO_ENABLED {if (!audioEnabled){std::cout<<"\nAudio disabled. Returning"; return;}}

/// Name is should correspond to filename or path, expected locatoin in ./sound/sfx/
void AudioManager::PlaySFX(String name, float volume /*= 1.f*/)
{
	static int sfxPlayed = 0;
	++sfxPlayed;
	CHECK_INITIALIZED
	CHECK_AUDIO_ENABLED

	pauseUpdates = true;

	LogAudio("PlaySFX: "+name, INFO);

	// Create audio
//	std::cout<<"\nAudio not found, tryng to create and load it.";
	Audio * audio = new Audio(AudioType::SFX, name, false, volume);
	bool loadResult = audio->Load();
//	std::cout<<"\nLoad result: "<<loadResult;
//	assert(loadResult && "Could not load audio data?");
	if (loadResult == false)
	{
        delete audio;
		LogAudio("ERROR: Unable to load audio \""+name+"\". Aborting playback.", ERROR);
        return;
	}
	/// Generate audio source if not existing.
	if (audioDriver == AudioDriver::OpenAL)
		audio->CreateALObjects();
//	audio->UpdateVolume(masterVolume);
	audio->deleteOnEnd = true;
//	assert(audio->audioStream->source > 0);
	audioList.Add(audio);
	audio->Play();
	pauseUpdates = false;
}
	
/// Plays target BGM, pausing all others. Default sets to repeat.
Audio * AudioManager::PlayBGM(String name, float volume /* = 1.f */)
{
	if (!bgmEnabled)
		return NULL;
	return Play(AudioType::BGM, name, true, volume);
}


// Pause
void AudioManager::Pause(String name)
{
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

void AudioManager::Pause(Audio * audio)
{
	if (!initialized)
		return;
	audio->Pause();
}

void AudioManager::Stop(String name)
{
	// Look if we already have an element with same name
	Audio * audio = GetAudioByName(name);
	if (audio)
		audio->Stop(false);
}

Audio * AudioManager::GetAudioByName(String name)
{
	for (int i = 0; i < audioList.Size(); ++i)
	{
		Audio * audio = audioList[i];
		if (audio->name == name)
			return audio;
	}
	return NULL;
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
void AudioManager::EnableAudio()
{
	audioEnabled = true;
	for (int i = 0; i < audioList.Size(); ++i){
		Audio * audio = audioList[i];
		audio->Resume();
	}
}

/// Disables audio and pauses all current playback indefinitely.
void AudioManager::DisableAudio()
{
	audioEnabled = false;
	for (int i = 0; i < audioList.Size(); ++i){
		Audio * audio = audioList[i];
		audio->Pause();
	}
}

/// o.o
void AudioManager::RegisterAudio(Audio * audio)
{
	assert(audio->registeredForRendering == false);
	audioList.AddItem(audio);
	audio->registeredForRendering = true;
}


void AudioManager::Update()
{
	audioNowMs = Time::Now().Milliseconds();

	if (!initialized)
	{
		std::cout<<"\nAudio manager not initialized. Skipping da beat.";
		return;
	}
	lastAudioInfo = "AudioManager::Update";

	// Process messages.
	lastAudioInfo = "AudioManager::ProcessAudioMessages";
	ProcessAudioMessages();

	// Update listener position
	if (listener)
		listenerPosition = listener->Position();

	// Then update volumes and buffer stuff.
	lastAudioInfo = "AudioManager::Update - volumes and buffering";
	int playingAudio = 0;
	for (int i = 0; i < audioList.Size(); ++i)
	{
		Audio * audio = audioList[i];
//		audio->UpdateVolume(mute? 0: masterVolume);
		// See if it ended.
		switch(audio->state)
		{
			case AudioState::ENDED:
			{
				// Remove it?
				if (audio->deleteOnEnd)
				{
					audioList.Remove(audio);
					delete audio;
					--i;
					continue;
				}
				break;
			}
			case AudioState::ENDING:
			{
				// Flag as ended after the audio marker in the mixer has been removed.
				if (usingMasterMixer)
				{
					ABM * abm = masterMixer->GetMarker(audio);
					if (!abm || abm->pcmQueueIndex <= 0)
						audio->state = AudioState::ENDED;
				}
				playingAudio++;
				break;
			}
			case AudioState::PLAYING:
				playingAudio++;
				break;
			case AudioState::READY:
			case AudioState::PAUSED:
				continue;
		}
		audio->Update();
	}
	// No audio playing? Sleep moar.
	if (playingAudio <= 0)
	{
		Sleep(20);
		return;
	}
	/// Send mixed audio to driver, if custom mixer enabled/driver requiring it
	if (usingMasterMixer)
		masterMixer->Update();
}

Vector3f AudioManager::ListenerPosition()
{
	return listenerPosition;
}



/// Sets master volume, from 0.0 to 1.0
void AudioManager::SetMasterVolume(float level)
{
	ClampFloat(level, 0, 1.f);
	masterVolume = level;
	masterMixer->volume = level;
	this->UpdateVolume();
}

/// Returns the current volume for target category (0.0 to 1.0)
float AudioManager::CategoryVolume(int category)
{
	bool good = category < categoryVolumes.Size() && category >= 0;
	assert(good);
	if (!good)
		return 0.f;
	return categoryVolumes[category];

}

void AudioManager::StopAndRemoveAll()
{
	while(audioList.Size())
	{
		Audio * audio = audioList[0];
		audio->Stop(false);
		delete audio;
		audioList.Remove(audio);
	}
}

/// Calls update volume for all audio
void AudioManager::UpdateVolume()
{
	/*
	for (int i = 0; i < audioList.Size(); ++i)
	{
		Audio * audio = audioList[i];
		if (mute)
			audio->UpdateVolume(0.f);
		else
			audio->UpdateVolume(masterVolume);
	}
	*/
}

void AudioManager::QueueMessage(AudioMessage * am)
{
	/// Claim mutex.
	while(!audioMessageQueueMutex.Claim(-1))
		;
	messageQueue.AddItem(am);
	// Release mutex
	audioMessageQueueMutex.Release();
}

void AudioManager::QueueMessages(const List<AudioMessage*> & messageList)
{
	while(!audioMessageQueueMutex.Claim(-1));
	messageQueue.Add(messageList);
	audioMessageQueueMutex.Release();
}


// yer.
void AudioManager::ProcessAudioMessages()
{
	if (!messageQueue.Size())
		return;
	List<AudioMessage*> messagesToProcess;
	/// Claim mutex.
	while(!audioMessageQueueMutex.Claim(-1))
		;
	messagesToProcess = messageQueue;
	messageQueue.Clear();
	// Release mutex
	audioMessageQueueMutex.Release();


	/// Process them.
	for(int i = 0; i < messagesToProcess.Size(); ++i)
	{
		AudioMessage * am = messagesToProcess[i];
		am->Process();
	}
	messagesToProcess.ClearAndDelete();
}


extern THREAD_HANDLE audioThread;

PROCESSOR_THREAD_START(AudioManager)
{
	LogAudio("Audio thread starting", INFO);
	/// Create AL Context, etc.
	if (!AudioMan.Initialize())
	{
		QuitApplicationFatalError("AudioManager failed to initialize. See /log/AudioLog.txt");
		goto audioThreadEnd;
//		RETURN_NULL(audioThread);
	}

	// Do stuff o.o
	while(AudioMan.shouldLive)
	{	
		/// Sleep 50 ms each frame?
		SleepThread(10);
		AudioMan.Update();
	}
audioThreadEnd:

	/// Shut down all remaining music.
	AudioMan.StopAndRemoveAll();

	AudioMan.Shutdown();
	// Cleanup
//	AudioMan.Deallocate();

	/// Inform that the thread has ended.
	RETURN_NULL(audioThread);
}
