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
		case AL_ILLEGAL_COMMAND:
	//	case AL_INVALID_OPERATION:
			errorMsg = "Illegal operation/command."; 
			assert(false);
			break;
		case AL_NO_ERROR:
			return error;
		case AL_INVALID_NAME: errorMsg = "AL invalid name."; break;
		case AL_INVALID_VALUE: errorMsg = "AL invalid value."; break;
		case AL_INVALID_ENUM: errorMsg = "AL Invalid enum."; break;
		case AL_OUT_OF_MEMORY: errorMsg = "AL out of memory!"; assert(false); break;
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

List<ALSource*> ALSource::sources;

ALSource::ALSource()
	: alSource(NULL), inUse(false)
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
	alGenSources(1, &alSource);
	if (alSource == 0)
	{
		int error = CheckALError("Error generating source in ALSource::New, function alGenSources");
		if (error != AL_NO_ERROR)
		{
			LogAudio("alGenSources call failed, returned 0.", ERROR);
			return 0;
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
