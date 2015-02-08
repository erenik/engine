// Emil Hedemalm
// 2013-03-17
/// Message manager which takes care of glueing together most of the engine.

#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include "Queue/Queue.h"
#include "Mutex/Mutex.h"

#define MesMan		(*MessageManager::Instance())

class Message;
class Packet;
class Mutex;
class UIElement;

// A message manager to process input events and network packets
class MessageManager {
private:
	MessageManager();					// Constructor &
	static MessageManager * messageManager;
public:
	/// Singleton instance getter.
	static void Allocate();
	static MessageManager * Instance();
	static void Deallocate();
	~MessageManager();					// Destructor

	// Processes Network packets
	void ProcessPackets();

	/** Processes messages from the message queue
		Messages can be generated from anywhere in the program and added to the queue.
	*/
	void ProcessMessages();

	/// Processes string-based message.
	void ProcessMessage(String message);

	bool messageQueueIsOff();			// Check if the message queue is empty

	/// For them delayed messages that require special treatment.. :P
	bool QueueDelayedMessage(Message * message);

	/// Queues a bunch of string-based messages in the form "Message1&Message2&Message3&..."
	bool QueueMessages(const List<Message*> & messages, UIElement * elementThatTriggeredIt = NULL);
	bool QueueMessages(String messages, UIElement * elementThatTriggeredIt = NULL);
	bool QueueMessages(List<String> messages, UIElement * elementThatTriggeredIt = NULL);
	bool QueueMessage(Message* msg);	// Queues a message
	bool QueueMessage(int msg);			// Queues a simple numerical message
	bool QueuePacket(Packet* packet);	// Queue an incoming network packet for future processing
	/// Queues a list of packets for future processing by relevant game and entity states.
	bool QueuePackets(List<Packet*> packets);

private:

	void ProcessPacket(Packet * packet);
	void ProcessMessage(Message * message);

	Mutex msgQueueMutex;
	Mutex packetQueueMutex;
	Queue<Packet *> packetQueue;
	List<Message *> messageQueue;
	/// Queue with messages that are delayed to a certain time of delivery.
	List<Message *> delayedMessages;

};


#endif
