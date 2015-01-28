// Emil Hedemalm
// 2013-03-22

#include "OggStream.h"

#ifdef BUILD_OGG

#include "Multimedia/MultimediaTypes.h"
#include "TextureManager.h"

#include "OS/Sleep.h" // For sleep..
#include <fstream>
#include "Texture.h"

// For memcpy and stuff
#include <cstring>

/// Theora required maybe?
#include <sys/timeb.h>
#include <fcntl.h>

#ifdef THEORA
#include <theora/codec.h>
#endif

#include "Audio/AudioManager.h"


#include "OS/OS.h"
#ifdef LINUX
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <limits>
#endif
#define INT_MAX 2147483647


using std::string;

OggStream::OggStream()
: MultimediaStream(MultimediaType::OGG)
{
	this->oggFile = 0;
	
	hasTheora = false;
	hasVorbis = false;
	theoraSetupInfo = NULL;
	theoraDecoderContext = NULL;
	rowBufferData = NULL;
	frameTexture = NULL;
	rowBufferSize = 0;

	vorbisInfo = 0;
	vorbisComment = 0;

	hasOpus = false;
	oggOpusFile = NULL;

	oggSyncStateUsed = false;
}

OggStream::~OggStream()
{
	/// Close the stream if not already closed.
	Close();
	if (frameTexture)
		delete frameTexture;
}


FILE* outfile = NULL;

/* Helper; just grab some more compressed bitstream and sync it for
   page extraction */
int ReadOggData(std::fstream & fromFile, ogg_sync_state * intoOss)
{
	char * buffer = ogg_sync_buffer(intoOss, 4096);
	fromFile.read(buffer, 4096);
	int bytesWritten = fromFile.gcount();
	ogg_sync_wrote(intoOss, bytesWritten);
	return bytesWritten;
}


/* dump the theora comment header */
static int dump_comments(th_comment *_tc){
  int   i;
  int   len;
  FILE *out;
  out = stderr;
  std::cout<<"\nEncoded by "<<_tc->vendor;
  if(_tc->comments){
    fprintf(out,"theora comment header:\n");
    for(i=0;i<_tc->comments;i++){
      if(_tc->user_comments[i]){
        len=_tc->comment_lengths[i]<INT_MAX?_tc->comment_lengths[i]:INT_MAX;
        fprintf(out,"\t%.*s\n",len,_tc->user_comments[i]);
      }
    }
  }
  return 0;
}

/// Attempts to open target file. Returns false upon failure.
bool OggStream::Open(String path)
{
	lastAudioInfo = "OggStream::Open: "+path;
	std::cout<<lastAudioInfo;

	// Save path we're streaming from.
	this->path = path;

	/// If already got another file open, close it.
	if (file.is_open()){
		std::cout<<"\nAlready got another file open! Closing it.";
		file.close();
	}

	file.open(path.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!file.is_open()){
		std::cout<<"\nUnable to open file.";
		return false;
	}

	name = FilePath::GetFileName(path);

	bool openedVideo = false;
#ifdef THEORA
	bool openedVideo = OpenTheora();
#endif

	bool opened = OpenVorbis();
	if (!opened)
		opened = OpenOpus();
	
	return opened | openedVideo;
}

/// Attempts to open Vorbis playback from the file-stream.
bool OggStream::OpenVorbis()
{
	lastAudioInfo = "OggStream::OpenVorbis";
	/// Grab any available OggVorbis data!
	bool result = ov_fopen(path.c_str(), &oggVorbisFile);
	if (result == 0){
		// Success!
		hasAudio = true;
		hasVorbis = true;
		/// Grab info and comment.
		vorbisInfo = ov_info(&oggVorbisFile, -1);
		vorbisComment = ov_comment(&oggVorbisFile, -1);
		std::cout<<"\nFound Vorbis.";

		audioChannels =	vorbisInfo->channels;
		samplesPerSecond = vorbisInfo->rate;
	//	std::cout<<"\nBitrate info: nominal: "<<vorbisInfo->bitrate_nominal<<" lower: "<<vorbisInfo->bitrate_lower;
//		assert(vorbisInfo->bitrate_nominal == vorbisInfo->bitrate_lower);

	}
	else 
	{
		// Require audio.
		lastAudioInfo = "OggStream::OpenVorbis - failed";
//		std::cout<<"\nUnable to find Vorbis.";
		return false;
	}
	lastAudioInfo = "OggStream::OpenVorbis - success";
	return true;
}

