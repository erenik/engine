// Emil Hedemalm
// 2013-07-21

#include "XMLParser.h"
#include "XMLElement.h"
#include <fstream>
#include <Globals.h>


/// /////////////// PRUURRURS
XMLParser::XMLParser()
{
	data = NULL;
	size = 0;
}

XMLParser::~XMLParser()
{
	if (data)
		delete[] data;
	data = NULL;
	CLEAR_AND_DELETE(rootElements);
}


/// Wosh. Returns false if could not le open. yo.
bool XMLParser::Read(String fromFile)
{
	assert(data == NULL);
	std::fstream file;
	file.open(fromFile.c_str(), std::ios_base::in);
	if (!file.is_open()){
		std::cout<<"\nERROR: Could not open filestream to "<<fromFile<<" in XMLParser::Read(String fromFile)!";
		file.close();
		return false;
	}
	// Get file length
	file.seekg( 0, std::ios::end );
	size = (int) file.tellg();
	// Allocate temp char array.
	data = new char [size];
	memset(data, 0, size);
	file.seekg( 0, std::ios::beg);

	// Read data
	file.read((char*) data, size);
	file.close();
	return true;
}

/** After reading, parsing will create all separate XMLElements and store them hierarchically.
	NOTE: For huge files the parser can take a while.
*/
bool XMLParser::Parse()
{
	assert(data);
	if (data == NULL)
		return false;
	/// Use the queue as a stack just!
	List<XMLElement*> elementStack;
	/// Current element.
	XMLElement * element = NULL;
	XMLArgument * argument = NULL;

	String line;

/// For spamming cout with stuff o-o;
// #define DEBUG_XML

	enum states{
		NONE, BEGUN,
		CREATING_ELEMENT,
		READING_ELEMENT_NAME,
		READING_ELEMENT_ARGS,
		READING_ARG_NAME,
		READING_ARG_VALUE,
		READING_DATA,
		ENDING_ELEMENT, // <- once a /-sign appeared or end-tag.
	};
	int previousState = NONE;
	int state = NONE;
	/// Build buff buff

	/// For when finding where to.. stuff.
	char * begin;
	char previousCharacter = 0;
	char character = 0;
	char nextCharacter = 0;

	for (int i = 0; i < size; ++i)
	{
		char * c = &data[i];
		previousCharacter = character;
		character = *c;
		nextCharacter = *(c + 1);
		if (character == 0){
			std::cout<<"\nNULL before end of file. No good, yes? No? o.o;";
			std::cout<<"\nConsidering file finished!";
			break;
		}
		/// State-dependent reading
		switch(state){
			/// Flagged from the previous loop, so we should now be at a '>' sign!
			case ENDING_ELEMENT: {
#ifdef DEBUG_XML
				std::cout<<"\nEnding element: "<<element->name;
				element->Print();
#endif
				/// Current one should have been at the top of the stack, so ensure it now.
				XMLElement * parent = NULL;
				if (elementStack.Size())
					parent = elementStack[elementStack.Size()-1];
				/// Pop the current element as it is now done.
				if (parent){
					state = READING_DATA;
					begin = NULL;
				}
				else
					state = BEGUN;
				element = NULL;
				break;
			}
			case NONE: case BEGUN: {
				if (character == '<'){
					state = CREATING_ELEMENT;
				}
				break;
			}
			case CREATING_ELEMENT: {
				// Begin an element.
				state = READING_ELEMENT_NAME;
				element = new XMLElement();
#ifdef DEBUG_XML
				std::cout<<"\nCreating element.. ";
#endif
		//		elementStack.Add(element); // Add ourselves to the stack. Remove once finished.
				if (elementStack.Size() == 0)
					rootElements.Add(element);
				else {
					// Add it to parent straight away.
				//	std::cout<<"\nParent stack size: "<<elementStack.Size();
					int index = elementStack.Size()-1;
					XMLElement * parent = elementStack[index];
					assert(parent);
#ifdef DEBUG_XML
					std::cout<<"Adding to parent "<<parent->name;
#endif
					parent->children.Add(element);
				}
				begin = c;
				state = READING_ELEMENT_NAME;
				break;
			}
			case READING_ELEMENT_NAME: {
				// Space = args will come
				switch(character){
					case ' ': case '/': case '>':
						element->name = String(begin, c);
#ifdef DEBUG_XML
						std::cout<<"\nElement name parsed: "<<element->name;
#endif
						if (element->name == "extra")
							c = c + 2 -1 * 2;
						break;
				}
				if (character == ' '){
					begin = 0;
					state = READING_ELEMENT_ARGS;
				}
				else if (character == '/' && ((nextCharacter == '>') || (previousCharacter == '<'))){
					state = ENDING_ELEMENT;
				}
				else if (character == '>'){
					state = READING_DATA;
					begin = NULL;
				}
				break;
			}
			case READING_ELEMENT_ARGS: {
				/// Interpret an ending ? as a / for simplicity and end this element, since it's just an info-tag...
				if (character == '?'){
					state = ENDING_ELEMENT;
					break;
				}
				if (character == '>'){
					state = READING_DATA;
					begin = NULL;
				}
				else if (character == '/'){
					state = ENDING_ELEMENT;
				}
				else if (argument == 0 && character != ' '){
					begin = c;
					/// Create the argument
					argument = new XMLArgument();
					element->args.Add(argument);
				}
				else if (character == '='){
					assert(argument);
					assert(begin);
					argument->name = String(begin, c);
					state = READING_ARG_VALUE;
					begin = 0;
				}

				break;
			}
			case READING_ARG_VALUE: {
				if (begin == 0 && character == '\"'){
					begin = c+1;
				}
				else if (character == '\"'){
					argument->value = String(begin, c);
					argument = NULL;
					state = READING_ELEMENT_ARGS;
				}
				break;
			}
			case READING_DATA: {
				if (begin == 0){
					/// If begun reading data, push element to the stack.
					if (element)
						elementStack.Add(element);
					begin = c;
				}
				if (character == '<'){
					char nextChar = *(c+1);
					// End tag found..!
					if (nextChar == '/'){
						/// Remove from stack the latest stack element
						element = elementStack[elementStack.Size()-1];
						elementStack.Remove(element);
						/// Only fetch data if we have no children, seems default for XML files?
						if (element->children.Size() == 0)
							element->data = String(begin, c);
						state = ENDING_ELEMENT;
					}
					// Children it seems! :)
					else {
						state = CREATING_ELEMENT;
					}
				}
				break;
			}
			default:
				break;
		}

	}

	// DO magic
#ifdef DEBUG_XML
	std::cout<<"\nXML file hopefully parsed successfully... Printing data o-o";
	for (int i = 0; i < rootElements.Size(); ++i)
		rootElements[i]->PrintHierarchy(0);
#endif
	return true;
}

/// Wosh. Recursive on all rootElements until a valid element is found or NULL if none :)
XMLElement * XMLParser::GetElement(String byName)
{
	for (int i = 0; i < rootElements.Size(); ++i)
	{
		XMLElement * e = rootElements[i]->GetElement(byName);
		if (e)
			return e;
	}
	return NULL;
}

/// Recursive fetcher which takes into consideration one single attribute-value combination which must fit as well.
XMLElement * XMLParser::GetElement(String byName, String withAttribute, String thatHasGivenAttributeValue)
{
	for (int i = 0; i < rootElements.Size(); ++i){
		XMLElement * e = rootElements[i]->GetElement(byName, withAttribute, thatHasGivenAttributeValue);
		if (e)
			return e;
	}
	return NULL;
}
	