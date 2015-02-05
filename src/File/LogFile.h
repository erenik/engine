/// Emil Hedemalm
/// 2014-07-24
/// Class specifically made for logging messages in a at-the-end-of-file manner, making debugging possibly easier in some cases.
/// Developed to easier fix release-build issues, but may be good for end-users later on too, probably (same scenario though, release-builds failing).

#ifndef LOG_FILE_H
#define LOG_FILE_H

#include "String/AEString.h"

#include "ThreadSetup.h"

#define LogPhysics(s,i) LogToFile(PHYSICS_THREAD, s, i)
#define LogMain(s,i) LogToFile(MAIN_THREAD, s, i)
#define LogAudio(s,i) LogToFile(AUDIO_THREAD, s,i)
#define LogGraphics(s,i) LogToFile(GRAPHICS_THREAD, s, i)

#ifdef ERROR
#undef ERROR
#endif
enum 
{
	INFO,
	WARNING,
	ERROR
};

/// Logs to file, creates the file (and folders) necessary if it does not already exist. Time stamps will probably also be available.
void LogToFile(String fileName, String text, int level = INFO);


#endif
