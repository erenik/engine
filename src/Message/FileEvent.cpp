/// Emil Hedemalm
/// 2014-01-18
/// Message type that is based on a string for what action to perform, but includes a list of 1 or more files or directories to act upon too.

#include "FileEvent.h"

FileEvent::FileEvent()
: Message(MessageType::FILE_EVENT)
{
	//
}