// Emil Hedemalm
// 2013-08-09

#include "ChatManager.h"
#include "StateManager.h"

ChatManager * ChatManager::chatManager = NULL;

ChatManager::ChatManager(){
    maxMessagesReturned = 0;
}
ChatManager::~ChatManager(){
    messages.ClearAndDelete();
}

void ChatManager::Allocate(){
    assert(chatManager == NULL);
    chatManager = new ChatManager();
}
void ChatManager::Deallocate(){
    assert(chatManager);
    delete chatManager;
    chatManager = NULL;
}
ChatManager * ChatManager::Instance(){
    assert(chatManager);
    return chatManager;
}

/// Fetches last X messages.
List<ChatMessage*> ChatManager::GetLastMessages(int amount){
	List<ChatMessage*> m;
	int firstIndex = messages.Size() - amount;
	if (firstIndex < 0)
		firstIndex = 0;
	for (int i = firstIndex; i < messages.Size(); ++i)
	{
		m.Add(messages[i]);	
	}
	return m;
}


List<ChatMessage*> ChatManager::GetMessages() {
    List<ChatMessage*> m;
    if (maxMessagesReturned <= 0)
        return messages;
    else {
        assert(false && "Implement");
    };
};
/// See ChatMessage.h for types.
List<ChatMessage*> ChatManager::GetMessages(int byType)
{
    List<ChatMessage*> m;
    for (int i = 0; i < messages.Size(); ++i){
        ChatMessage * cm = messages[i];
        if (cm->type == byType)
            m.Add(cm);
    }
    return m;
}
/// Returns messages created/received in a certain time interval. Passing a 0 to the final time will make it get all messages until current time.
List<ChatMessage*> ChatManager::GetMessages(int fromTime, int toTime/* = 0*/){
    assert(false && "Implement");
	List<ChatMessage*> lcm;

	return lcm;
}
/// Wosh. o-o
void ChatManager::AddMessage(ChatMessage * cm){
    messages.Add(cm);

    if (StateMan.GlobalState())
        StateMan.GlobalState()->OnChatMessageReceived(cm);
    if (StateMan.ActiveState())
        StateMan.ActiveState()->OnChatMessageReceived(cm);
}

/// Posts a general message for localhost only.
void ChatManager::PostGeneral(String message){
	ChatMessage * cm = new ChatMessage(NULL, message);
	AddMessage(cm);
}

/// Deletes all messages.
void ChatManager::ClearMessages(){
    messages.ClearAndDelete();
}

/// Otherwise every single message will be returned every time, or some other random number of messages.
void ChatManager::SetMaxMessagesReturned(int maxMessages){
    assert(maxMessages >= 0);
    maxMessagesReturned = maxMessages;
}
