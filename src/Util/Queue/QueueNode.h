
#ifndef QueueNode_H
#define QueueNode_H

#include <cstdlib>

template <class Item>
class QueueNode
{
	typedef Item value_type;
	typedef size_t size_type;
public:
	QueueNode(value_type i_item);
	value_type getData();
	QueueNode<value_type>* getNext();
	void setNext(QueueNode<value_type>* i_next);

private:
	value_type data;
	QueueNode<value_type>* next;
};

#include "QueueNode.template"

#endif