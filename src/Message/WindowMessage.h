/// Emil Hedemalm
/// 2015-04-20
/// Sent after window-systems receive updates.

#ifndef WINDOW_MESSAGE_H
#define WINDOW_MESSAGE_H

#include "Message.h"
#include "MessageTypes.h"

namespace WMes {
	enum 
	{
		SIZE_UPDATED,
	};
};

class WindowMessage : public Message 
{
public:
	WindowMessage(int wmes)
		: Message(MessageType::WINDOW_MESSAGE), wmes(wmes)
	{
	};
	/// Contains the window-message code, see namespace above.
	int wmes;
	Vector2i size;
private:
};

#endif

