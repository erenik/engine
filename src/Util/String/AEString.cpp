/// Emil Hedemalm
/// 2013-03-01
/// updated 2013-06-19 by Fredrik Larsson


#include "AEString.h"
#include <cstdio>       // used for snprintf(...), replace itoa

#include "OS/OS.h"
#include <fstream>

#include <cstring>

//#include <stdio.h>
#ifdef WINDOWS
#define snprintf _snprintf
#endif

// #define USE_BLOCK_ALLOCATOR
#ifdef USE_BLOCK_ALLOCATOR
#include "System/BlockAllocator.h"
BlockAllocator stringAllocator;
#endif

/// Initializes block allocator to be used with strings.
void String::InitializeAllocator()
{
#ifdef USE_BLOCK_ALLOCATOR
	stringAllocator.Initialize(200000);
#endif
}

String::String()
{
	Nullify();
}

String::~String()
{
//	std::cout<<"\nString destructor.";
    if (testbuff){
        if (strcmp(testbuff, "Deleted") == 0 || strlen(testbuff) > 3){
            std::cout<<"\nWARNIGNGISNGDNSG: DEleted string is deleted hallready D:";
        }
        strcpy(testbuff, "Deleted");
    }
	Delete();
}
String::String(const String * string){
	Nullify();
	type = string->type;
	Reallocate(string->Length()+1);
	Copy(*string, type);
}

/// Copy constructor and assignment operators
String::String(const String & string){
	Nullify();
	type = string.type;
	int lengthRequired = string.Length() + 1;
	Reallocate(lengthRequired);
	Copy(string, type);
}
String::String(const char c){
	Nullify();
	type = String::CHAR;
	Reallocate(1+1);
	arr[0] = c;
	arr[1] = '\0';
}
String::String(const char * string){
	Nullify();
	type = String::CHAR;
	int lengthArr = 0;
    if (arr)
        lengthArr = strlen(arr);

    int lengthStr = 0;
    if (string)
        lengthStr = strlen(string);
    Reallocate(lengthStr+1);
//    std::cout<<"\nStr: "<<string;
 //   std::cout<<"\nString lengths: "<<lengthArr<<" "<<lengthStr<<" Arr:"<<arr<<" Str: "<<string;
//	std::cout<<"\nStrlen: "<<lengthStr<<" ArraySize: "<<arraySize;
	strncpy(arr, string, arraySize-1);
	assert(arraySize > 0);
}
// User-defined length of any given c_str :)
String::String(const char * from, const char * to){
	Nullify();
	type = String::CHAR;
	int length = to - from;
	Reallocate(length+1); // + Null-signus?
	strncpy(arr, from, length);
	arr[length] = '\0';
}

String::String(const wchar_t * string){
	Nullify();
	arraySize = Size(string);
	assert(arraySize > 0);
	type = String::WIDE_CHAR;
	if (arraySize){
		Reallocate(arraySize+1);
		wcscpy(warr, string);
	}
}
// ..assignment operators
const String& String::operator = (const String & otherString)
{
	Delete();
	assert(arraySize == 0);
	// Yes, allow assigning a NULL(-lengthed) string...
	if (otherString.arraySize <= 0){
		type = NULL_TYPE;
		return *this;
	}
	type = otherString.type;
	Reallocate(otherString.arraySize);
	Copy(otherString, type);
	return *this;
}

const String& String::operator = (const char * otherString)
{
	Delete();
	if (otherString == NULL){
		arraySize = 0;
		type = String::NULL_TYPE;
		return *this;
	}
    int length = Size(otherString);
	assert(length < 50000 && length >= 0);
	type = String::CHAR;
	Reallocate(length+1);
    memset(arr, 0, arraySize);
/*	std::cout<<"lall";
	std::cout<<"Length of \""<<otherString<<"\": "<<length<<" arraySize: "<<arraySize;
	for (int i = 0; i < arraySize; ++i)
	{
		char c = arr[i];
		if (c == 0)
			std::cout<<"0";
		else
			std::cout<<c;
	}
	*/
	// Safer: http://www.cplusplus.com/reference/cstring/memmove/
	memmove(arr, otherString, length);
	assert(arraySize > 0);
	return *this;
}
const String& String::operator = (const wchar_t * otherString){
	Delete();
	arraySize = Size(otherString);
	assert(arraySize > 0);
	type = String::WIDE_CHAR;
	Reallocate(arraySize+1);
	wcscpy(warr, otherString);
	return *this;
}

/// Concatenation assignment operators
const String& String::operator += (const String & otherString){
	Add(otherString);
	return *this;
}
const String& String::operator += (const char * otherString){
	Add(otherString);
	return *this;
}
const String& String::operator += (const wchar_t * otherString){
	assert(false && "Implement");
	return *this;
}

