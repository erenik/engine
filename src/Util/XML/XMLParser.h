// Emil Hedemalm
// 2013-07-21

#ifndef XML_PARSER_H
#define XML_PARSER_H

/// ...Because I like own classes and own structurizations of things :)
class XMLElement;

#include "String/AEString.h"
#include "List/List.h"

/// Eased definitions for usage.
#define Xparser		XMLParser

class XMLParser {
public:
	XMLParser();
	~XMLParser();
	
	/// Wosh. Returns false if could not le open. yo.
	bool Read(String fromFile);
	/** After reading, parsing will create all separate XMLElements and store them hierarchically.
		NOTE: For huge files the parser can take a while.
	*/
	bool Parse();
	
	/// Wosh. Recursive on all rootElements until a valid element is found or NULL if none :)
	XMLElement * GetElement(String byName);
	/// Recursive fetcher which takes into consideration one single attribute-value combination which must fit as well.
	XMLElement * GetElement(String byName, String withAttribute, String thatHasGivenAttributeValue);
	
	List<XMLElement*> rootElements;

	/// The whole data, as one big fat chunk.
	char * data;
	int size;

	/// Full comprehensive list of lines of the file?
	List<String> lines;
	
private:
	
};

#endif
