/// Emil Hedemalm
/// 2014-09-22
/// Wav (Waveform Audio File Format) streaming class.
/// Wav Specification: http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html

#include "WavStream.h"

#include "../MultimediaTypes.h"
#include <cstring>

// Default constructor, See types in MultimediaTypes.h
WavStream::WavStream()
	: MultimediaStream(MultimediaType::WAV)
{	
	dataBuffer = NULL;
	dataBufferSize = 0;

	chunkSize = 
		formatCode = 
		audioChannels = 
		samplesPerSecond = 
		averageBytesPerSecond = 
		bytesBufferedInCurrentChunk =
		bitsPerSample =
		extensionSize =
		blockAlign = 0;
}

WavStream::~WavStream()
{
	Close();
	if (dataBuffer)
		delete[] dataBuffer;
}

	
/// Attempts to open target file. Returns false upon failure.
bool WavStream::Open(String path)
{
#define Error(s) { file.close(); std::cout<<"\nError: "<<s; return false;}
	// Save path we're streaming from.
	this->path = path;

	file.open(path.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!file.is_open())
	{
		Error("Unable to open file.");
	}

	// http://enigma-dev.org/forums/index.php?topic=730.0;wap2
//	-- Code: (C) ---FILE* f = fopen("audio.wav", "fb");
//	char xbuffer[5];

	const int BUF_SIZE = 20;
	char buf[BUF_SIZE];
	memset(buf, 0, BUF_SIZE);
	
	// First the RIFF
	file.read(buf, 4);
	if (String(buf) != "RIFF")
		Error("File does not contain starting \'RIFF\'");


	// Then the chunks
	int chunks;
	file.read((char*)&chunks, sizeof(int));

	// Then the WAVE fourcc
	file.read(buf, 4);
	if (String(buf) != "WAVE")
		Error("File does not contain starting fourcc \'WAVE\'");

	// Save current position so that we can jump back to it later?

	// Then the actual WAVE chunks!
	file.read(buf, 4);
	if (String(buf) != "fmt ")
		Error("Expected format chunk.");

	file.read((char*) &chunkSize, 4);

	// Begin of actual format chunk contents
	file.read((char*) &formatCode, 2);
	file.read((char*) &audioChannels, 2);
	file.read((char*) &samplesPerSecond, 4);
	file.read((char*) &averageBytesPerSecond, 4);
	file.read((char*) &blockAlign, 2);
	file.read((char*) &bitsPerSample, 2);
	// 16 bytes read now
	if (chunkSize > 16)
		file.read((char*) &extensionSize, 2);
	// 18 with extension size
	if (extensionSize == 22)
	{
		file.read((char*) &validBitsPerSample, 2);
		file.read((char*) &channelMask, 4);
		file.read((char*) &subFormat, 16);
	}
	// 40 if the extension is included

	// Get first chunk?
	while(dataBuffer == 0)
		GetNextChunk(false);

	return true;
}
/// Closes target file and stream.
void WavStream::Close()
{
	file.close();
}

/** Buffers audio data into buf, up to maximum of maxBytes. Returns amount of bytes buffered. 
	Amount of bytes may reach 0 even if the media has not ended. Returns -1 on error. Returns -2 if the stream is paused.
	If loop is set to true it will try to automatically seek to the beginning when it reaches the end of the file.
*/
int WavStream::BufferAudio(char * buf, int maxBytes, bool loop)
{
	if (!file.is_open())
		return -1;

	int bytesBuffered = 0;
	// Assume we have a chunk already, as it should have been loaded on first opening of the file.
	int bytesAvailable = dataBufferSize - bytesBufferedInCurrentChunk;
	// If we have more than enough, a single stream is enough.
	int bytesFirstStream = bytesAvailable > maxBytes? maxBytes : bytesAvailable;
	int bytesToBuffer = maxBytes;

	assert(formatCode == WavFormats::WAVE_FORMAT_PCM);

	// Copy first batch.
	memcpy(buf, dataBuffer + bytesBufferedInCurrentChunk, bytesFirstStream);
	bytesBufferedInCurrentChunk += bytesFirstStream;
	bytesToBuffer -= bytesFirstStream;
	bytesBuffered += bytesFirstStream;

	/// Until we have reached the designated total..
	while(bytesBuffered < maxBytes)
	{
		// First loop should be skipped if everything was available in the current chunk.
		// Therefore: grab a new chunk!
		bool ok = GetNextChunk(loop);
		if (!ok)
		{
			// No new chunk? End of stream!
			return bytesBuffered;
		}
		// Re-evaluate the bytes available.
		bytesAvailable = dataBufferSize - bytesBufferedInCurrentChunk;
		// Stream again.
		int bytesNStream = bytesAvailable > bytesToBuffer? bytesToBuffer : bytesAvailable;
		memcpy(buf + bytesBuffered, dataBuffer + bytesBufferedInCurrentChunk, bytesNStream);
		bytesBufferedInCurrentChunk += bytesNStream;
		bytesToBuffer -= bytesNStream;
		bytesBuffered += bytesNStream;
	}
	return bytesBuffered;
}

/** Fetches next data chunk from the file stream into the data buffer. Returns false if it fails/reaches end of stream?
	Re-allocates the dataBuffer as needed.
*/
bool WavStream::GetNextChunk(bool loop)
{
	// Check for next data chunk?
	char chunkID[5];
	memset(chunkID, 0, 5);
	file.read(chunkID, 4);
	if (String(chunkID) == "data")
	{
		file.read((char*)&chunkSize, 4);

		// Re-allocate the buffer if needed.
		if (chunkSize > dataBufferSize)
		{
			if (dataBuffer)
				delete[] dataBuffer;
			dataBufferSize = chunkSize;
			dataBuffer = new char[dataBufferSize];
		}
		// Fetch the sample data
		file.read(dataBuffer, chunkSize);
		// Read pad byte if needed
		if (chunkSize % 2 == 1)
			file.read(chunkID, 1);
	}
	else if (String(chunkID) == "fact")
	{
		// Will contain info about compression or something?
		file.read((char*)&chunkSize, 4);
		int samplesPerChannel = 0;
		file.read((char*)&samplesPerChannel, 4);
		if (chunkSize > 4)
		{
			assert(false && "Implement? or error?");
		}
	}
	else if (loop)
	{
		// Couldn't fnd data.. but are we looping? If so go to first chunk again!
		GoToDataChunk(0);
	}
	else {
		// Handle error..
		return false;
	}
	// Reset amount of bytes we've buffered from the chunk
	bytesBufferedInCurrentChunk = 0;
	return true;
}

/// o.o
void WavStream::GoToDataChunk(int chunkIndex)
{
	assert(false && "Implement");
}