// Conversion operators
String::operator const char * () {
	if (type == NULL_TYPE)
		return NULL;
	if (type != CHAR){
		if (arr)
			delete[] arr;
		assert(arraySize > 0);
		arr = new char[arraySize];
		wcstombs(arr, warr, arraySize);
	}
	else if (type == NULL_TYPE)
		return NULL;
	return arr;
}
String::operator const char * () const {
	if (type == NULL_TYPE)
		return NULL;
	assert(type == CHAR);
	return arr;
}
String::operator const wchar_t * () const {
	if (type == NULL_TYPE)
		return NULL;
	assert(type == WIDE_CHAR && "Convert to wide-char before using it, yo.");
	return warr;
}
String::operator const bool () const {
	switch(type){
		case CHAR:
			if (arr)
				return true;
			return false;
		case WIDE_CHAR:
			if (warr)
				return true;
			return false;
		default:
			assert("Bad type in String::operator const bool()");
	}
	return false;
}

// Static Parsing functions
String String::ToString(const int value){
	char buf[50];
    int success;
	success = snprintf(buf, sizeof(buf), "%d", value);
    if (!success)
        std::cout << "Util::ToString(const int) failed";
    return String(buf);

}
String String::ToString(const float value, int decimalsAfterZero /* = -1*/){
	char buf[50];
    int success;
	/// If default, use as many zeros as needed?
	if (decimalsAfterZero < 0)
		decimalsAfterZero = 10;
    String format = "%5."+ ToString(decimalsAfterZero) + "f";
    success = snprintf(buf, sizeof(buf), format.c_str(), value);
	if (!success){
        std::cout << "Util::ToString(const float) failed";
		return String();
	}
	/// Remove unneccessary trailing zeros, and the decimal point if it's still there.
	int len = strlen(buf);
	bool done = false;
	for (int i = len-1; i > 0; --i){
		switch(buf[i]){
			/// Waste of space	
			case '0': 
				buf[i] = '\0';
				break;
			/// Waste of space, but end cutoff after this, or we'll clip away valueable numbers.
			case '.':
				buf[i] = '\0';
				done = true;
				break;
			/// Valid number, stop cutting away digits.
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				done = true;
				break;
		}
		if (done)
			break;
	}
    return String(buf);

}

/// For example used with the arguments (lines,"//","/*","*/") for C++
List<String> String::RemoveComments(List<String> lines,
									String singleLineComment,
									String multipleLineCommentStart,
									String multipleLineCommentEnd)
{
	List<String> newLines;
	bool inCommentBlock = false;
	for (int i = 0; i < lines.Size(); ++i){
		String line = lines[i];
		if (line.Contains(singleLineComment)){
			/// Tokenize and remove the remainder.
			/// Just remove it for now... cba.
			continue;
		}
		else if (line.Contains(multipleLineCommentStart)){
			inCommentBlock = true;
		}
		if (line.Contains(multipleLineCommentEnd)){
			inCommentBlock = false;
		}
		if (inCommentBlock)
			continue;
		newLines.Add(line);
	}
	return newLines;
}

/// Removing
String String::operator - (const String & otherString){
	if (Contains(otherString))
		Remove(otherString);
	return String(this);
}
/// Concatenating
String String::operator + (const String & otherString) {
	String newString = String(this);
	newString.Add(otherString);
	return newString;
}
String String::operator + (const char * otherString){
	String string(this);
	string.Add(otherString);
	return string;
}

String String::operator + (const char c){
	String string(this);
	string.Add(c);
	return string;
}
String String::operator + (const int value){
	String string(this);
	string.Add(String::ToString(value));
	return string;
}

/// Printing out data
std::ostream& operator <<(std::ostream& os, const String& str){
	switch(str.type){
		case String::CHAR:
			os << str.arr;
			break;
		case String::WIDE_CHAR:
			os << str.warr;
			break;
		default:
			os << "";
			//assert(false && "Bad type in String::operator <<();");
	}
	return os;
}


/// Returns a substring, from index to index, -1 signifies end of the string.
String String::Part(int fromIndex /*= 0*/, int toIndex /*= -1*/){
	String newString;
	if (toIndex <= -1)
		toIndex = arraySize;
	int partSize = toIndex - fromIndex;
	if (partSize <= 0)
		return newString;
	assert(partSize);
	// Set type and reallocate.
	newString.type = type;
	newString.Reallocate(partSize+1);
	int charsWritten = 0;
	switch(type){
		case CHAR:
			assert(arr);
			assert(newString.arr);
			for (int i = fromIndex; i < toIndex; ++i){
				newString.arr[charsWritten] = arr[i];
				++charsWritten;
			}
			break;
		case WIDE_CHAR:
			assert(warr);
			assert(newString.warr);
			for (int i = fromIndex; i < toIndex; ++i){
				newString.warr[charsWritten] = warr[i];
				++charsWritten;
			}
			break;
	}
	return newString;
}

