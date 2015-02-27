/// Emil Hedemalm
/// 2015-02-27
/// Raw audio stream dedicated to provided swift playback of SFX.

#ifndef PCM_STREAM_H
#define PCM_STREAM_H

#include "List/List.h"
#include "Multimedia/MultimediaStream.h"

class PCMStream : public MultimediaStream 
{
public:
	static void PreloadDirectory(String dir);
	static bool Exists(String forPath);
	static PCMStream * New(String forPath);
private:

	bool Preload();

	/// Reference to preloaded, whose data we will be using.
	PCMStream * preloadStream;
	/// Used by the preloaded streams.
	char * data;
	/// o.o
	static List<PCMStream*> preloadedStreams;
};

#endif