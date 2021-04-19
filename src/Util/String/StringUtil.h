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
/// Utility function for merging lines, using the glue between each line.
String MergeLines(List<String> lines, String glue = "\r\n");
/// Fetches target section in string using given start and end-tokens. Returns the first occurance, if any, or an empty string if none such exist.
String GetSection(String inString, char withStartToken, char andEndToken);

/// o.o prepends of all the strings in a list.
void PrependStrings(List<String> & list, String withString);

/// Finds appropriate CSV delimiter in given string. Usually highest occurrance of the following ('\t;,')
char FindCSVDelimiter(String csvString);
/// For loading CSV files, automatically looks for and tokenizes by appropriate delimiter (',\t ;'), and abserves quotation marks for any occurences of delimiter within the actual data.
List<String> TokenizeCSVAuto(String csvString);
/// For loading CSV files, looks for the delimiter (usually comma ','), and abserves quotation marks for any occurences of commas within the actual data.
List<String> TokenizeCSV(String csvString, int delimiter = ',');
/** Used to tokenize with some characters used to start and stop the tokenization procedure temporarily.
	Sample use-case would be to tokenize the string "Aim(7,3), Guard(2,3)" and returning "Aim(7,3)" and "Guard(2,3)",
	using the tokenizer ',' and ignoreparts "()". 
	Ignore parts should be in pairs, one starting the ignore part, the other stopping it.
*/
List<String> TokenizeIgnore(String string, String tokenizers, String ignoreParts);


#include "MathLib.h"
class Color;

String ColorString(std::shared_ptr<Color> c);
String ColorString(const Color& c);
String VectorString(const Vector4f& v);
String VectorString(const Vector3f & v);
String VectorString(const Vector2i & v);
String VectorString(const Vector2f & v);

/// Conversion.
List<float> StringListToFloatList(List<String> & stringList);
List<int> StringListToIntList(List<String> & stringList);

void PrintStringList(List<String> & toPrint, bool sortingValueToo = false);
/// Assigns values to strings based on numbers found within.
void AssignStringsValues(List<String> & stringList, bool debugPrint = false);
/// Sorts by assigned values.
void SortStrings(List<String> & listToSort, bool debugPrint = false);

/// Assigns values and sorts, based on found numbers inside.
void SortStringsByNumberizedValues(List<String> & listToSort, bool debugPrint = false);


#endif
