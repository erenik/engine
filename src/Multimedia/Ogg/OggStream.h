// Emil Hedemalm
// 2013-03-22

#include "Libs.h"

#ifdef BUILD_OGG

#ifndef OGG_STREAM_H
#define OGG_STREAM_H

#include "String/AEString.h"
#include <iostream>
#include <Util.h>
#include <fstream>
//using namespace std;
//using std::string;

#include "Multimedia/MultimediaStream.h"

/// Include all necessary files for Ogg container playback. Or.. subclass it? Not sure D:
#include <ogg/ogg.h>
/// Vorbis for audio.
#include <codec.h>
#include <vorbisenc.h>
#include <vorbisfile.h>
/// Also Opus for audio http://www.opus-codec.org/ :D
#include <opusfile.h>
/// Theora for video.
#include <theora/theoradec.h>
#ifdef WINDOWS
#include <theora/decint.h>
#endif

class OggStream : public MultimediaStream {
public:
	// Default constructor, initializes crap to 0.
	OggStream();
	virtual ~OggStream();

	/// Attempts to open target file. Returns false upon failure.
    virtual bool Open(String path);
	/// Closes target file and stream.
	virtual void Close();

	/// Starts buffering of the stream. Nothing is done until this command has been executed successfully, following a call to Open.
	virtual bool Play();
	/// Pauses the stream.
	virtual bool Pause();
	/// Seeks to target time in milliseconds.
	virtual bool Seek(int toTime);
	/** Streams the next frame. Does not perform any type of check that the frame is beyond current time, meaning the MultimediaManager has this responsibility!
		Frames to pass defines how many frames are to be evaluated. Default value is 1.
	*/
	virtual bool StreamNextFrame(int framesToPass = 1);
	/** Buffers audio data into buf, up to maximum of maxBytes. Returns amount of bytes buffered.
		Amount of bytes may reach 0 even if the media has not ended. Returns -1 on error. Returns -2 if the stream is paused.
		If loop is set to true it will try to automatically seek to the beginning when it reaches the end of the file.
	*/
	virtual int BufferAudio(char * buf, int maxBytes, bool loop);

	/** Returns a texture which will be updated as the stream progresses.
		If the stream lacks graphics NULL will be returned.
		If the texture has the rebufferize flag set it has to be re-buffered to gain new frame data.
	*/
	virtual Texture * GetFrameTexture();

	/// Returns amount of channels present in the audio stream.
//	virtual int AudioChannels();
	/// Gets frequency of the audio. This is typically 48000 or similar?
//	virtual int AudioFrequency();

	

private:

	/// Attempts to open Vorbis playback from the file-stream.
	bool OpenVorbis();
	/// Attempts to open Opus playback from the file-stream.
	bool OpenOpus();
	/// Attemps to open Theora playback from the file-stream.
	bool OpenTheora();

	/** Current frame data in a format liked by the Texture class!
		Pixel index psi below. and the pixel index with offsets of 0 to 3 give the RGBA components respectively.
		int psi = y * width * bpp + x * bpp;
			buf[psi] += color[0] * 255.0f;
			buf[psi+1] += color[1] * 255.0f;
			buf[psi+2] += color[2] * 255.0f;
			buf[psi+3] += alpha * 255.0f;
		}
	*/
	int frameWidth;
	int frameHeight;
	int frameBufferSize;
	/// Buffer big enough to hold a single row of pixel-data.
	unsigned char * rowBufferData;
	int rowBufferSize;
	/// Pointer to the texture we update each frame.
	Texture * frameTexture;

	/// File stream handle.
	std::fstream file;
	bool oggSyncStateUsed; // If used or not.
	/// Ogg synchronization state, for parsing Ogg pages
	ogg_sync_state oggSyncState;
	/// List of states for encode/decode status of all logical bitstreams
	ogg_stream_state oggStreamState;

	/// Used when parsing the ogg file.
	ogg_page oggPage;
	ogg_packet oggPacket;

	/// Ogg vars
	FILE*           oggFile;       // File handle

	/// Theora
	bool hasTheora;
	int theoraPackets;
	th_info	theoraInfo;
	th_comment theoraComment;
	th_setup_info * theoraSetupInfo;
	th_dec_ctx * theoraDecoderContext;

	/// Vorbis
	bool hasVorbis;
	OggVorbis_File  oggVorbisFile;     // Stream handle
	vorbis_info*    vorbisInfo;    // Some formatting data
	vorbis_comment* vorbisComment; // User comments
	double oggVorbisTime;

	/// Opus
	bool hasOpus;
	OggOpusFile * oggOpusFile;
	const OpusHead * opusHead;
	const OpusTags * opusTags;
	OpusFileCallbacks opusFileCallbacks;

};

#endif

#endif // BUILD_OGG