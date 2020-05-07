/// Emil Hedemalm
/// 2014-07-24
/// Class specifically made for logging messages in a at-the-end-of-file manner, making debugging possibly easier in some cases.
/// Developed to easier fix release-build issues, but may be good for end-users later on too, probably (same scenario though, release-builds failing).

#include "LogFile.h"

#include "FileUtil.h"
#include "Time/Time.h"
#include <fstream>

#include "Output.h"

List<TextError> gErrors, pErrors, aErrors, mErrors;

/// o=-o
List<String> logsStartedThisSession;

// yer.
bool loggingEnabled = true;
LogLevel logLevel = INFO;

/// Logs to file, creates the file (and folders) necessary if it does not already exist. Time stamps will probably also be available.
void LogToFile(String fileName,  String codeFile, String function, String logText, LogLevel levelFlags, List<TextError> * previousErrors /* = 0*/)
{
	Time time = Time::Now();
	String timeString = time.ToString("H:m:S ");
	String logTextWithFunction = codeFile +"::"+function+" "+logText;

	String textToStdOut = logTextWithFunction;
	String textToFile = timeString + logTextWithFunction + "\n";

	int level = levelFlags % 16;
	// What does this even do..?
	if (level < logLevel)
		return;
//	if (logLevel > DEBUG && level <= DEBUG)
	//	return;
	bool printed = Output(logText, previousErrors);
//	if (!printed)
//		return;
	if (level < logLevel)
		return;
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

	file.write((char*)textToFile.c_str(), textToFile.Length());
	file.close();

	bool causeAssertionError = levelFlags & CAUSE_ASSERTION_ERROR;
	if (causeAssertionError)
		assert(false && "LogFile-triggered assertion error. See relevant log file for details.");
}

void SetLogLevel(String fromString)
{
	if (fromString.Contains("FATAL"))
		logLevel = FATAL;
	if (fromString.Contains("INFO"))
		logLevel = INFO;
	if (fromString.Contains("WARNING"))
		logLevel = WARNING;
	if (fromString.Contains("ERROR"))
		logLevel = ERROR;
	if (fromString.Contains("DEBUG"))
		logLevel = DEBUG;
	if (fromString.Contains("EXTENSIVE_DEBUG"))
		logLevel = EXTENSIVE_DEBUG;
}
