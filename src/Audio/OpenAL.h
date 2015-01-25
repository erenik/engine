/// Emil Hedemalm
/// 2014-07-21
/// Handler for alSources and alBuffers, since they act strange after a while on windows if creating new ones all the time

#ifndef OPEN_AL_H
#define OPEN_AL_H

#include "List/List.h"
#include "AudioBuffer.h"

// Includes Libs generated file which specifies which audio libraries we are to link to.
#include "Libs.h"

#ifdef OPENAL
	#include <AL/al.h>
	#include <AL/alc.h>
	// #include <AL/alut.h>

	int CheckALError();
	int AssertALError();

	// Open AL device and context, similar to OpenGL device and context!
	extern ALCdevice * alcDevice;		// Device
	extern ALCcontext * alcContext;	// Rendering audio context

#define GRAB_AL_CONTEXT {ALCboolean result = alcMakeContextCurrent(alcContext); assert(result && "Unable to make alc context current");}


#define AL_FREE_ALL {ALSource::FreeAll();}

#else // NO AL
#define AL_FREE_ALL {}
#define GRAB_AL_CONTEXT 
#endif

class ALSource 
{
private:
	ALSource();
public:
	/// Returns a new buffer to be used.
	static unsigned int New();
	// Frees all buffers.
	static void FreeAll();
	static void Free(unsigned int alSource);
private:
	static ALSource * GetFree();
	// The ID as it is known to al.
	unsigned int alSource;
	bool inUse;
	static List<ALSource *> sources;
};

#endif
