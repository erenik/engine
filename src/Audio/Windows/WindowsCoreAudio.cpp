/// Emil Hedemalm
/// 2015-02-24
/// Manager for getting audio to work on Windows when OpenAL fails (as it seems to do for me...)

#include "WindowsCoreAudio.h"

#ifdef WINDOWS

#include "File/LogFile.h"
#include <Functiondiscoverykeys_devpkey.h>
// #include <Audiopolicy.h>
// #include <Devicetopology.h>
//#include <Endpointvolume.h>

#include <initguid.h>
#include <cmath>
#include "MathLib/AEMath.h"

List<WMMDevice*> WMMDevice::devices;
bool WMMDevice::initialized = false;

IMMDevice *				WMMDevice::endpoint;
IPropertyStore *		WMMDevice::properties;
LPWSTR					WMMDevice::pwszID;
IMMDeviceEnumerator *	WMMDevice::enumerator;
IMMDeviceCollection *	WMMDevice::collection;


// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

#define CANCEL_ON_ERROR	if (hr != S_OK) return false;
#define SAFE_RELEASE(punk)  \
          if ((punk) != NULL)  \
            { (punk)->Release(); (punk) = NULL; }


WMMDevice::WMMDevice()
{
	device = NULL;	
	playing = false;
	audioClient = NULL;
	renderClient = NULL;
	pwfx = NULL;
	// Default buffer of 1 second is a bit long, causing volume changes to appear laggy, better let AudioMixer have bigger size.
	// 50 ms buffer size should provide more than enough leeway (20 updates per second?).
	bufferFrameCount = REFTIMES_PER_SEC * 0.05; 
	waveFormat = WaveFormat::BAD_TYPE;
}

WMMDevice::~WMMDevice()
{
	// Free resources used by this specific device.
    HRESULT hr = audioClient->Stop();  // Stop playing.
	assert(hr == S_OK);
}



bool WMMDevice::Initialize()
{
	pwszID = NULL;
	const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	enumerator = NULL;
 
	HRESULT hr;
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	CANCEL_ON_ERROR
	
	hr = CoCreateInstance(
		CLSID_MMDeviceEnumerator, NULL,
		CLSCTX_ALL, IID_IMMDeviceEnumerator,
		(void**)&enumerator);
	CANCEL_ON_ERROR
	// Stuff.

	// https://msdn.microsoft.com/en-us/library/windows/desktop/dd371400%28v=vs.85%29.aspx
	// eRender, eCapture or eAll
	hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection);
	CANCEL_ON_ERROR
	UINT count;
	hr = collection->GetCount(&count);
	CANCEL_ON_ERROR
	if (count == 0)
		LogAudio("No audio endpoints found", INFO);

	   // Each loop prints the name of an endpoint device.
    for (ULONG i = 0; i < count; i++)
    {
        // Get pointer to endpoint number i.
        hr = collection->Item(i, &endpoint);
        CANCEL_ON_ERROR

        // Get the endpoint ID string.
        hr = endpoint->GetId(&pwszID);
        CANCEL_ON_ERROR
        
        hr = endpoint->OpenPropertyStore(
                          STGM_READ, &properties);
        CANCEL_ON_ERROR

        PROPVARIANT varName;
        // Initialize container for property value.
        PropVariantInit(&varName);

        // Get the endpoint's friendly-name property.
        hr = properties->GetValue(
                       PKEY_Device_FriendlyName, &varName);
        CANCEL_ON_ERROR

		WMMDevice * newDevice = new WMMDevice();
		newDevice->device = endpoint;
		newDevice->name = varName.pwszVal;

		// Set up for output?
		bool ok = newDevice->SetupForOutput();
		if (ok)
			devices.AddItem(newDevice);
		else 
			delete newDevice;


        // Print endpoint friendly name and endpoint ID.
        printf("Endpoint %d: \"%S\" (%S)\n",
               i, varName.pwszVal, pwszID);

        CoTaskMemFree(pwszID);
        pwszID = NULL;
        PropVariantClear(&varName);
        SAFE_RELEASE(properties)
        SAFE_RELEASE(endpoint)
    }
	
	SAFE_RELEASE(enumerator);
	SAFE_RELEASE(collection);
	// Do stuff.
	initialized = true;
	return true;
}