/// Attempts to open Opus playback from the file-stream.
bool OggStream::OpenOpus()
{
#ifdef OPUS
	lastAudioInfo = "OggStream::OpenOpus";
	/// Grab any available OggVorbis data!
	int error = 0;
	oggOpusFile = op_open_file(path.c_str(), &error);
	if (oggOpusFile)
	{
		// Success!
		hasAudio = true;
		hasOpus = true;
		/// Grab info and comment.
//		vorbisInfo = ov_info(&oggVorbisFile, -1);
	//	vorbisComment = ov_comment(&oggVorbisFile, -1);
		std::cout<<"\nFound Opus.";
		opusHead = op_head(oggOpusFile, -1);
		opusTags = op_tags(oggOpusFile, -1);
		std::cout<<"\nVendor: "<<opusTags->vendor;
		audioChannels = opusHead->channel_count;
		// Original sampling rate may be used, but for playback 48kHz should be used as that is standard for all Opus.
		samplesPerSecond = 48000; //opusHead->input_sample_rate
	}
	else 
	{
		// Require audio.
	//	std::cout<<"\nUnable to open audio! aborting.";
		return false;
	}
	return true;
#else
	return false;
#endif // OPUS
}

/// Attemps to open Theora playback from the file-stream.
bool OggStream::OpenTheora()
{
	lastAudioInfo = "OggStream::OpenTheora";
	/// Initialize Ogg synchronization state. Always returns 0.
	ogg_sync_init(&oggSyncState);
	oggSyncStateUsed = true;

	/// Initialize Theora structures needed.
#ifdef THEORA
	th_info_init(&theoraInfo);
	th_comment_init(&theoraComment);
#endif

	/// Temporary stream state used when parsing.
	ogg_stream_state tempStreamState;

	int result;

	// Read data until parse is done.
	bool nonHeaderPageEncountered = false;
	while(!nonHeaderPageEncountered)
	{
		/// Read in the initial page of the OGG, file, being the first 4 kb (4096 bytes)
		int read = ReadOggData(file, &oggSyncState);
		if (!read){
			std::cout<<"\nData read from file..";
			break;
		}
		// Parse data (pages) until raw-data pages are encountered.
		while(true)
		{
			/// Take sync state data and insert it into an ogg-page.
			result = ogg_sync_pageout(&oggSyncState, &oggPage);
			if (result == 0){
//				std::cout<<"\nMore data needed or internal error. Unable to insert ogg-page.";
				break;
			}
			else if (result == -1){
				std::cout<<"\nStream not captured yet. Wat";
				break;
			}
			/// Check if this is the initial page..?
			int isStartPage = ogg_page_bos(&oggPage);
			if (!isStartPage){
//				std::cout<<"\nNot start page. Storing it for later. wat";
				nonHeaderPageEncountered = true;
				// If we know we have Theora, store this page into that stream then.
				if(hasTheora)
					ogg_stream_pagein(&oggStreamState, &oggPage);
				break;
			}
			/// Is initial page, do stuff.
			/// Get serial number of the stream the page belongs to.
			int pageSerialNumber = ogg_page_serialno(&oggPage);

			// Allocate and initialize an ogg stream for the given serial number.
			result = ogg_stream_init(&tempStreamState, pageSerialNumber);
			if (result != 0){
				std::cout<<"\nUnable to initialize ogg stream with serial number D:";
				break;
			}
			// Add the current page to the given stream too!
			result = ogg_stream_pagein(&tempStreamState, &oggPage);
			// Peek at the packet to see its contents.
			// http://xiph.org/ogg/doc/libogg/ogg_stream_packetpeek.html
			result = ogg_stream_packetpeek(&tempStreamState, &oggPacket);
#ifdef THEORA
			// Check the codec of the stream. Is it theora? Should be called until it returns 0?
			// If we haven't already found a Theora packet, check that
			if (result == 1 && !hasTheora)
			{
				result = th_decode_headerin(&theoraInfo, &theoraComment, &theoraSetupInfo, &oggPacket);
				if (result >= 0){
					// Theora header found, so save away this current stream state into the ... wat
					// Re-look at this later, since we might need 2 ogg streams .. or wat? Dunno why they used is like this in the test code...
					memcpy(&oggStreamState, &tempStreamState, sizeof(tempStreamState));
					hasTheora = true;
					hasVideo = true;
					theoraPackets = 1;
					std::cout<<"\nTheora packet found.";

					/// Nullify setup info which should be a valid pointer by now?
					if (theoraSetupInfo)
						memset(theoraSetupInfo, 0, sizeof(theoraSetupInfo));

					/* Advance past the first processed header.*/
					if(result){
						ogg_stream_packetout(&oggStreamState, NULL);
					}
					else if (result == 0){
						// End of theora-decoding!
						/* whatever it is, we don't care about it */
					//	ogg_stream_clear(&test);
					}
				}
				/// Check if it was an error packet! Only one negative answer is returned if the packet type is non-Theora.
				else if (result == TH_ENOTFORMAT)
				{
					/// Error!
				//	std::cout<<"\nFound non-Theora packet.";
				}
				else {
					std::cout<<"\nError parsing theora packet";
				}
			}
			else {
				/// Packet we dont care about for now.
				ogg_stream_clear(&tempStreamState);
			}
#endif
			/// Check for vorbis packets?
			/// TODO: Use regular ov_open and stuff? Seems much easier than Theora o.O
		}
		// fall through to non-bos page parsing - whatever that means.
	}

	// Fully de-allocate the temp stream?
	if (tempStreamState.body_storage)
		ogg_stream_clear(&tempStreamState);

//	ogg_stream_clear(&tempStreamState);

#ifdef THEORA
	/// Alrighty. We seem to have initial packets and pages handled, now check for additional pages
	int processingTheoraHeaders = 1;
	while(hasTheora && processingTheoraHeaders)
	{
		/// Look through the encoder packets for Theora then.
		int packetPeekResult = 1;
		while (processingTheoraHeaders && (packetPeekResult = ogg_stream_packetpeek(&oggStreamState, &oggPacket))){
			// Look for more theora headers?
			if (packetPeekResult < 0)
				continue;
			processingTheoraHeaders = th_decode_headerin(&theoraInfo, &theoraComment, &theoraSetupInfo, &oggPacket);
			switch(processingTheoraHeaders){
				case TH_ENOTFORMAT: std::cout<<"\nNot a theora packet. Skipping."; continue; break;
				case TH_EVERSION: std::cout<<"\nPacket is Theora, but unsupported version!"; return false; break;
				case TH_EBADHEADER: std::cout<<"\nOgg packet was null, the packet was not the next packet in the expected sequence, or the format of the header data was invalid."; continue; break;
				case TH_EFAULT: std::cout<<"\nOne of the 3 primary arguments were NULL"; continue; break;
			}
			if (processingTheoraHeaders < 0){
				std::cout<<"\nError parsing Theora stream header!";
				return false;
			}
			else if (processingTheoraHeaders > 0)
			{
				// Advance past the successfully processed header.
				ogg_stream_packetout(&oggStreamState, NULL);
			}
			/// No more theora headers!
			theoraPackets++;
		}
		if (!(theoraPackets && processingTheoraHeaders))
			break;

		/// Get next page as needed?
		result = ogg_sync_pageout(&oggSyncState, &oggPage);
		if (result > 0){
			// demux into appropriate
			ogg_stream_pagein(&oggStreamState, &oggPage);
		}
		else {
			// Someone needs more data? wat
			result = ReadOggData(file, &oggSyncState);
			if (result == 0){
				std::cout<<"\nEnd of file while searching for codec headers! D:";
			}
		}
	}

	/// IF has theora
	if (hasTheora){
		// , stuff.
		dump_comments(&theoraComment);
		theoraDecoderContext = th_decode_alloc(&theoraInfo, theoraSetupInfo);
		std::cout<<"\nOgg logical stream "<<oggStreamState.serialno<<" is Theora "<<theoraInfo.frame_width<<"x"<<theoraInfo.frame_height<<" "<<theoraInfo.fps_numerator<<" fps video.";
		std::cout<<"\nEncoded frame content is "<<theoraInfo.pic_width<<"x"<<theoraInfo.pic_height<<" with "<<theoraInfo.pic_x<<"x"<<theoraInfo.pic_y<<" offset.";
		fpsNom = theoraInfo.fps_numerator;
		fpsDenom = theoraInfo.fps_denominator;
		fps = (String::ToString(fpsNom)+"."+String::ToString(fpsDenom)).ParseFloat();

		// Also allocate the frame-buffer! pic_ represents the inner picture, and frame_ the outer frame which has to be a multiple of 16.
		// http://theora.org/doc/libtheora-1.0/structth__info.html
		frameWidth = theoraInfo.pic_width;
		frameHeight = theoraInfo.pic_height;
		frameBufferSize = frameHeight * frameWidth * 4;
		rowBufferSize = frameWidth * 4;
		assert(frameBufferSize);
		/// Also create the frame texture to buffer data to!
		if (frameTexture == NULL){
			frameTexture = TexMan.New();
			frameTexture->bpp = 4;
			frameTexture->format = Texture::RGBA;
			frameTexture->width = frameWidth;
			frameTexture->height = frameHeight;
			frameTexture->dynamic = true; // Enable multiple bufferization.
			bool res = frameTexture->CreateDataBuffer();
			frameTexture->SetColor(Vector4f(0,0,0,1));
			assert(res);
		}
		rowBufferData = new unsigned char[rowBufferSize];
	}
	else{
	    /* tear down the partial theora setup */
	    th_info_clear(&theoraInfo);
	    th_comment_clear(&theoraComment);
	}

	/*Either way, we're done with the codec setup data.*/
	th_setup_free(theoraSetupInfo);
#endif

	// In the example they set up additional options for a callback function, namely decoder th_stripe_callback

	/// Print some more shit.
#ifdef USE_THEORA
	static const char *CHROMA_TYPES[4]={"420jpeg","Cpbarn","422","444"};
	if(theoraInfo.pixel_fmt >= 4 || theoraInfo.pixel_fmt == TH_PF_RSVD)
	{
		fprintf(stderr,"Unknown pixel format: %i\n", theoraInfo.pixel_fmt);
		return false;
	}
	if (hasTheora)
	{
		std::cout<<"\nYUV4MPEG2 C "<<CHROMA_TYPES[theoraInfo.pixel_fmt]<<" A"<<theoraInfo.aspect_numerator<<":"<<theoraInfo.aspect_denominator<<" H%d F%d:%d I%c A%d:%d\n";
		std::cout<<"\nTheora video should now be available for streaming to target texture object.";
	}
#endif
//	, theoraInfo.frame_width, theoraInfo.frame_height, theoraInfo.fps_numerator,theoraInfo.fps_denominator,'p',
//	theoraInfo.aspect_numerator,theoraInfo.aspect_denominator);

	/// Stuff.
	while(ogg_sync_pageout(&oggSyncState, &oggPage) > 0)
	{
		if(hasTheora)
			ogg_stream_pagein(&oggStreamState, &oggPage);
	}

	/// Set stream-state to ready if we have any data available for playback!
	if (hasTheora){
		streamState = StreamState::READY;
	}
	return hasTheora;
}


