// Emil Hedemalm
// 2014-02-14
// A class to hold all kinds of messages

#ifndef MESSAGES_H
#define MESSAGES_H

//#include "UIElement.h"
#include "Globals.h"
#include "MessageTypes.h"
#include <Util.h>

class Script;
class UIElement;

class Message {
	friend class MessageManager;
public:
	Message(int type);
	Message(const String & msg);
	virtual ~Message();
	
	String msg;
	int type;
	char * data;
	/// Script that spawned this message! Can be nice.
	Script * event;
	/// Triggered element. for (this)-applications.
	UIElement * element;
	/// Time to process this message. If 0 process straight away (default).
	long long timeToProcess;
};

class PasteMessage : public Message
{
public:
	PasteMessage();
	String text;
};

class SetStringMessage : public Message {
public:
	SetStringMessage(String message, String value);
	virtual ~SetStringMessage();
	String value;
};

class TextureMessage : public Message {
public:
	TextureMessage(String message, String textureSource);
	virtual ~TextureMessage();
	String texSource;
};

/// Send by the UIMatrix class when elements inside have been adjusted.
class DataMessage : public Message {
public:
	DataMessage();
	virtual ~DataMessage();
	List<bool> binaryData;
};

struct ConnectionErrorMessage : public Message{
	char text[1024];
	ConnectionErrorMessage(const char i_text[]);
};

/*void Message::Process(){

}*/

#endif