/// Quering functions
bool String::Equals(const String & otherString) const {
	int type = Type(), type2 = otherString.Type();

	// Unneccessary? Should be able to compare them even if different, yes?
//	if (type != type2)
//		return false;
	int result;
	if (comparisonMode == CASE_SENSITIVE){
		switch(type){
			case String::NULL_TYPE: default:
				if (otherString == 0)
					return true;
				return false;
				break;
			case String::CHAR:
				if (otherString.arr == NULL){
					// Allocate temp array to compare with
					char * tempArray = new char[otherString.arraySize];
					result = strcmp(arr, tempArray);
					delete[] tempArray;
					if (result == 0)
						return true;
					break;
				}
				assert(arr && otherString.arr);
				result = strcmp(arr, otherString.arr);
				if (result == 0)
					return true;
				break;
			case String::WIDE_CHAR:
				// If not same type, convert first
				if (otherString.type == CHAR){
					char * compareString;
					bool requiredTempAllocation = false;
					if (arr == NULL){
						compareString = new char[arraySize];
						requiredTempAllocation = true;
					}
					else
						compareString = arr;
					wcstombs(compareString, warr, otherString.arraySize);
					assert(compareString && otherString.arr);
					result = strcmp(compareString, otherString.arr);

					if (requiredTempAllocation == true)
						delete[] compareString;

					if (result == 0)
						return true;
					break;
				}
				assert(warr && otherString.warr);
				result = wcscmp(warr, otherString.warr);
				if (result == 0)
					return true;
				break;
		}
	}
	else if (comparisonMode == NOT_CASE_SENSITIVE){
		switch(type){
			case CHAR:
			{
				assert(type == CHAR && otherString.type == CHAR);
				int currentChar = 0;
				while(true){
					if(toupper(arr[currentChar]) != toupper(otherString.arr[currentChar]))
						return false;
					else if (arr[currentChar] == '\0' && otherString.arr[currentChar] == '\0'){
						return true;
					}
					else if (arr[currentChar] == '\0'){
						return false;
					}
					else if (otherString.arr[currentChar] == '\0'){
						return false;
					}
					++currentChar;
					assert(currentChar < arraySize);
				}
				break;
			}
			case WIDE_CHAR:
				// Create char-equivalent and do a comparison with it if needed
				if (otherString.type == CHAR){
					String tmpString;
					tmpString = *this;
					tmpString.ConvertToChar();
					return tmpString.Equals(otherString);
				}
		}
	}
	else {
		assert(false && "shouldn't be here");
	}
	return false;
}

bool String::ContainsChar(char c) const{
	if (type == CHAR){
		for (int i = 0; i < arraySize; ++i)
			if (arr[i] == c)
				return true;
	}
	else if (type == WIDE_CHAR){
		for (int i = 0; i < arraySize; ++i)
			if (warr[i] == c)
				return true;
	}
    return false;
}

/// Counts occurences of target character in the string.
int String::Count(char c) const
{
	int count = 0;
	if (type != CHAR)
		return 0;
	for (int i = 0; i < arraySize; ++i)
	{
		if (arr[i] == c)
			++count;
	}
	return count;
}

bool String::Contains(const String & subString){
	if (subString.Length() == 0)
		return false;
	switch(type){
		case NULL_TYPE: default:
		//	assert(false && "Bad type");
			return false;
		case String::CHAR:
			// If needed, convert the sub string to char before continuing
			if (subString.type == String::WIDE_CHAR){
				char * tempArray = new char[subString.arraySize];
				wcstombs(tempArray, subString.warr, subString.arraySize);
				char * result = strstr(arr, tempArray);
				delete[] tempArray;
				if (result)
					return true;
				return false;
			}

			if (subString.arr == NULL || arr == NULL)
				return false;
			// Not case-senstitive.. do some more work..!
			if (comparisonMode == NOT_CASE_SENSITIVE){
				String temp = arr;
				String temp2 = subString;
				temp.ToUpper();
				temp2.ToUpper();
				if (temp.Contains(temp2))
					return true;
			}
			else {// Case-sensitive
        //        std::cout<<"\nArr: "<<arr;
        //        PrintData();
        //        std::cout<<"\nSubString.arr: "<<subString.arr;
        //        subString.PrintData();
                if (strstr(arr, subString.arr)){
			//	    std::cout<<"\nFound substring "<<subString.arr;
			//	    std::cout<<"\nIn string "<<arr;
					return true;
				}
			}
			break;
		case String::WIDE_CHAR:
			// If needed, convert our wide char string to simple string before doing the comparison!
			if (subString.type == String::CHAR){
				if (arr) 
					delete[] arr;
				arr = new char[arraySize];
				wcstombs(arr, warr, arraySize);
				if (comparisonMode == NOT_CASE_SENSITIVE){
					String temp = arr;
					String temp2 = subString;
					temp.ToUpper();
					temp2.ToUpper();
					if (temp.Contains(temp2))
						return true;
				}
				else if (strstr(arr, subString.arr))
					return true;
				return false;
			}
			if (wcsstr(warr, subString.warr))
				return true;
	}
	return false;
}

