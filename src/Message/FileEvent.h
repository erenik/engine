/// Emil Hedemalm
/// 2014-01-18
/// Message type that is based on a string for what action to perform, but includes a list of 1 or more files or directories to act upon too.

#ifndef FILE_EVENT_H
#define FILE_EVENT_H

#include "Message/Message.h"

/** Message type that is based on a string for what action to perform, but includes a list of 1 or more files or directories to act upon too.
	The action to be taken with the files is stored in the msg parameter of the Message-class.
*/
class FileEvent : public Message {
public:
	FileEvent();

	/// List of files.
	List<String> files;
};

//FILE_EVENT,



#endif