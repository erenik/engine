
#ifndef NODE_H
#define NODE_H

#include <GL/glew.h>
#include "MathLib.h"
struct GraphicsState;

/// A base node class for usage in a Scenegraph rendering tree.
class Node {

public:
	/// Default constructor.
	Node();
	/// Default destructor. Deallocates all children and siblings if any.
	~Node();

	/** Calls render on children and siblings accordingly.
	*/
	virtual void Render();
	/** Calls render on all children. */
	virtual void renderChildren(GraphicsState &state);
	/** Calls render on all siblings. */
	virtual void renderSibling(GraphicsState &state);

	/** Tree modification function.
		Postcondition: Adds a child below this node. If it already has a child, the node will be placed at the next open sibling location.
	*/
	void addChild(Node * node);
	/** Tree modification function.
		Postcondition: Adds a child at first empty location along the sibling chain.
	*/
	void addSibling(Node * node);

	/** Calls delete on this node's child. Recursively deleting everything below. Sets the child pointer to null afterwards.
	*/
	void deleteChild();



private:
	/** Pointer to this node's first child. Further children will be stored in a sibling-pointer-chain beginning from the first child. */
	Node * child;
	/** Pointer to this node's sibling. */
	Node * sibling;
	/** Pointer to this node's parent. Note that this is not currently updated correctly if any node is deleted/removed. */
	Node * parent;
};

#endif
