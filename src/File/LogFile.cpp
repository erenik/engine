/// Emil Hedemalm
/// 2014-07-24
/// Class specifically made for logging messages in a at-the-end-of-file manner, making debugging possibly easier in some cases.
/// Developed to easier fix release-build issues, but may be good for end-users later on too, probably (same scenario though, release-builds failing).

#include "LogFile.h"

#include "FileUtil.h"
#include "Time/Time.h"
#include <fstream>

/// o=-o
List<String> logsStartedThisSession;

// yer.
bool loggingEnabled = false;

/// Logs to file, creates the file (and folders) necessary if it does not already exist. Time stamps will probably also be available.
void LogToFile(String fileName, String logText)
{
	if (fileName == PHYSICS_THREAD ||
		fileName == MAIN_THREAD)
	{
		if (!loggingEnabled)
			return;
	}
	
	bool newLog = true;
	if (logsStartedThisSession.Exists(fileName))
	{
		newLog = false;
	}
	/// If a new session, overwrite the file with a new empty one too.
	if (!FileExists(fileName) || newLog)
	{
		CreateDirectoriesForPath(fileName, true);
		std::fstream newFile;
		newFile.open(fileName.c_str(), std::ios_base::out);
		newFile.close();
		logsStartedThisSession.Add(fileName);
	}

	// Save the text, first a time-stamp.
	std::fstream file;
	file.open(fileName.c_str(), std::ios_base::out | std::ios_base::app);
	if(!file.is_open())
		return;

	String text;
	Time time = Time::Now();
	String timeString = time.ToString("h:m:s ");
	text = timeString + logText + "\n";

	file.write((char*) text.c_str(), text.Length());
	file.close();
}