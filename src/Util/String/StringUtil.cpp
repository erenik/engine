/// Emil Hedemalm
/// 2014-01-24
/// String utilities.

#include "StringUtil.h"
#include "Sorting/InsertionSort.h"
#include "Color.h"

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

String ColorString(Color* c) {
	return ColorString(*c);
}

String ColorString(const Color& c) {
	return "rgba "+ String::ToString(c.r) + " " + String::ToString(c.g) + " " + String::ToString(c.b) + " " + String::ToString(c.a);
}

String VectorString(const Vector4f& v) {
	return String::ToString(v[0]) + " " + String::ToString(v[1]) + " " + String::ToString(v[2]) + " " + String::ToString(v[3]);
}
String VectorString(const Vector3f & v, bool prefixed)
{
	return (prefixed? "xyz " : "") +String::ToString(v[0]) + " " + String::ToString(v[1]) + " " + String::ToString(v[2]);
};
String VectorString(const Vector2i & v)
{
	return String(v.x) +" "+String(v.y);
}

String VectorString(const Vector2f & v) {
	return String(v.x) + " " + String(v.y);
}

/// o.o prepends the names of all variables in the list.
void PrependStrings(List<String> & list, String withString)
{
	for (int i = 0; i < list.Size(); ++i)
	{
		list[i] = withString + list[i];
	}
}


/// Finds appropriate CSV delimiter in given string. Usually highest occurrance of the following ('\t;,')
char FindCSVDelimiter(String csvString)
{
	String tokens(";,\t");
	int maxDelim = 0, maxCount = 0;
	for (int i = 0; i < tokens.Length(); ++i)
	{
		char delim = tokens.CharAt(i);
		int count = csvString.Count(delim);
		if (count > maxCount)
		{
			maxCount = count;
			maxDelim = delim;
		}
	}
	return maxDelim;
}

/// For loading CSV files, automatically looks for and tokenizes by appropriate delimiter (',\t ;'), and abserves quotation marks for any occurences of delimiter within the actual data.
List<String> TokenizeCSVAuto(String csvString)
{
	char delimiter = FindCSVDelimiter(csvString);
	return TokenizeCSV(csvString, delimiter);
}

/// For loading CSV files, looks for the delimiter (usually comma ','), and abserves quotation marks for any occurences of commas within the actual data.
List<String> TokenizeCSV(String csvString, int delimiter)
{
	List<String> tokens;
//	csvString.PrintData();
	bool withinQuotes = false;
	const char * cString = csvString.c_str();
	String str;
	for (int i = 0; i < csvString.Length(); ++i)
	{
		char c = cString[i];
		if (c == '\"')
		{
			if (!withinQuotes)
			{
				withinQuotes = true;
			}
			/// Exiting quotes.
			else 
			{
				/// Evaluate token straight away, since we will encounter a comma next.
				tokens.Add(str);
				str = String();
				withinQuotes = false;
				i += 1;
			}
		}
		else if (c == delimiter && !withinQuotes)
		{
			tokens.Add(str);
			str = String();
		}
		else 
		{
			str += c;
		}
	}
	// Add final one.
	tokens.Add(str);
	return tokens;
}

/** Used to tokenize with some characters used to start and stop the tokenization procedure temporarily.
	Sample use-case would be to tokenize the string "Aim(7,3), Guard(2,3)" and returning "Aim(7,3)" and "Guard(2,3)",
	using the tokenizer ',' and ignoreparts "()". 
	Ignore parts should be in pairs, one starting the ignore part, the other stopping it.
*/
List<String> TokenizeIgnore(String string, String tokenizers, String ignoreParts)
{
	List<String> tokens;
	int inIgnorePart = 0;
	const char * cString = string.c_str();
	String str;
	for (int i = 0; i < string.Length(); ++i)
	{
		char c = cString[i];
		for (int i = 0; i < ignoreParts.Length(); i += 2)
		{
			if (c == ignoreParts.c_str()[i])
				++inIgnorePart;
			if (c == ignoreParts.c_str()[i+1])
				--inIgnorePart;
		}

		if (tokenizers.Contains(c) && inIgnorePart == 0)
		{
			tokens.Add(str);
			str = String();
		}
		else 
		{
			str += c;
		}
	}
	// Add final one.
	tokens.Add(str);
	return tokens;
}



/// Conversion.
List<float> StringListToFloatList(List<String> & stringList)
{
	List<float> floats;
	for (int i = 0; i < stringList.Size(); ++i)
	{
		float f = stringList[i].ParseFloat();
		floats.Add(f);
	}
	return floats;
}

List<int> StringListToIntList(List<String> & stringList) 
{
	List<int> ints;
	for (int i = 0; i < stringList.Size(); ++i)
	{
		float f = stringList[i].ParseInt();
		ints.Add(f);
	}
	return ints;
}

void PrintStringList(List<String> & toPrint, bool sortingValueToo)
{
	for (int i = 0; i < toPrint.Size(); ++i)
	{
		std::wcout<<"\n"<<toPrint[i];
		if (sortingValueToo)
			std::cout<<" sortingValue: "<<toPrint[i].sortingValue;
	}
}


/// Assigns values to strings based on numbers found within.
void AssignStringsValues(List<String> & stringList, bool debugPrint /*= false*/)
{
	// Assign them sorting values.
	for (int i = 0; i < stringList.Size(); ++i)
	{
		String & string = stringList[i];
		String numberized = string.Numberized();
		if (debugPrint)
			std::wcout<<"\nUnsorted list "<<i<<": "<<string;
		string.sortingValue = numberized.ParseInt();
		if (debugPrint)
			std::cout<<" value: "<<string.sortingValue;
	}
}

/// Sorts by assigned values.
void SortStrings(List<String> & listToSort, bool debugPrint /*= false*/)
{
	//// Sort the LIST!
	List<String> sorted;
	List<Sortable*> unsortedList, sortedList;

	/// copy pointers of all strings.
	for (int i = 0; i < listToSort.Size(); ++i)
	{
		unsortedList.Add((Sortable*)(&listToSort[i]));
	}

	// Assign them sorting values.
	InsertionSort insertionSort;
	insertionSort.Sort(unsortedList, sortedList, true);
	for (int i = 0; i < sortedList.Size(); ++i)
	{
		String & string = *(String*)sortedList[i];	
		if (debugPrint)
			std::wcout<<"\nSorted list "<<i<<": "<<string;
		sorted.Add(string);
	}
	// Copy list.
	listToSort = sorted;
}



/// Assigns values and sorts, based on found numbers inside.
void SortStringsByNumberizedValues(List<String> & listToSort, bool debugPrint /*= false*/)
{
	AssignStringsValues(listToSort);
	if (debugPrint)
		PrintStringList(listToSort, debugPrint);
	SortStrings(listToSort);
	std::cout<<"\nSorted "<<listToSort.Size()<<" strings.";
	if (debugPrint)
		PrintStringList(listToSort, debugPrint);
}
