// Emil Hedemalm
// 2014-02-14
// A class to hold all kinds of messages

#ifndef MESSAGES_H
#define MESSAGES_H

//#include "UIElement.h"
#include "Globals.h"
#include "MessageTypes.h"
#include <Util.h>
#include "DataTypes.h"
#include "MathLib.h"
#include "Time/Time.h"
#include "Entity/Entity.h"

class Script;
class UIElement;

class Message {
	friend class MessageManager;
public:
	Message(int type);
	Message(const String & msg);
	void Nullify();
	virtual ~Message();
	
	String msg;
	int type;
	char * data;
	/// Script that spawned this message! Can be nice.
	Script * scriptOrigin;
	/// Triggered element. for (this)-applications.
	UIElement * element;
	/// Time when this message should be processed.
	AETime timeToProcess;

	/// If this is non-null, the message is sent only to this entity, instead of being processed by the general application states.
	EntitySharedPtr recipientEntity;
};

class AppWindow;
/// Sent from graphics-manager after having being processed in the UI there.
class MouseMessage : public Message 
{
public:
	MouseMessage(int interaction, AppWindow * window, Vector2i coords);
	enum interactions 
	{
		MOVE,
		LDOWN,
		RDOWN,
		LUP,
		RUP,
		SCROLL
	};
	AppWindow * window;
	Vector2i coords;
	int interaction;
	float scrollDistance;
};

class PasteMessage : public Message
{
public:
	PasteMessage();
	String text;
};

class DragAndDropMessage : public Message 
{
public:
	/// Text drag-n-drop.
	DragAndDropMessage(Vector2i pos, String text);
	// Add more constructors later..
	
	/// Data type contained in the message. See DataType.h for types.
	int dataType;
	String string;
	/// Mouse/cursor position when drop is commenced.
	Vector2i position;

	/// If a UI has been processed by this message, this flag will be set to true.
	bool processed;
};

class SetStringMessage : public Message {
public:
	SetStringMessage(String message, String value);
	virtual ~SetStringMessage();
	String value;
};

class BoolMessage : public Message 
{
public:
	BoolMessage(String message, bool value);
	virtual ~BoolMessage();
	bool value;
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


#define CURLMessage HttpMessage 

class HttpMessage : public Message 
{
public:
	HttpMessage(String msg, int responseCode)
		: Message(MessageType::CURL), responseCode(responseCode)
	{
		this->msg = msg;
	}
	int responseCode;
	String contents;
	String url;
}; 

/** SIP packet contents wrapped into a Message class here for eased processing across the engine.
	Unless specified otherwise, only SIP INFO messages may be transmitted like this.
*/
class SIPMessage : public Message 
{
public:
	SIPMessage(String msg)
		: Message(MessageType::SIP)
	{
		this->msg = msg;
	}
};

class OnUIPushed : public Message
{
public:
	OnUIPushed(String uiName) 
		: Message(MessageType::ON_UI_PUSHED) {
		this->msg = uiName;
	}
};

/*void Message::Process(){

}*/

#endif