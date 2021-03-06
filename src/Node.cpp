/*


#include "Node.h"

Node::Node(){
	child = NULL;
	sibling = NULL;
	parent = NULL;
}

Node::~Node(){
	Node * ptr = child;
	while (ptr != NULL){
		Node * toDelete = ptr;
		ptr = ptr->sibling;
		delete toDelete;
	}
}

void Node::Render(){
	if (child != NULL)
		child->Render();
	if (sibling != NULL)
		sibling->Render();
}

/** Calls render on all children. 
void Node::renderChildren(GraphicsState &state){
	Node * ptr = child;
	while (ptr != NULL){
		ptr->Render();
		ptr = ptr->sibling;
	}
}

void Node::renderSibling(GraphicsState &state){
	if (sibling != NULL)
		sibling->Render();
}

void Node::addChild(Node * node){
	if (child == NULL){
		child = node;
		node->parent = this;
	}
	else {
		Node * n = child;
		while (n->sibling != NULL)
			n = n->sibling;
		n->sibling = node;
		node->parent = this;
	}
}

void Node::addSibling(Node * node){
	if (sibling == NULL){
		sibling = node;
		node->parent = parent;
	}
	else {
		Node * n = sibling;
		while (n->sibling == NULL)
			n = n->sibling;
		n->sibling = node;
		node->parent = parent;
	}
}

/** Calls delete on this node's child. Recursively deleting everything below. Sets the child pointer to null afterwards. 
void Node::deleteChild(){
	Node * toDelete = child;
	child = NULL;
	if (toDelete)
		delete toDelete;
}

*/

