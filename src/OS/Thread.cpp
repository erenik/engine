/// Emil Hedemalm
/// 2015-02-23
/// Contains macros for declaration and implementation of threads, no matter the OS.

#include "Message/MessageManager.h"

int errorCode;
String errorMessage;

/// Call if e.g. audio or graphics fail to set up. Be sure to Log the errors first! e.g. using LogFile
void QuitApplicationFatalError(const String & errMsg)
{
	errorMessage = errMsg;
	errorCode = -1;
	MesMan.QueueMessages("QuitApplication");
	// Set error message?
}
