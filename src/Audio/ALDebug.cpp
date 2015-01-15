// Emil Hedemalm
// 2013-08-09

#include "AudioSettings.h"

#ifdef USE_AUDIO

#ifdef OPENAL

#include "ALDebug.h"
#include <AL/al.h>

void PrintALError(int alErrorCode, bool withNewline )
{
	if (withNewline)
		std::cout<<"\n";
	switch(alErrorCode){
		case AL_INVALID_NAME:
			std::cout<<"Invalid name";
			break;
		case AL_INVALID_ENUM:
			std::cout<<"Invalid enum";
			break;
		case AL_INVALID_VALUE:
			std::cout<<"Invalid value";
			break;
		case AL_INVALID_OPERATION:
			std::cout<<"Invalid operation";
			break;
		case AL_OUT_OF_MEMORY:
			std::cout<<"Out of memory";
			break;
	}
//	assert(false && "Error in AudioStream!");
}

void PrintALError(int alErrorCode, String errorMessage)
{
    if(alErrorCode != AL_NO_ERROR)
	{
		std::cout<<"\nAL Error: "<<errorMessage<<": ";
		PrintALError(alErrorCode, false);
	}
}


void CheckALError(String errorMessage)
{
	int error = alGetError();
	PrintALError(error, errorMessage);
}

#endif // OPENAL

#endif /// USE_AUDIO
