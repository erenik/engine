// Emil Hedemalm
// 2013-08-09

#ifndef CHAT_MANAGER_H
#define CHAT_MANAGER_H

#include "ChatMessage.h"
#include "List/List.h"

#define ChatMan (*ChatManager::Instance())

class ChatManager {
    ChatManager();
    ~ChatManager();
    static ChatManager * chatManager;
public:
    static void Allocate();
    static void Deallocate();
    static ChatManager * Instance();

	/// Fetches last X messages.
	List<ChatMessage*> GetLastMessages(int amount);

    List<ChatMessage*> GetMessages();
    /// See ChatMessage.h for types.
    List<ChatMessage*> GetMessages(int byType);
    /// Returns messages created/received in a certain time interval. Passing a 0 to the final time will make it get all messages until current time.
    List<ChatMessage*> GetMessages(int fromTime, int toTime);
    /// Adds a message! 
    void AddMessage(ChatMessage * cm);
	/// Posts a general message for localhost only.
	void PostGeneral(String message);
    /// Deletes all messages.
    void ClearMessages();

    /// Otherwise every single message will be returned every time, or some other random number of messages. Setting 0 will make all return. (default)
    void SetMaxMessagesReturned(int maxMessages);

private:
    int maxMessagesReturned;
    List<ChatMessage*> messages;
};

#endif
