// Emil Hedemalm
// 2013-03-22

#include "../AudioSettings.h"

#ifndef WAV_STREAM_H
#define WAV_STREAM_H

#include <String>
#include <iostream>
#include <Util.h>
//using namespace std;
//using std::string;

#include <OpenAL/al.h>

#define FOURKB		4096
#define BUFFER_SIZE (FOURKB * 64)

class WavStream
{
    public:
		WavStream();	// Default constructor, initializes crap to 0.

		friend class AudioManager;
		friend class Audio;

        void Open(const char * path); // obtain a handle to the file
        void Release();         // release the file handle
        void Display();         // display some info on the Ogg
        bool Playback();        // play the Ogg stream
		bool Restart();			// Try begin buffering anew from the start
		bool Pause();			// Stop the Ogg stream
        bool IsPlaying();         // check if the source is playing
        bool Update();          // update the stream if necessary
 
    protected:
 
        bool Stream(ALuint buffer);   // reloads a buffer
        void Empty();                 // empties the queue
        void Check();                 // checks OpenAL error state
        String ErrorString(int code); // stringify an error code

	private:
 
		char	buf[BUFFER_SIZE];		// Statically allocated buffer to avoid runtime allocation for temporary buffers.

        FILE*           oggFile;       // File handle
        OggVorbis_File  oggStream;     // Stream handle
        vorbis_info*    vorbisInfo;    // Some formatting data
        vorbis_comment* vorbisComment; // User comments
 
        ALuint alBuffers[2];		// Front and back buffers
		bool   bufferIsQueued[2];	// IDs of currently occupied buffers
        ALuint source;				// Audio source ID
        ALenum format;				// Internal format
};

#endif

#endif // USE_VORBIS