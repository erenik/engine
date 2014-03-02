/// Emil Hedemalm
/// 2014-01-24
/// String utilities.

#include "StringUtil.h"

/// Utility function that splits all lines, removing any CR LF or the combination CRLF
List<String> GetLines(String string){
	// Remove any \r
	string.Remove('\r', true);
	List<String> list = string.Tokenize("\n");
	return list;	
}

/// Utility function for merging lines.
String MergeLines(List<String> lines, String newLine /*= "\r\n"*/){
	String merged;
	for (int i = 0; i < lines.Size(); ++i){
		merged += lines[i];
		if (i < lines.Size()-1)
			merged += newLine;
	}
	return merged;
}

/// Fetches target section in string using given start and end-tokens. Returns the first occurance, if any, or an empty string if none such exist.
String GetSection(String inString, char withStartToken, char andEndToken){
	int startIndex = -1, endIndex = -1;
	for (int i = 0; i < inString.Length(); ++i){
		char c = inString.CharAt(i);
		if (c == withStartToken)
			startIndex = i+1;
		if (c == andEndToken)
			endIndex = i;
		if (startIndex >= 0 && endIndex >= 0)
			break;
	}
	if (startIndex < 0 && endIndex < 0)
		return String();
	String newString;
	for (int i = startIndex; i < endIndex; ++i){
		newString.Add(inString.CharAt(i));
	}
	std::cout<<"newString: "<<newString<<" length: "<<newString.Length();
	return newString;
}
