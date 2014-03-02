/// Emil Hedemalm
/// 2013-03-01


#ifndef QUEUE_H
#define QUEUE_H

#include "QueueNode.h"
#include <cstdlib>



template <class Item>
class Queue
{
public:
	typedef Item value_type;
	
	Queue();
	~Queue();
	void Push(value_type i_item);
	void PushFront(value_type i_item);	// Push to the front of the queue
	value_type Pop();					// Pops first element
	value_type PopLast();				// Pops last element
	/// Peeks at the front of the queue.
	value_type Peek() const;
	int Length();
	bool isOff();
	/// Polls the existance/copy of target item in the queue
	bool Exists(value_type item);
private:
	QueueNode<value_type>* head_ptr;
	QueueNode<value_type>* tail_ptr;
	int length;
	bool inUse;		// Defines that the queue is currently being popped or pushed.
};

#include "Queue.template"

#endif