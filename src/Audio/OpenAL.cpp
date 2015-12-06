/// Emil Hedemalm
/// 2014-07-21
/// Handler for alSources and alBuffers, since they act strange after a while on windows if creating new ones all the time

#include "OpenAL.h"
#include "String/AEString.h"
#include "File/LogFile.h"

#ifdef OPENAL

// Open AL device and context, similar to OpenGL device and context!
ALCdevice * alcDevice = 0;		// Device
ALCcontext * alcContext = 0;	// Rendering audio context

int CheckALError(const String & errorLocation)
{
	int error = alGetError();
	String errorMsg;
	switch(error)
	{
	//	case AL_ILLEGAL_COMMAND:
		case AL_INVALID_OPERATION:
			errorMsg = "Illegal operation/command."; 
			assert(false);
			break;
		case AL_NO_ERROR:
			return error;
		case AL_INVALID_NAME: errorMsg = "AL invalid name."; break;
		case AL_INVALID_VALUE: errorMsg = "AL invalid value."; break;
		case AL_INVALID_ENUM: errorMsg = "AL Invalid enum."; break;
		case AL_OUT_OF_MEMORY: errorMsg = "AL out of memory!"; break;
		default:
			assert(false && "Unidentified al error D:");
			break;
	}
	std::cout<<"\n"<<errorLocation<<" AL error "<<errorMsg;
	return error;
}

int AssertALError(const String & errorLocation)
{
	int error = CheckALError(errorLocation);
	assert(error == AL_NO_ERROR);
	return error;
}		

bool OpenAL::Initialize()
{
//	std::cout<<"\nInitializing OpenAL Utilities...";
	LogAudio("Checking for available audio devices...", INFO);
	// Just remove initial error blerhp?
	alGetError();
	if (alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") == AL_TRUE)
	{
	//	std::cout<<"\nEnumeration extension found.";
		const char * deviceSpecifier = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		const char * defaultDevice = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
		LogAudio("Device specifier: "+String(deviceSpecifier)+"\nDefault device: "+String(defaultDevice), INFO);
	}
	else
		LogAudio("Enumeration extension not available!", WARNING);

	// TODO: Proper initialization
	alcDevice = alcOpenDevice(NULL); // Open default alcDevice
	if (alcDevice == 0)
	{
		LogAudio("ERROR: Unable to open AL device.", FATAL);
		//	assert(alcDevice && "Unable to open AL device in AudioManager::Initialize");
		//audio = this;
		return false;
	}
	int tries = 0;
	while(alcContext == NULL)
	{
		alcContext = alcCreateContext(alcDevice, NULL);
		if (alcContext == NULL)
		{
			if (tries > 5)
			{
				LogAudio("Unablet to create alc context.", FATAL);
				return false;
			}
			LogAudio("Unable to create alc context, waiting and trying again.", ERROR);
			++tries;
		}
	}
	int result = alcMakeContextCurrent(alcContext);
	if (result == ALC_FALSE)
	{
		LogAudio("ERROR: Unable to make AL context active.", FATAL);
		return false;
	}
	CheckALError("AudioMan - making context current.");

	/// Set initial values, like the listener position?

//	std::cout<<"Queueing initial playback";
	// Set initialized to true after all initialization has been completed correctly.
	std::cout<<"\nOpenAL initialized successfully.";
	return true;
}
bool OpenAL::Deallocate()
{
	
	assert(false);
	return false;
}


List<ALSource*> ALSource::sources;

ALSource::ALSource()
	: alSource(0), inUse(false)
{
};

unsigned int ALSource::New()
{
	// Check for a free source.
	ALSource * oldSource = GetFree();
	if (oldSource)
	{
		oldSource->inUse = true;
		return oldSource->alSource;
	}
	ALuint alSource;
	CheckALError("ALSource::New");
	alGenSources(1, &alSource);
	int error;
	if (error = CheckALError("Error generating source in ALSource::New, function alGenSources"))
	{
		if (error != AL_NO_ERROR)
		{
			LogAudio("alGenSources call failed, returned 0.", ERROR);
			return AL_BAD_SOURCE;
		}
	}
	ALSource * newSource = new ALSource();
	newSource->alSource = alSource;
	newSource->inUse = true;
	sources.Add(newSource);
	return newSource->alSource;
}

void ALSource::Free(unsigned int alSource)
{
	for (int i = 0; i < sources.Size(); ++i)
	{
		ALSource * source = sources[i];
		if (source->alSource == alSource)
			source->inUse = false;
	}
}

void ALSource::FreeAll()
{
	while(sources.Size())
	{
		ALSource * source = sources[0];
		alDeleteSources(1, &source->alSource);
		sources.RemoveIndex(0);
		// Actually delete it too..
		delete source;
	}
}


ALSource * ALSource::GetFree()
{
	for (int i = 0; i < sources.Size(); ++i)
	{
		ALSource * source = sources[i];
		if (!source->inUse)
			return source;
	}
	return NULL;
}

#endif
