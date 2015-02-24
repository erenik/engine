/// Emil Hedemalm
/// 2015-02-24
/// Manager for getting audio to work on Windows when OpenAL fails (as it seems to do for me...)
/**
	Overview: https://msdn.microsoft.com/en-us/library/windows/desktop/dd370784%28v=vs.85%29.aspx

	Related APIS: 
	- MMDevice API: https://msdn.microsoft.com/en-us/library/windows/desktop/dd316556%28v=vs.85%29.aspx
	- WASAPI: https://msdn.microsoft.com/en-us/library/windows/desktop/dd371455%28v=vs.85%29.aspx

*/

#ifndef WINDOWS_CORE_AUDIO_H
#define WINDOWS_CORE_AUDIO_H

#include "OS/OS.h"

#ifdef WINDOWS

#include "String/AEString.h"
#include "List/List.h"

#include <Mmdeviceapi.h>
#include <Audioclient.h>
#include <Ks.h>
#include <KsMedia.h>

namespace WaveFormat {
	enum 
	{
		BAD_TYPE,
		IEEE_FLOAT
	};
};

/// Windows Multimedia device
class WMMDevice 
{
public:
	WMMDevice();
	~WMMDevice();

	/// Finds out devices for us.
	static bool Initialize();
	static bool Deallocate();	
	static WMMDevice * MainOutput();

	/// Sets the device up for output.
	bool SetupForOutput();
	/// Queries how many bytes are bufferable. This should then be used as the maxData param for the next call to BufferData.
	int BytesToBuffer();
	/// Sample is sample. Multiply by bits per sample and you should get bytes to buffer.
	int SamplesToBuffer();
	/// Buffers data. Returns amount of bytes actually buffered, or -1 on error.
	int BufferData(char * data, int maxData);

private:
	// ID and handle.
	String name;
	IMMDevice * device;
	/// Buffer used for the PCM data.
	int waveFormat;
	BYTE * buffer;
	bool playing;
	REFERENCE_TIME hnsActualDuration;
	IAudioClient * audioClient;
	IAudioRenderClient * renderClient;
	UINT32 bufferFrameCount;
	WAVEFORMATEX * pwfx;

	/// For buffering.
	UINT32 numFramesAvailable;
	UINT32 numFramesPadding;

	/// Shared statics for handling it all.
	static List<WMMDevice*> devices;
	static bool initialized;

	static IMMDevice * endpoint;
	static IPropertyStore * properties;
	static LPWSTR pwszID;
	static IMMDeviceEnumerator * enumerator;
	static IMMDeviceCollection * collection;

};


#endif // WINDOWS
#endif // WINDOWS_CORE_AUDIO_H