bool WMMDevice::SetupForOutput()
{
	// Set request duration to what was specified in the constructor.
	REFERENCE_TIME hnsRequestedDuration = bufferFrameCount;
	audioClient = NULL;
    DWORD flags = 0;

	HRESULT hr = device->Activate(
					__uuidof(IAudioClient), CLSCTX_ALL,
					NULL, (void**)&audioClient);
	CANCEL_ON_ERROR;

	hr = audioClient->GetMixFormat(&pwfx);
	assert(hr == S_OK);
	CANCEL_ON_ERROR;

	hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL);
	while(hr != S_OK)
	{
		std::cout<<"\nWas unable to initialize audio cient with request duration, trying again.";
		Sleep(5);
		hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL);
	}
	assert(hr == S_OK);
	CANCEL_ON_ERROR;

   // Tell the audio source which format to use. o.O
	std::cout<<pwfx->nSamplesPerSec;
	if (pwfx->wFormatTag  == WAVE_FORMAT_EXTENSIBLE)
	{
		WAVEFORMATEXTENSIBLE * extensible = (WAVEFORMATEXTENSIBLE*) pwfx;
		std::cout<<"\nExtensible WAVE format"; //<<extensible->SubFormat;
		if (extensible->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
		{
			 // o.o
			std::cout<<"\nKSDATAFORMAT_SUBTYPE_IEEE_FLOAT";
			waveFormat = WaveFormat::IEEE_FLOAT;
		}
	}
	// pMySource refers to some custom class... right.
//	hr = pMySource->SetFormat(pwfx);
	assert(hr == S_OK);
    CANCEL_ON_ERROR;

    // Get the actual size of the allocated buffer.
    hr = audioClient->GetBufferSize(&bufferFrameCount);
	assert(hr == S_OK);
    CANCEL_ON_ERROR;

    hr = audioClient->GetService(
                         __uuidof(IAudioRenderClient),
                         (void**)&renderClient);
	assert(hr == S_OK);
    CANCEL_ON_ERROR;

    // Grab the entire buffer for the initial fill operation.
    hr = renderClient->GetBuffer(bufferFrameCount, &buffer);
	assert(hr == S_OK);
    CANCEL_ON_ERROR;

	// Test output
	/*
	for (int i = 0; i < 10000; ++i)
	{
		BufferData(NULL, 1000000);
		Sleep(500);
	}
*/
	return true;
}

/// Queries how many bytes are bufferable. This should then be used as the maxData param for the next call to BufferData.
int WMMDevice::BytesToBuffer()
{
	// See how much buffer space is available.
    HRESULT hr = audioClient->GetCurrentPadding(&numFramesPadding);
	assert(hr == S_OK);
    CANCEL_ON_ERROR;

    numFramesAvailable = bufferFrameCount - numFramesPadding;
	int samplesAvailable = numFramesAvailable * pwfx->nChannels;
	int bytesAvailable = samplesAvailable * pwfx->wBitsPerSample / 8;
	return bytesAvailable;
}

int WMMDevice::SamplesToBuffer()
{
	// See how much buffer space is available.
    HRESULT hr = audioClient->GetCurrentPadding(&numFramesPadding);
	assert(hr == S_OK);
    CANCEL_ON_ERROR;

    numFramesAvailable = bufferFrameCount - numFramesPadding;
	int samplesAvailable = numFramesAvailable * pwfx->nChannels;
	return samplesAvailable;	
}

