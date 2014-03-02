// Emil Hedemalm
// 2013-07-21

#ifndef XML_ELEMENT_H
#define XML_ELEMENT_H

#include "String/AEString.h"
#include "List/List.h"
#include "Queue/Queue.h"

/// Argument to an Element
struct XMLArgument {
	String name;
	String value;
};

class XMLElement {
	friend class XMLParser;
	XMLElement();
	~XMLElement();
public:
	/** Recursive downwards. If it cannot be found consider using the XMLParser's GetElement
		which applies the search to all top-level root-elements. */
	XMLElement * GetElement(String byName);
	/// Require ID too! Recursive on all rootElements until a valid element is found or NULL if none :)
	XMLElement * GetElement(String byName, String withID);
	/// Returns all elements with the given name.
	List<XMLElement*> GetElements(String byName); 
	/// Returns the given argument's value string if it exists, or NULL if no such argument exists within this element.
	XMLArgument * GetArgument(String byName);
	/// Debug
	void Print();
	// Wosh.
	void PrintHierarchy(int level);

	/// <XMLElementName arg="value"> data </XMLElement>
	String name;
	String data;
	List<XMLArgument*> args;
	List<XMLElement*> children;
};


#endif