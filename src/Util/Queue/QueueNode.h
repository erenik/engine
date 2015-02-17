
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

template <class Item>
QueueNode<Item>::QueueNode(value_type i_item)
{
	data = i_item;
	next = NULL;
}

template <class Item>
Item QueueNode<Item>::getData()
{
	return data;
}

template <class Item>
QueueNode<Item>* QueueNode<Item>::getNext()
{
	return next;
}

template <class Item>
void QueueNode<Item>::setNext(QueueNode<Item>* i_next)
{
	next = i_next;
}

#endif