/// Emil Hedemalm
/// 2014-07-24
/// Class specifically made for logging messages in a at-the-end-of-file manner, making debugging possibly easier in some cases.
/// Developed to easier fix release-build issues, but may be good for end-users later on too, probably (same scenario though, release-builds failing).

#ifndef LOG_FILE_H
#define LOG_FILE_H

#include "String/AEString.h"

// Some standard files used for logging.
#define PHYSICS_THREAD "log/PhysicsLog.txt"
#define MAIN_THREAD	"log/MainThreadLog.txt"

#define LogGraphics(s) LogToFile(PHYSICS_THREAD, s)
#define LogPhysics(s) LogToFile(PHYSICS_THREAD, s)
#define Log(s) LogToFile(MAIN_THREAD, s)

/// Logs to file, creates the file (and folders) necessary if it does not already exist. Time stamps will probably also be available.
void LogToFile(String fileName, String text);


#endif