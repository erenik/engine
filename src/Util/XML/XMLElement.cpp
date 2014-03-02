// Emil Hedemalm
// 2013-07-21

#include "XMLElement.h"

XMLElement::XMLElement(){
}
XMLElement::~XMLElement(){
	CLEAR_AND_DELETE(children);
	CLEAR_AND_DELETE(args);
}

/** Recursive downwards. If it cannot be found consider using the XMLParser's GetElement
	which applies the search to all top-level root-elements. */
XMLElement * XMLElement::GetElement(String byName){
	XMLElement * result = NULL;
	for (int i = 0; i < children.Size(); ++i){
		XMLElement * child = children[i];
		if (child->name == byName)
			return child;
		result = child->GetElement(byName);
		if (result)
			return result;
	}
	return NULL;
}

/// Require ID too! Recursive on all rootElements until a valid element is found or NULL if none :)
XMLElement * XMLElement::GetElement(String byName, String withID){
	for (int i = 0; i < children.Size(); ++i){
		XMLElement * child = children[i];
		if (child->name == byName){
			XMLArgument * arg = child->GetArgument("id");
			if (arg == NULL)
				continue;
			if (arg->value == withID)
				return child;
		}
	}
	return NULL;
}


/// Returns all elements with the given name.
List<XMLElement*> XMLElement::GetElements(String byName){
	List<XMLElement*> list, tmp;
	for (int i = 0; i < children.Size(); ++i){
		XMLElement * child = children[i];
		if (child->name == byName)
			list.Add(child);
		/// Fetch additional such elements from our children, if any.
		tmp = child->GetElements(byName);
		if (tmp.Size())
			list += tmp;
	}
	return list;
}

/// Returns the given argument's value string if it exists, or NULL if no such argument exists within this element.
XMLArgument * XMLElement::GetArgument(String byName){
	for (int i = 0; i < args.Size(); ++i){
		XMLArgument * arg = args[i];
		if (arg->name == byName)
			return arg;
	}
	return NULL;
}

/// Debug
void XMLElement::Print(){
	if (children.Size())
		std::cout<<"\n- Children "<<children.Size()<<":";
	for (int i = 0; i < children.Size(); ++i){
		XMLElement * child = children[i];
		std::cout<<" "<<child->name;
	}

	if (args.Size())
		std::cout<<"\n- Args "<<args.Size()<<":";
	for (int i = 0; i < args.Size(); ++i){
		XMLArgument * arg = args[i];
		std::cout<<" "<<arg->name;
	}
	if (data.Length() > 0){
		std::cout<<"\n- Data length: "<<data.Length();
	}
}

// Wosh.
void XMLElement::PrintHierarchy(int level){
	std::cout<<"\n";
	for (int i = 0; i < level; ++i)
		std::cout<<"-";
	std::cout<<name;
	for (int i = 0; i < children.Size(); ++i){
		children[i]->PrintHierarchy(level+1);
	}
}