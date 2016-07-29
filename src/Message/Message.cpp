// Emil Hedemalm
// 2013-06-15

#include "Message/Message.h"

/*
Message::Message(){
	type = Message::NULL_TYPE;
}
*/
Message::Message(const String & i_string)
{
	Nullify();
	msg = i_string;
	type = MessageType::STRING;
}

Message::Message(int i_type)
{
	Nullify();
	type = i_type;
};

void Message::Nullify()
{
	data = NULL;
	recipientEntity = 0;
}

Message::~Message(){
	/// TODO: Evaluate whether data parameter should be deleted or not, probably not.
   // 	assert(data == NULL && "Data should have been deallocated earlier! WTF are you doing developer?!");
}


MouseMessage::MouseMessage(int interaction, AppWindow * window, Vector2i coords)
	: Message(MessageType::MOUSE_MESSAGE), interaction(interaction), window(window), coords(coords)
{

}

PasteMessage::PasteMessage()
	: Message(MessageType::PASTE)
{

}

/// Text drag-n-drop.
DragAndDropMessage::DragAndDropMessage(Vector2i pos, String text)
	: Message(MessageType::DRAG_AND_DROP), position(pos), string(text)
{
	dataType = DataType::STRING;
	processed = false;
}


SetStringMessage::SetStringMessage(String message, String value)
: Message(MessageType::SET_STRING)
{
	msg = message;
	this->value = value;
}
SetStringMessage::~SetStringMessage()
{

}

BoolMessage::BoolMessage(String message, bool value)
	: Message(MessageType::BOOL_MESSAGE), value (value)
{
	msg = message;
}
BoolMessage::~BoolMessage()
{

}

TextureMessage::TextureMessage(String message, String textureSource)
: Message(MessageType::TEXTURE_MESSAGE)
{
	texSource = textureSource;
	msg = message;
}
TextureMessage::~TextureMessage()
{
}

DataMessage::DataMessage()
: Message(MessageType::DATA_MESSAGE)
{

}
DataMessage::~DataMessage(){

}