/// Similar to Contains but works only on the beginning of the string.
bool String::StartsWith(const String & subString)
{
	if (subString.Length() == 0)
		return false;
	String thisSubString = Part(0, subString.Length());
	if (thisSubString == subString)
		return true;
	return false;
}

/// Removes string subpart, returns true if it found and successfully removed it, false if not. Works recursively if all is true.
bool String::Remove(const String & subString, bool all /*= false*/){
    switch(type){
		case NULL_TYPE: default:
			assert(false && "Bad type");
			return false;
		case String::CHAR: {

            // Check if it exists within.
			char * beginPtr = strstr(arr, subString.arr);
			if (!beginPtr)
				return false;

            int subStringLength = subString.Length();
            assert(subStringLength);
            char * endPtr = beginPtr + subStringLength;

            for (int i = (long)(beginPtr - arr); i < arraySize && i + subStringLength < arraySize; ++i){
                char * ptr, * ptr2;
                ptr = arr + i;
                ptr2 = arr + subStringLength + i;
                *ptr = *ptr2;
           //     std::cout<<"\nArrnow: "<<arr;
            }
            /// Should not overlap in memory, so avoid strcpy here.
            // strcpy(beginPtr, endPtr); // +1 so null-sign comes along!

		//	strcpy(charptr, &arr[index + subStringSize]);
			// If recursive, call again.
			if (all)
				Remove(subString, all);
			return true;
		}
		case String::WIDE_CHAR: 
		{
			assert(false && "implement");
			/*
			// Check if it exists within.
			wchar_t * beginPtr = wcsstr(warr, subString.warr);
			if (!beginPtr)
				return false;

            int subStringLength = subString.Length();
            assert(subStringLength);
            wchar_t * endPtr = beginPtr + subStringLength;
			/// Remove, em.
            for (int i = (long)(beginPtr - arr); i < arraySize && i + subStringLength < arraySize; ++i){
                wchar_t * ptr, * ptr2;
                ptr = arr + i;
                ptr2 = arr + subStringLength + i;
                *ptr = *ptr2;
            }
    		// If recursive, call again.
			if (all)
				Remove(subString, all);
			return true;
			*/
		}
	}
}

/// Concatenates strings
void String::Add(const String & otherString){

	this->Reallocate(Length() + otherString.Length() + 1);
	switch(this->type){
		case CHAR:
			assert(arr);
			// If wide-char merge
			if (otherString.type == WIDE_CHAR){
				this->type = WIDE_CHAR;
				if (warr)
					delete[] warr;
				warr = new wchar_t[arraySize];
				mbstowcs(warr,arr,arraySize);
				wcscat(warr, otherString.warr);
			}
			else if (otherString.type == CHAR)
				Add(otherString.arr);
			break;
		case WIDE_CHAR:
			assert(warr);
			if (otherString.type == CHAR){
				wchar_t * tmp = new wchar_t [otherString.arraySize];
				mbstowcs(tmp, otherString.arr, otherString.arraySize);
				wcscat(warr, tmp);
				delete[] tmp;
			}
			else if (otherString.type == WIDE_CHAR){
				wcscat(warr, otherString.warr);
			}
			break;
		default:
		/// Inherit type of the other string
		{
			this->type = otherString.type;
			assert(this->type && "trying to add two strings of null-type! Are you doing everything correct?");
			/// And re-add
			Add(otherString);
		}
	}
}
void String::Add(const char * otherString){
	switch(this->type){
		case CHAR:
			this->Reallocate(strlen(arr) + strlen(otherString)+1);
			strcat(this->arr, otherString);
			break;
		case WIDE_CHAR: {
			int length = 1;
			if (otherString)
				length += strlen(otherString);
			if (warr)
				length += wcslen(warr);
			this->Reallocate(length);
			wchar_t * buf = new wchar_t[length];
			buf[0] = '\0';
			mbstowcs(buf, otherString, length);
			wcscat(this->warr, buf);
			delete[] buf;
			break;
		}
		default:
			this->type = CHAR;
			this->Reallocate(strlen(otherString)+1);
			strcat(arr, otherString);
		//	assert(false);
			break;
	}
}

