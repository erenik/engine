/// Emil Hedemalm
/// 2014-09-08
/// A variant of a list, displaying/filtering messages, rendering each message in a specific color (if wanted)

#ifndef UI_LOG_H
#define UI_LOG_H

#include "UILists.h"

class LogMessage 
{
public:
	String msg;
	int category;
};


class UILog : public UIList 
{
public:
	UILog();
	/// For simple logging of just text
	void Append(CTextr text);
	/// Sets all messages to filter/display.
	void SetLogMessages(List<LogMessage> messages);

private:
	List<LogMessage> logMessages;
};

#endif

