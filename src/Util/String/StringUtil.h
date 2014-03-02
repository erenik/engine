/// Emil Hedemalm
/// 2014-01-24
/// Some util functions based on the String class.

#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include "AEString.h"


#ifdef WINDOWS
#define StringToLongLong    _atoi64
#elif defined LINUX
#define StringToLongLong(a)    strtoull(a, NULL, 10)
#endif


/// Utility function that splits all lines, removing any CR LF or the combination CRLF
List<String> GetLines(String string);
/// Utility function for merging lines.
String MergeLines(List<String> lines, String newLine = "\r\n");
/// Fetches target section in string using given start and end-tokens. Returns the first occurance, if any, or an empty string if none such exist.
String GetSection(String inString, char withStartToken, char andEndToken);

#endif