/// Char-wise replacement
void String::Replace(const char a, const char withB){
    switch(type){
        case String::NULL_TYPE:
            return;
        case String::CHAR:
            for (int i = 0; i < arraySize; ++i)
                if (arr[i] == a)
                    arr[i] = withB;
            break;
        case String::WIDE_CHAR:
            for (int i = 0; i < arraySize; ++i)
                if (warr[i] == a)
                    warr[i] = withB;
    }
}

// Extract data o-o
bool String::ParseBool(){
	int oldComparisonMode = this->comparisonMode;
	this->comparisonMode = String::NOT_CASE_SENSITIVE;
	bool result;
	if (this->Equals("true") || this->Equals("yes"))
		result = true;
	else if (this->Equals("false") || this->Equals("no"))
		result = false;
	else {
		std::cout<<"\nString::ParseBool:"<<c_str()<<"\n";
		assert(false && "String::ParseBool was unable to parse a true or false value");
	}
	comparisonMode = oldComparisonMode;
	return result;
}
int String::ParseInt(){
	//assert(this->type == CHAR && "Implement for wide char or convert first..!");
	if (this->type == WIDE_CHAR){
		if (arr)
			delete[] arr;
		arr = new char[arraySize];
		wcstombs(arr,warr, arraySize);
	}
	else if (this->type == NULL_TYPE)
		return 0;
	return atoi(arr);
}
float String::ParseFloat(){
	assert(this->type != NULL_TYPE);
	if (this->type == WIDE_CHAR){
		arr = new char[arraySize];
		wcstombs(arr, warr, arraySize);
	}
//	std::cout<<"\nArr:" <<arr;
    float f = (float)atof(arr);
    for (int i = 0; i < (int)strlen(arr); ++i){
        char c = arr[i];
  //      std::cout<<"\nc: (int)"<<(int)c<<" char: "<<c;
    }
//    std::cout<<"\nReturning float: "<<f;
	return (float)atof(arr);
}
double String::ParseDouble(){
	assert(this->type == CHAR && "Implement for wide char or convert first..!");
	return atof(arr);
}

/// Returns a number of strings by splitting it's current contents using the provided tokens
List<String> String::Tokenize(const char * charTokens) const {
	List<String> list;
	int tokens = strlen(charTokens);
	switch(this->type){
		case WIDE_CHAR: // Perform quick conversion
			{
			}
			break;
		case CHAR:
			break;
		default:	assert(false);
			break;
	}
	if (type == CHAR){
		assert(this->arr && this->arraySize);
		char * stringStart = arr, * stringEnd = NULL;
		const int BUFFER_SIZE = (4096*5);
		char buf[BUFFER_SIZE];
		assert(BUFFER_SIZE > arraySize);
		int tokenLength = 0;
		int i;
		String token;
		for (i = 0; i < arraySize; ++i){
			memset(buf, 0, BUFFER_SIZE);
			bool isToken = true;
			while(isToken ){
				isToken = false;
				for (int t = 0; t < tokens; ++t){
					if(arr[i] == charTokens[t]){
						isToken = true;
						// Token found, tokenize
						stringEnd = &arr[i];
						tokenLength = stringEnd - stringStart;
						++i;
						if (tokenLength <= 0){
							stringStart = &arr[i];
							// Break inner loop;
							t = tokens;
							continue;
						}
						assert(tokenLength < BUFFER_SIZE);
						strncpy(buf, stringStart, tokenLength);
						stringStart = stringEnd+1;
						token = buf;
					//	token.PrintData();
						list.Add(token);
						memset(buf, 0, BUFFER_SIZE);
					}
				}
			}
			if (arr[i] == 0){
				// Final char, so tokenize the last part too
				stringEnd = &arr[i];
				tokenLength = stringEnd - stringStart;
				++i;
				if (tokenLength <= 0){
					stringStart = &arr[i];
					i = arraySize;
					break;
				}
				strncpy(buf, stringStart, tokenLength);
				stringStart = stringEnd+1;
				String token(buf);
				list.Add(token);
				i = arraySize;
				break;
			}
		}
	}
	else if (type == WIDE_CHAR){
		int sizeofWcharT = sizeof(wchar_t);
		assert(this->warr && this->arraySize);
		wchar_t * stringStart = warr, * stringEnd = NULL;
		int stringStartIndex = 0, stringEndIndex = 0;
		const int BUFFER_SIZE = 1024;
		wchar_t buf[BUFFER_SIZE];
		int tokenLength = 0;
		int i;
		String token;
		for (i = 0; i < arraySize; ++i){
			wmemset(buf, 0, BUFFER_SIZE);
			for (int t = 0; t < tokens; ++t){
				while (warr[i] == charTokens[t]){
					// Token found, tokenize
					stringEnd = &warr[i];
					stringEndIndex = i;
					tokenLength = (stringEndIndex - stringStartIndex);
					int tokenLength2 = stringEnd - stringStart;
					++i;
					if (tokenLength <= 0){
						stringStart = &warr[stringEndIndex];
						continue;
					}
					assert(tokenLength < BUFFER_SIZE);
					wmemcpy(buf, stringStart, tokenLength);
					stringStart = stringEnd+1;
					stringStartIndex = stringEndIndex+1;
					token = buf;
					list.Add(token);
					wmemset(buf, 0, BUFFER_SIZE);
				}
			}
			if (warr[i] == 0){
				// Final char, so tokenize the last part too
				stringEnd = &warr[i];
				tokenLength = stringEnd - stringStart;
				++i;
				if (tokenLength <= 0){
					stringStart = &warr[i];
					i = arraySize;
					break;
				}
				wmemcpy(buf, stringStart, tokenLength);
				stringStart = stringEnd+1;
				String token(buf);
				list.Add(token);
				i = arraySize;
				break;
			}
		}
	}
	return list;
}

