/// Emil Hedemalm
/// 2014-01-24
/// String utilities.

#include "StringUtil.h"
#include "Sorting/InsertionSort.h"

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


String VectorString(Vector3f v)
{
	return String::ToString(v.x) + " " + String::ToString(v.y) + " " + String::ToString(v.z);
};


/// o.o prepends the names of all variables in the list.
void PrependStrings(List<String> & list, String withString)
{
	for (int i = 0; i < list.Size(); ++i)
	{
		list[i] = withString + list[i];
	}
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
