/// Emil Hedemalm
/// 2014-07-21
/// Handler for alSources and alBuffers, since they act strange after a while on windows if creating new ones all the time

#ifndef OPEN_AL_H
#define OPEN_AL_H

#include "List/List.h"
#include "AudioBuffer.h"

#define BUILD_AL
#ifdef BUILD_AL
#include <AL/al.h>
#include <AL/alc.h>
// #include <AL/alut.h>
#endif

int CheckALError();
int AssertALError();

// Open AL device and context, similar to OpenGL device and context!
extern ALCdevice * alcDevice;		// Device
extern ALCcontext * alcContext;	// Rendering audio context

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