/// Returns a number of strings by dividing them by '\n' and '\r', which are removed in the process(!).
List<String> String::GetLines() const {

    List<String> list;
    switch(this->type){
		case WIDE_CHAR: // Perform quick conversion
        {
            assert(false && "GetLines currently only works for c_strings!");
            return list;
        }
			break;
		case CHAR:
			break;
		default:	assert(false);
			break;
	}


    /// Search how many occurences of \n we got.
    int newLines = 0;
    for (int i = 0; i < arraySize; ++i){
        if (arr[i] == '\n')
            ++newLines;
    }
 //   std::cout<<"\nNewlines: "<<newLines;
    list.Allocate(newLines+5);

	assert(this->arr && this->arraySize);
	char * stringStart = arr, * stringEnd = NULL;
    const int BUFFER_SIZE = 1024;
	char buf[BUFFER_SIZE];
	int i;
	bool lastOne = false;

    for (i = 0; i < arraySize; ++i){
		memset(buf, 0, 1024);

        if (arr[i] == '\n' || arr[i] == '\0'){
			if (arr[i] == '\0'){
	//		    std::cout<<"\nLast line found at row "<<list.Size();
				lastOne = true;
            }

            // Token found, tokenize
            stringEnd = &arr[i];
            int stringLength = stringEnd - stringStart;
            assert(stringLength < BUFFER_SIZE);
            strncpy(buf, stringStart, stringLength);
            stringStart = stringEnd+1;

            String token(buf);
      //      token.PrintData();
            token.Remove("\r");
            token.Remove("\n");
       //     token.PrintData();
            list.Add(token);

            // Break will only break the inner loop, make sure to flag the outside for-loop's index too!
			if (lastOne){
			    i = arraySize;
				break;
            }
			memset(buf, 0, BUFFER_SIZE);


            if (list.Size() >= 30000){
                std::cout<<"\nReturning "<<list.Size()<<" lines.";
                for (int i = 0; i < list.Size(); ++i){
                    std::cout<<"Line "<<i<<": "<<list[i].c_str();

                }
                return list;
            }


        }
    }
	return list;
}

/// Converts all characters to uppercase if possible
void String::ToUpper(){
	switch(type){
		case CHAR:
			for (int i = 0; i < arraySize; ++i){
				arr[i] = toupper(arr[i]);
			}
			break;
		case WIDE_CHAR:
			for (int i = 0; i < arraySize; ++i){
				warr[i] = toupper(warr[i]);
			}
			break;
	}

}

/// Returns the string as a char array pointer.
const char * String::c_str(){
	if (type == WIDE_CHAR){
		// Convert
		if (arr)
			delete[] arr;
		arr = new char[arraySize];
		wcstombs(arr, warr, arraySize);
	}
	return arr;
};
const wchar_t * String::wc_str(){
	if (type == CHAR){
		// Convert
		if (warr)
			delete[] warr;
		warr = new wchar_t[arraySize];
		mbstowcs(warr, arr, arraySize);
	}
	return warr;
}

char String::At(int index) const {
	assert(index >= 0 && index < arraySize);
	if (type == CHAR)
		return arr[index];
	else if (type == WIDE_CHAR)
		return (char)warr[index];
	return 0;
}

/// Reference extractor.
char & String::CharAt(int index){
	assert(index >= 0 && index < arraySize);
	if (type == CHAR)
		return arr[index];
	else if (type == WIDE_CHAR)
		assert(false && "Convert to char");
	assert(false);
}


/// Make the string a wchar_t primary type instead of char
void String::ConvertToWideChar(){
	if (type == WIDE_CHAR)
		return;
	if (type == CHAR){
		type = WIDE_CHAR;
		if (warr)
			delete[] warr;
		warr = new wchar_t[arraySize];
		mbstowcs(warr, arr, arraySize);
	}
}

