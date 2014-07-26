/// Emil Hedemalm
/// 2014-07-21
/// Handler for alSources and alBuffers, since they act strange after a while on windows if creating new ones all the time

#include "OpenAL.h"


// Open AL device and context, similar to OpenGL device and context!
ALCdevice * alcDevice = 0;		// Device
ALCcontext * alcContext = 0;	// Rendering audio context

int CheckALError()
{
	int error = alGetError();
	switch(error)
	{
		case AL_ILLEGAL_COMMAND:
	//	case AL_INVALID_OPERATION:
			std::cout<<"\nIllegal operation/command."; 
			assert(false);
			break;
		case AL_NO_ERROR:
			return error;
		case AL_INVALID_NAME:
			std::cout<<"\nAL invalid name."; break;
		case AL_INVALID_VALUE:
			std::cout<<"\nAL invalid value."; break;
		case AL_INVALID_ENUM:
			std::cout<<"\nAL Invalid enum."; break;
		case AL_OUT_OF_MEMORY:
			std::cout<<"\nAL out of memory!"; assert(false); break;
		default:
			assert(false && "Unidentified al error D:");
			break;
	}
	return error;
}

int AssertALError()
{
	int error = CheckALError();
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
	ALSource * newSource = new ALSource();
	alGenSources(1, &newSource->alSource);
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