/// Buffers data. Returns amount of bytes actually buffered, or -1 on error.
int WMMDevice::BufferData(char * data, int maxBytes)
{
	HRESULT hr;
	/// First buffer! o.o
	if (!playing)
	{
		/// Use min amount of bytes, based on supplied amount (arg) and the buffer frame count at last poll to the API/driver-thingy.
		int bufferByteCount = bufferFrameCount * this->pwfx->nChannels * this->pwfx->wBitsPerSample / 8;
		int bytesToBuffer = min(maxBytes, bufferByteCount);
		// Load the initial data into the shared buffer.
		// Add some sine-data
		if (data == NULL)
		{
			float * fBuf = (float*) buffer;
			for (int i = 0; i < bytesToBuffer / 4; ++i)
			{
				fBuf[i] = sin(i * 0.01f);
			}
		}
		else 
		{
			memcpy(buffer, data, bytesToBuffer);
		}

		// https://msdn.microsoft.com/en-us/library/windows/desktop/dd368244%28v=vs.85%29.aspx
		hr = renderClient->ReleaseBuffer(bufferFrameCount, 0);
		assert(hr == S_OK);
		CANCEL_ON_ERROR;

		// Calculate the actual duration of the allocated buffer.
		hnsActualDuration = (double)REFTIMES_PER_SEC *
							bufferFrameCount / pwfx->nSamplesPerSec;

		hr = audioClient->Start();  // Start playing.
		assert(hr == S_OK);
		CANCEL_ON_ERROR;
		playing = true;
//		int bytesWritten = numFramesWritten * BytesPerFrame();
		return bytesToBuffer;
	}
	// See how much buffer space is available.
    hr = audioClient->GetCurrentPadding(&numFramesPadding);
	assert(hr == S_OK);
    CANCEL_ON_ERROR;

    numFramesAvailable = bufferFrameCount - numFramesPadding;
	int framesNeeded = maxBytes / (pwfx->wBitsPerSample / 8) / 2;
	int framesToBuffer = min(framesNeeded, numFramesAvailable);

//	hr = audioClient->Start();  // Start playing.

    // Grab all the available space in the shared buffer.
    hr = renderClient->GetBuffer(framesToBuffer, &buffer);
	assert(hr == S_OK);
	CANCEL_ON_ERROR;

    // Get next 1/2-second of data from the audio source.
	/*
	for (int i = 0; i < bytesToBuffer; ++i)
	{
		buffer[i] = sin(i * 0.01f) * 255;
	}
*/

	// The size in bytes of an audio frame equals the number of channels in the stream multiplied by the sample size per channel. 
	// For example, the frame size is four bytes for a stereo (2-channel) stream with 16-bit samples.
	// https://msdn.microsoft.com/en-us/library/windows/desktop/dd370868%28v=vs.85%29.aspx
//	int samplesAvailable = numFramesAvailable * pwfx->nChannels;
//	int bytesAvailable = samplesAvailable * pwfx->wBitsPerSample / 8;

	// Grab based on supplied data and available frames.
	int bytesToBuffer = framesToBuffer * pwfx->wBitsPerSample / 8 * 2;

	
	if (data == NULL)
	{
		float * fBuf = (float*) buffer;
		static float angle = 0;
		for (int i = 0; i < bytesToBuffer / 4; ++i)
		{
			angle += 0.025f;
			if (angle > TwoPI)
				angle -= TwoPI;
			fBuf[i] = sin(angle) + sin(angle * 1.5f);
		}
	}
	else 
	{
		// Fill earlier part of buffer with 0s?
//		memset(buffer, 0, bufferFrameCount * 4);
		memcpy(buffer, data, bytesToBuffer);
	}

	// Replace with a memcpy
	int numFramesWritten = framesToBuffer;
	hr = renderClient->ReleaseBuffer(numFramesWritten, 0);
	assert(hr == S_OK);
	CANCEL_ON_ERROR;
	int bytesWritten = numFramesWritten * BytesPerFrame();
	return bytesWritten;
}

int WMMDevice::BytesPerFrame()
{
	return this->pwfx->nChannels * this->pwfx->wBitsPerSample / 8;
}



bool WMMDevice::Deallocate()
{
	devices.ClearAndDelete();

	CoTaskMemFree(pwszID);
	SAFE_RELEASE(enumerator);
	SAFE_RELEASE(collection);
    SAFE_RELEASE(properties);
    SAFE_RELEASE(endpoint);
	return true;
}

WMMDevice * WMMDevice::MainOutput()
{
	if (devices.Size())
		return devices[0];
	return NULL;
}


#endif // WINDOWS