/// Make the string a char primary type instead of wchar_t.
void String::ConvertToChar(){
	if (type == CHAR){
		assert(false && "Is already char-type!");
		return;
	}
	if (type == WIDE_CHAR){
		type = CHAR;
		if (arr)
#ifdef USE_BLOCK_ALLOCATOR
			stringAllocator.Deallocate(arr);
		arr = stringAllocator.AllocateNewArray<char>(arraySize);
#else
			delete[] arr;
		arr = new char [arraySize];
#endif
		assert(warr);
		assert(arraySize > 0);
		wcstombs(arr, warr, arraySize);
	}
}

/// Amount of characters (not counting ending NULL-character)
int String::Length() const {
	switch(type){
		case CHAR:
			if (!arr)
				return 0;
			return strlen(arr);
		case WIDE_CHAR:
			if (!warr)
				return 0;
			return wcslen(warr);
		default:
			return 0;
	}
	return 0;
}

/// Removes all whitespace characters up until first non-whitespace character.
int String::RemoveInitialWhitespaces(){
	if (arraySize <= 0)
		return -1;
	switch(type){
		case CHAR: {
			int lastWhitespace = -1;
			for (int i = 0; i < arraySize; ++i){
				if (arr[i] == ' ' ||
					arr[i] == '\t' ||
					arr[i] == '\n')
				{
					lastWhitespace = i;
				}
				else
					break;
			}
			if (lastWhitespace != -1){
		//	    std::cout<<"\nString pre Whitespace removal: "<<c_str();
				int whitespaces = lastWhitespace - (-1);
				// Strncpy not safe for using the same string, use memmove for this!
				// http://www.cplusplus.com/reference/cstring/strncpy/
				// http://www.cplusplus.com/reference/cstring/memmove/
				memmove(arr, arr + whitespaces, arraySize - whitespaces);
			//	std::cout<<"\nString post Whitespace removal: "<<c_str();
			}
			return lastWhitespace;
		}
		case WIDE_CHAR:
			assert(false && "implement!");
			return -2;
		default:
			return -1;
	}
}


/// Prints the contents of the string both in integer and character form
void String::PrintData() const{
    std::cout<<"\nArr: "<<arr;
    for (int i = 0; i < Length(); ++i){
        std::cout<<"\nchar "<<i<<": "<<(int)arr[i]<<" (char): "<<arr[i];
    }
}

/// Reads data from file stream
bool String::ReadFrom(std::fstream& file){
	file.read((char*)&type, sizeof(int));
	file.read((char*)&arraySize, sizeof(int));
	assert(arraySize < 1000000);
	if (arraySize > 1000000)
		return false;
	if (arraySize == 0)
		return true;
	/// Allocate!
	Reallocate(arraySize);
	switch(type){
		case String::CHAR:
			file.read((char*) arr, arraySize * sizeof(char));
			break;
		case String::WIDE_CHAR:
			file.read((char*) warr, arraySize * sizeof(wchar_t));
			break;
		default: // Write nothing, maybe assert or inform of waste of space.
			std::cout<<"\nWARNING: No content in string when writing to filestream...";
			break;
	}
	return true;
}
/// Write data to file stream
bool String::WriteTo(std::fstream& file) const{
	file.write((char*)&type, sizeof(int));
	file.write((char*)&arraySize, sizeof(int));
	if (arraySize == 0)
		return true;
	switch(type){
		case String::CHAR:
			file.write((char*) arr, arraySize * sizeof(char));
			break;
		case String::WIDE_CHAR:
			file.write((char*) warr, arraySize * sizeof(wchar_t));
			break;
		default: // Write nothing, maybe assert or inform of waste of space.
		//	std::cout<<"\nWARNING: No content in string when writing to filestream...";
			break;
	}
	return true;
}


/// Calculation functions for array size needed for common procedures (generally string length + 1).
int String::Size(const String & string){
	assert(string.arraySize >= 0);
	return string.Length();
}
int String::Size(const char * c_str){
	return strlen(c_str);
}
int String::Size(const wchar_t * wc_str){
	return wcslen(wc_str);
}

/// Sets all pointers to NULL and all sizes to 0.
void String::Nullify(){
	arr = NULL;
	warr = NULL;
	arraySize = 0;
	type = NULL_TYPE;
	comparisonMode = CASE_SENSITIVE;
  //  testbuff = new char[10];
    testbuff = 0;
   // strcpy(testbuff, "W");
}

/// Deletes and nullifies all arrays.
void String::Delete()
{
	if (arr)
#ifdef USE_BLOCK_ALLOCATOR
		stringAllocator.Deallocate(arr);
#else
		delete[] arr;
#endif
	if (warr)
#ifdef USE_BLOCK_ALLOCATOR
		
#else
		delete[] warr;
#endif
	arr = NULL;
	warr = NULL;
    arraySize = 0;
}

