/// Emil Hedemalm
/// 2014-07-24
/// A general audio buffer. Encapsulates the alBuffers, so that they may be re-used.

#ifndef AUDIO_BUFFER_H
#define AUDIO_BUFFER_H

#include "OpenAL.h"

/// Buffer object that stores state of a buffer of RAW PCM data.
class AudioBuffer 
{
protected:
	AudioBuffer();
public:
	/// Returns a new buffer to be used.
	static AudioBuffer * New();
		/// Frees this buffer. Calls the static handler equivalent.
	virtual void Free();

	// Static handler: Frees all buffers.
	static void FreeAll();
	// Static handler:Frees this buffer.
	static void Free(AudioBuffer * buffer);

	bool AttachTo(unsigned int alSource);
	bool DetachFrom(unsigned int alSource);

	/// Default false. Should be set to true when attaching/queueing it to source. Must be false when de-allocation is to be done.
	bool attached;

	/// If it has gotten good new data.
	bool buffered;
	// The actual buffer number.
	unsigned int alBuffer;

private:
	static AudioBuffer * GetFree();
	/// Used by the static handler for re-using buffers.
	bool inUse;
	static List<AudioBuffer*> buffers;

};

#endif

