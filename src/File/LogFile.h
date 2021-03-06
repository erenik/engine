/// Emil Hedemalm
/// 2014-07-24
/// Class specifically made for logging messages in a at-the-end-of-file manner, making debugging possibly easier in some cases.
/// Developed to easier fix release-build issues, but may be good for end-users later on too, probably (same scenario though, release-builds failing).

#ifndef LOG_FILE_H
#define LOG_FILE_H

#include "String/AEString.h"

#include "ThreadSetup.h"

#include "Output.h"
extern List<TextError> gErrors, pErrors, aErrors, mErrors;

#define CURRENT_FILE __FILE__

#define LogPhysics(text, level) LogToFile(PHYSICS_THREAD, CURRENT_FILE, __func__, text, level, &pErrors)
#define LogMain(text, level) LogToFile(MAIN_THREAD, CURRENT_FILE, __func__, text, level, &mErrors)
#define LogAudio(text, level) LogToFile(AUDIO_THREAD, CURRENT_FILE, __func__, text, level, &aErrors)
#define LogGraphics(text, level) LogToFile(GRAPHICS_THREAD, CURRENT_FILE, __func__, text, level, &gErrors)

#ifdef ERROR
#undef ERROR
#endif
enum LogLevel
{
	EXTENSIVE_DEBUG = 0,
	DEBUG = 1,
	INFO = 2,
	WARNING = 3,
	ERROR = 4,
	FATAL = 5,
	CAUSE_ASSERTION_ERROR = 16
};

/// Global log- and debugging level, adjustable at start-up.
extern LogLevel logLevel;

/// Logs to file, creates the file (and folders) necessary if it does not already exist. Time stamps will probably also be available.
void LogToFile(String fileName, String codeFile, String funcName, String text, LogLevel level, List<TextError> * previousErrors = 0);
void SetLogLevel(String fromString);


#endif