///
void String::Reallocate(int size)
{

//	std::cout<<"\nReallocating with size: "<<size;
	bool debug = false;
	switch(type){
		case CHAR: {
			if (arr && arraySize > 0){
				/// Allocate temp array for holding stuffs if need be
#ifdef USE_BLOCK_ALLOCATOR
				char * tmp = stringAllocator.AllocateNewArray<char>(arraySize);
#else
				char * tmp = new char[arraySize];
#endif
				strcpy(tmp, arr);
				/// And delete the previous array too, yo.
#ifdef USE_BLOCK_ALLOCATOR
				stringAllocator.Deallocate(arr);
				arr = stringAllocator.AllocateNewArray<char>(arraySize);
#else
				delete[] arr;
				arr = new char [size];
#endif
				memset(arr, 0, size);
				strcpy(arr, tmp);
				/// Delete it too, yo.
#ifdef USE_BLOCK_ALLOCATOR
				stringAllocator.Deallocate(tmp);
#else
				delete[] tmp;
#endif
			}
			else {
#ifdef USE_BLOCK_ALLOCATOR
				arr = stringAllocator.AllocateNewArray<char>(size);
#else
				arr = new char [size];
#endif
				memset(arr, 0, size);
				strcpy(arr, "");
			}

			break;
		}
		case WIDE_CHAR: 
		{
			if (arraySize != 0)
			{
				/// Check if we have any contents before the new array is to be built.		
				if (warr){
					wchar_t * tmp = new wchar_t[arraySize];
					wcscpy(tmp, warr);
					delete[] warr;
					warr = new wchar_t[size];
					wcscpy(warr, tmp);
					delete[] tmp;
				}
				else {
					warr = new wchar_t[size];
					wmemset(warr, L'\0', size);
					wcscpy(warr, L"");
				}
			}
			else {
				assert(warr == NULL);
				assert(arr == NULL);
				warr = new wchar_t[size];
			}
			break;
		}
		default: {
			// Allocate both
			if (arr)
#ifdef USE_BLOCK_ALLOCATOR
				stringAllocator.Deallocate(arr);
#else
				delete[] arr;
#endif
			if (warr)
				delete[] warr;
#ifdef USE_BLOCK_ALLOCATOR
			arr = stringAllocator.AllocateNewArray<char>(size);
#else
			arr = new char[size];
#endif
			warr = new wchar_t[size];
			strcpy(arr, "");
			wcscpy(warr, L"");
		//	assert(false && "implement String::Reallocate");
		}
	}
	arraySize = size;
	assert(arraySize > 0);
}

/// Copies data from other string, using method determined by ofType.
void String::Copy(const String & fromTargetString, int ofType){
    int length = fromTargetString.Length();
    assert(length < arraySize);
	switch(ofType){
		case NULL_TYPE:
			return;
			break;
		case CHAR:
			strncpy(arr, fromTargetString.arr, length+1); // +1 so we get the ending null-sign.
			break;
		case WCHAR:
			if (fromTargetString.warr == NULL)
				wcscpy(warr, L"");
			else
				wcscpy(warr, fromTargetString.warr);
			break;
	}
}


/// Concatenation operators, left-hand-string and right-hand-strings respectively
//String operator + (const String & lhs, const char * rhs);	// char*
String operator + (const String & lhs, const String & rhs){
	String string = lhs + rhs;
	return string;
}

String operator + (const String & lhs, const char * rhs){
	String string = lhs + String(rhs);
	return string;
}

String operator + (const char * lhs, const String & rhs){
	String string(lhs);
	string += rhs;
	return string;
}
//String operator + (const String & lhs, const wchar_t * rhs);	// Wide-char string!
//String operator + (const wchar_t * lhs, const String & rhs);

/// Comparison operators/functions
bool operator == (const String & lhs, const String & rhs){
	return lhs.Equals(rhs);
}
bool operator == (const String & lhs, const char * rhs){
	return lhs.Equals(String(rhs));
}
/// Char * first
bool operator == (const char * lhs, const String & rhs){
	return rhs.Equals(String(lhs));
}
/*
	// char*
bool operator == (const String & lhs, const wchar_t * rhs);		// Wide-char string!
bool operator == (const wchar_t * lhs, const String & rhs);
*/

bool operator == (const String & lhs, const int value){
	if (value == 0 && lhs.Length() == 0)
		return true;
	return false;
}

/// Inequality operators
bool operator != (const String & lhs, const String & rhs){
	return ! rhs.Equals(lhs);
}
bool operator != (const char * lhs, const String & rhs){
	return ! rhs.Equals(String(lhs));
}
bool operator != (const String & lhs, const char * rhs){
	return ! lhs.Equals(String(rhs));
}
