/// Emil Hedemalm
/// 2014-09-22
/// Wav (Waveform Audio File Format) streaming class.

#ifndef WAV_STREAM_H
#define WAV_STREAM_H

#include "../MultimediaStream.h"

#include <fstream>

namespace WavFormats 
{
	enum {
		WAVE_FORMAT_PCM = 0x0001,
		WAVE_FORMAT_IEEE_FLOAT = 0x0003,
		WAVE_FORMAT_ALAW = 0x0006,
		WAVE_FORMAT_MULAW = 0x0007,
		WAVE_FORMAT_EXTENSIBLE = 0xFFFE,
	};
};

class WavStream : public MultimediaStream
{
public:
	WavStream();
	virtual ~WavStream();

	/// Attempts to open target file. Returns false upon failure.
    virtual bool Open(String path);
	/// Closes target file and stream.
	virtual void Close();

	/** Buffers audio data into buf, up to maximum of maxBytes. Returns amount of bytes buffered. 
		Amount of bytes may reach 0 even if the media has not ended. Returns -1 on error. Returns -2 if the stream is paused.
		If loop is set to true it will try to automatically seek to the beginning when it reaches the end of the file.
	*/
	virtual int BufferAudio(char * buf, int maxBytes, bool loop);

private:
	/** Fetches next data chunk from the file stream into the data buffer. Returns false if it fails/reaches end of stream?
		Re-allocates the dataBuffer as needed.
	*/
	bool GetNextChunk(bool loop);
	/// o.o
	void GoToDataChunk(int chunkIndex);

	int chunkSize;
	int formatCode;
	int averageBytesPerSecond;
	int blockAlign;
	int bitsPerSample;
	int extensionSize;
	int validBitsPerSample;
	int channelMask;
	int subFormat;

	std::fstream file;

	/** Temporary data buffer which will contain all samples (PCA) data from the current data chunk.
		dataBufferSize dictates its current length.
	*/
	char * dataBuffer;
	/// Dictates the length of the dataBuffer
	int dataBufferSize;
	/// Dictates how many bytes of the current data chunk and dataBuffer which has been buffered in the last call to BufferAudio
	int bytesBufferedInCurrentChunk;
};

#endif