/// Closes target file and stream.
void OggStream::Close()
{
	/// Close the main Ogg/Theora file-stream too.
	if (file.is_open())
		file.close();
	// Close it too..
	ov_clear(&oggVorbisFile);
	// Clear sync state of any allocated data.
	if (oggSyncStateUsed)
	{
		ogg_sync_clear(&oggSyncState);
	}
	// Close Opus file stream.
	if (oggOpusFile)
	{
#ifdef OPUS
		op_free(oggOpusFile);
		oggOpusFile = NULL;
#endif
	}
	return;
}

/// Starts buffering of the stream. Nothing is done until this command has been executed successfully, following a call to Open.
bool OggStream::Play()
{
	return MultimediaStream::Play();
}
/// Pauses the stream.
bool OggStream::Pause()
{
	return MultimediaStream::Pause();
}
/// Seeks to target time in milliseconds.
bool OggStream::Seek(int toTime)
{
	/// Seek for audio!
	double toTimeDouble = toTime / 1000.f;
	if (hasOpus)
	{
#ifdef OPUS
		ogg_int64_t samplesIn48kHz = toTime * 48000;
		op_pcm_seek(oggOpusFile, samplesIn48kHz);
#endif
	}
	if (hasVorbis)
	{
		if (toTime == 0)
			toTime = 1;
		int result = ov_time_seek(&oggVorbisFile, toTimeDouble);
		if (result != 0){
			std::cout<<"some error";
		}
	}
	return true;
}

