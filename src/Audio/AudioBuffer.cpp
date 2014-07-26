/// Emil Hedemalm
/// 2014-07-24
/// A general audio buffer. Sub-class it to work for your specific audio-system.


#include "AudioBuffer.h"

List<AudioBuffer*> AudioBuffer::buffers;


AudioBuffer::AudioBuffer()
: inUse(false), attached(false), buffered(false)
{
	alBuffer = 0;
}

AudioBuffer * AudioBuffer::New()
{
	// Check for a free source.
	AudioBuffer * oldBuffer = GetFree();
	if (oldBuffer)
	{
		oldBuffer->inUse = true;
		return oldBuffer;
	}
	AudioBuffer * newBuffer = new AudioBuffer();
	alGenBuffers(1, &newBuffer->alBuffer);
	newBuffer->inUse = true;
	buffers.Add(newBuffer);
	return newBuffer;
}

/// Frees this buffer. Calls the static handler equivalent.
void AudioBuffer::Free()
{
	Free(this);
	assert(!attached);
	assert(!buffered);
}


void AudioBuffer::FreeAll()
{
	// Fetch context if needed.
	ALCboolean result = alcMakeContextCurrent(alcContext);
	assert(result && "Unable to make alc context current");
	int freed = 0;
	while(buffers.Size())
	{

		AudioBuffer * buffer = buffers[0];
		/// Ensure that the buffer is not attached to anything!
		assert(!buffer->attached);
		alDeleteBuffers(1, &buffer->alBuffer);
		buffers.RemoveIndex(0);
		int error = CheckALError();
		if (error = AL_NO_ERROR)
			++freed;
	}
	std::cout<<"\n"<<freed<<" alBuffers freed.";
}

/// oo
void AudioBuffer::Free(AudioBuffer * buffer)
{
	buffer->inUse = false;
	buffer->attached = false;
}

bool AudioBuffer::AttachTo(ALuint alSource)
{
	assert(attached == false);
	alSourceQueueBuffers(alSource, 1, &alBuffer);
	int error = CheckALError();
	if (error == AL_NO_ERROR)
	{
		attached = true;
		return true;
	}
	return false;
};

bool AudioBuffer::DetachFrom(ALuint alSource)
{
	assert(attached);
	alSourceUnqueueBuffers(alSource, 1, &alBuffer);
	int error = AssertALError();
	if (error == AL_NO_ERROR)
	{
		attached = false;
		return true;
	}
	return false;
}


AudioBuffer * AudioBuffer::GetFree()
{
	for (int i = 0; i < buffers.Size(); ++i)
	{
		AudioBuffer * buffer = buffers[i];
		if (!buffer->inUse)
		{
			return buffer;
		}
	}
	return NULL;
}