/** Streams the next frame. Does not perform any type of check that the frame is beyond current time, meaning the MultimediaManager has this responsibility!
	Frames to pass defines how many frames are to be evaluated. Default value is 1.
*/
bool OggStream::StreamNextFrame(int framesToPass /*= 1*/)
{
	if (streamState == StreamState::ENDED && !loop)
		return false;

	/// Always go at least 1 frame forward.
	if (framesToPass <= 0)
		framesToPass = 1;

	// Main decode loop.
	ogg_int64_t videoBufferGranulePosition = 0;
	int frames = 0;
	int videoBufferReady = false;

	/// Read data until video-buffer is ready, but also continue until the desired amount of frames has passed!
	while(!videoBufferReady || framesToPass > 0)
	{
#ifdef THEORA
		// Buffer video until we have a decent frame. If we can't read any more packets we break and fetch more data from the file-stream below.
		while(hasTheora && (!videoBufferReady || framesToPass))
		{
			// Get next ogg stream packet theora is one in, one out... */
			if(ogg_stream_packetout(&oggStreamState, &oggPacket) > 0)
			{
				// And try to decode it.
				if(th_decode_packetin(theoraDecoderContext, &oggPacket, &videoBufferGranulePosition) >= 0)
				{
					// One packet is 1 frame in Theora, so woo!
					double videobuf_time = th_granule_time(theoraDecoderContext, videoBufferGranulePosition);
					videoBufferReady = 1;

					frames++;
					/// Increment current frame!
					currentFrame++;
					--framesToPass;
					if (framesToPass > 1){
					//	std::cout<<"\nFrames to pass: "<<framesToPass;
					}
				}
			}
			else
				break;
		}
#endif

		/// If at end of file, stop streamin'
		bool atEndOfFile = false;
		if(!videoBufferReady && atEndOfFile)
			break;

		/// If video buffer not ready, buffer more data!
		if(!videoBufferReady || framesToPass)
		{
			/* no data yet for somebody.  Grab another page */
			int bytesRead = ReadOggData(file, &oggSyncState);
			while(ogg_sync_pageout(&oggSyncState, &oggPage) > 0)
			{
				ogg_stream_pagein(&oggStreamState, &oggPage);
			}
			// If at end of file, stahp.
			if (bytesRead == 0)
				break;
		}

	}

#ifdef THEORA
	/// If previous loop was successful, we have a frame ready.
	if (videoBufferReady)
	{
		// Woo!
		// dumpvideo frame!
		int h;

		// Uncomment the following to do normal, non-striped decoding.
		// th_ycbcr_buffer: http://theora.org/doc/libtheora-1.0/codec_8h.html
		th_ycbcr_buffer ycbcr;
		// Outputs the next available frame into the ycbcr
		th_decode_ycbcr_out(theoraDecoderContext, ycbcr);


		/// Ensure blue and red diff screen planes are exactly half the size of the ycbcr equivalents.
		assert(ycbcr[1].width == ycbcr[0].width/2);
		assert(ycbcr[2].width == ycbcr[0].width/2);
		assert(ycbcr[1].height == ycbcr[0].height/2);
		assert(ycbcr[2].height == ycbcr[0].height/2);

		/// For each row. http://www.theora.org/doc/libtheora-1.0/structth__img__plane.html#_details
		for(h = 0; h < ycbcr[0].height; h++)
		{
			/// Fetch rows of Y', Cb, and Cr! http://en.wikipedia.org/wiki/YCbCr
			unsigned char * lumaRow = ycbcr[0].data+ycbcr[0].stride * h;
			assert(ycbcr[1].stride == ycbcr[2].stride);
			int stride = ycbcr[1].stride * (h/2);
			unsigned char * blueDiffRow = ycbcr[1].data+stride;
			unsigned char * redDiffRow = ycbcr[2].data+stride;
			float luma;
			float blueDiff;
			float redDiff;
			/// Final r, g and b components, preferably in the range 0 to 255.
			float r, g, b;
			/// Generate pixels into the texture buffer for these!
			for (int w = 0; w < frameWidth; ++w){
				int lColumn = w, cColumn = w/2;
				luma = lumaRow[lColumn];
				blueDiff = blueDiffRow[cColumn];
				redDiff = redDiffRow[cColumn];
				int psi = h * frameWidth * 4 + w * 4;
				/// Row-buffer index!
				int rbi = w * 4;
				assert(rbi < rowBufferSize);

				/// Clear out the other parameters.
				bool redDiffOnly = false;
				bool blueDiffOnly = false;
				if (redDiffOnly){
					luma = 105.0f;
					blueDiff = 0;
				}
				else if (blueDiffOnly){
					luma = 105.0f;
					redDiff = 0;
				}


				// Greyscale
				bool greyScale = false;
				if (greyScale){
					rowBufferData[rbi] = rowBufferData[rbi+1] = rowBufferData[rbi+2] = (unsigned char)luma;
				}
				// RGBA in order
				else {
					/// Inverse transform
					int colorType = 2;
					if (colorType == 0){
						r = luma * 298.082f / 256.f + 408.583f * redDiff / 256.f - 222.291f;
						g = luma * 298.082f / 256.f - 100.291f * blueDiff / 256.f - 208.120f * redDiff / 256.f + 135.576f;
						b = luma * 298.082f / 256.f + 516.412f * blueDiff / 256.f - 276.836f;
					}
					// Inverse transform without any roundings as per ITU-R BT.601 recommendation.
					else if (colorType == 1){
						r = (luma-16) * 255.f / 219.f + 255.f * (redDiff - 128.f) / 112.f * 0.701f;
						g = (luma-16) * 255.f / 219.f - 255.f * (blueDiff - 128.f) / 112.f  * 0.886f  * 0.114f / 0.587f
							- 255.f * (redDiff - 128.f) / 112.f * 0.701f * 0.299f / 0.587f;
						b = (luma-16) * 255.f / 219.f + 255.f * (blueDiff - 128.f) / 112.f * 0.886f;
					}
					/// JPEG conversion.
					else if (colorType == 2){
						r = luma + 1.402f * (redDiff - 128.f);
						g = luma - 0.34414f * (blueDiff - 128.f) - 0.71414f * (redDiff - 128.0f);
						b = luma + 1.772f * (blueDiff - 128.f);
					}
				}

				ClampFloat(r, 0, 255);
				ClampFloat(g, 0, 255);
				ClampFloat(b, 0, 255);
				rowBufferData[rbi] = (unsigned char)r;
				rowBufferData[rbi+1] = (unsigned char)g;
				rowBufferData[rbi+2] = (unsigned char)b;
				rowBufferData[rbi+3] = 255;
			}
			/// Don't copy paste the size of the image data.
			int rowStartIndex = h * frameWidth * 4;
			if (rowStartIndex + rowBufferSize > frameTexture->dataBufferSize)
				continue;
			assert(rowBufferSize);
			/// Copy the row into the frame buffer.
			memcpy(frameTexture->data + rowStartIndex, rowBufferData, rowBufferSize);
		//	break;
		}
		frameTexture->queueRebufferization = true;
		frameTexture->lastUpdate = Timer::GetCurrentTimeMs();
		frameTexture->FlipY();
	}
	/// If we did not manage to generate any frames, it means that we're at the end of the file, so queue stopping!
	else {
		streamState = StreamState::ENDED;
	}
#endif
	return true;
}
/** Buffers audio data into buf, up to maximum of maxBytes. Returns amount of bytes buffered.
	Amount of bytes may reach 0 even if the media has not ended. Returns -1 on error. Returns -2 if the stream is paused.
	If loop is set to true it will try to automatically seek to the beginning when it reaches the end of the file.
*/
int OggStream::BufferAudio(char * buf, int maxBytes, bool loop)
{
	if (streamState == StreamState::PAUSED){
		return -2;
	}
	int bytes = 0;
	if (!hasAudio)
		return -1;
	int bytesToRead = maxBytes;
	int bitstream = 0;
	int bytesRead = 0;
	int bytesReadThisLoop = 0;
	int bytesPerSample = 2;
	bool seekDone = false;
	// Reset it to 0s?
	memset(buf, 0, maxBytes);
	// Max bytes typical 4096
	/// Read until we can read no more!
	while(true)
	{
		if (hasOpus)
		{
#ifdef OPUS
			// Since opus works in 16-bits (shorts), read the incoming buffer as one.
			int samplesToRead = bytesToRead * 0.5f;
		//	samplesToRead = 4;
			/// Break once we reach a threshold of what we can fit in the buffer.
			// http://www.opus-codec.org/docs/opusfile_api-0.6/group__stream__decoding.html#ga963c917749335e29bb2b698c1cb20a10
			assert(samplesToRead % audioChannels == 0);
			char * pointerLoc = buf + bytesRead;
			short * shortPointer = (short*) pointerLoc;
			int samplesRead = op_read(oggOpusFile, shortPointer, samplesToRead, NULL);  
			if (samplesRead == 0)
			{
				std::cout<<"\nSamples read 0. Assuming at end of stream.";
			}
			if (samplesRead % audioChannels != 0)
			{
				std::cout<<"\n Samples read not even compared to given audio channels. Assuming at end of stream.";
			}
			bytesReadThisLoop = samplesRead * bytesPerSample * audioChannels;
#endif // OPUS
		}
		else if (hasVorbis)
		{
			bytesReadThisLoop = ov_read(&oggVorbisFile, buf + bytesRead, bytesToRead, 0, 2, 1, &bitstream);
		}
		if (bytesReadThisLoop <= 0){
			/// Check if we might be at the end of the track.
			if (bytesToRead > 4096 && loop && !seekDone)
			{
				/// We're probably at the end if we get no data.
				/// So if loop is specified, seek to start and try again.
				Seek(0);
				seekDone = true;
				continue;
			}
			break;
		}
		bytesRead += bytesReadThisLoop;
		bytesToRead = maxBytes - bytesRead;
		// Break if we are approaching the end?
		if (bytesToRead < 4096)
			break;

		/// If looping, seek and retry
		/// ov_time_seek(&oggVorbisFile, 0);
	}
	// Read time.
	if (hasVorbis)
		audioTime = oggVorbisTime = ov_time_tell(&oggVorbisFile);
	else if (hasOpus)
	{
	//	OpusFileCallbacks callbacks;
	//	callbacks.tell;
//		int64 time = op_tell_func(oggOpusFile);
	//	audioTime = time;
	}
	return bytesRead;
}


/** Creates a texture pointer which will be updated as the stream progresses.
	If the stream lacks graphics NULL will be returned.
	If the texture has the rebufferize flag set it has to be re-buffered to gain new frame data.
*/
Texture * OggStream::GetFrameTexture()
{
	return frameTexture;
}

/*
/// Returns amount of channels present in the audio stream.
int OggStream::AudioChannels()
{
	if (oggOpusFile)
	{
		int channels = op_channel_count(oggOpusFile, 0);
		return channels;
	}
	assert(vorbisInfo);
	if (!vorbisInfo)
		return 0;
	return vorbisInfo->channels;
}
*/

/*
/// Gets frequency of the audio. This is typically 48000 or similar?
int OggStream::AudioFrequency()
{
	if (oggOpusFile)
	{
		return opusHead->input_sample_rate;
	}
	return vorbisInfo->rate;
}
*/

#endif // BUILD_OGG

