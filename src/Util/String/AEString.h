/// Emil Hedemalm
/// 2013-03-01

#ifndef GAMEENGINE_STRING_H
#define GAMEENGINE_STRING_H

#include <cstdlib>
#include <cassert>
#include <iostream>
#include "../List/List.h"

#if PLATFORM == PLATFORM_WINDOWS
	//#define snprintf(a,b,c,d) _snprintf(a,b,c,d)
#endif

/// A custom string class that handles dynamic allocation as well as single-/multi-byte conversions as needed.
class String {
public:
	/// Initializes block allocator to be used with strings.
	static void InitializeAllocator();
	String();
	virtual ~String();
	/// Copy constructor and..
	String(const String * string);
	String(const String & string);
	String(const char * string);
	String(const char * from, const char * to); // User-defined length of any given c_str :)
	String(const char c); // Because constructors are awesome.
	String(const int iValue);
	/// -1 will make the float be printed with default amount (as needed). Use String::SCIENTIFIC_NOTATION if that is desired.
	String(const float fValue, int decimalsAfterZeroAndNotation = -1);
	String(const wchar_t * string);

	// For printing floats in various formats
	enum {
		SCIENTIFIC_NOTATION = 16
	};

	// ..assignment operators
	const String& operator = (const String & otherString);
	const String& operator = (const char * otherString);
	const String& operator = (const wchar_t * otherString);
	/// Concatenation assignment operators
	const String& operator += (const String & otherString);
	const String& operator += (const char * otherString);
	const String& operator += (const wchar_t * otherString);

	// Conversion operators
	operator const char * ();
	operator const char * () const;
	operator const wchar_t * () const;
	// What..? seriously?
//	operator const bool () const;

	// Static Parsing functions
	static String ToString(const int value);
	// Uses the format "0xNNNNNNNN".
	static String ToHexString(const int value);
	static String ToString(const float value, int decimalsAfterZero = -1);

	/// For example used with the arguments (lines,"//","/*","*/") for C++
	static List<String> RemoveComments(List<String> lines, String singleLineComment, String multipleLineCommentStart, String multipleLineCommentEnd);

	/// Removing
	String operator - (const String & otherString);
	/// Concatenating
	String operator + (const String & otherString);
	String operator + (const char * otherString);
	String operator + (const char c);
	String operator + (const int value);
	
	/// Returns a substring, from index to index, -1 signifies end of the string.
	String Part(int fromIndex = 0, int toIndex = -1);

	/// Returns the last detected number within.
	String Numberized() const;

	/// For printing it o-o;
	/// Printing out data
	friend std::ostream& operator <<(std::ostream& os, const String& str);

	/// Quering functions
	bool Equals(const String & otherString) const;
	bool ContainsChar(char c) const;
	/// Counts occurences of target character in the string.
	int Count(char c) const;
	bool Contains(const String & subString);
	/// Similar to Contains but works only on the beginning of the string.
	bool StartsWith(const String & subString);
	/// Removes string subpart, returns true if it found and successfully removed it, false if not. Works recursively if all is true.
	bool Remove(const String & subString, bool all = false);
	/// Concatenates strings
	void Add(const String & otherString);
	void Add(const char * otherString);
	/// Char-wise replacement
	void Replace(const char a, const char withB);

	// Extract data o-o
	bool ParseBool();
	int ParseInt();
	float ParseFloat();
	double ParseDouble();

	enum comparisonModes {
		NULL_MODE,
		CASE_SENSITIVE,
		NOT_CASE_SENSITIVE,
	};
	/// Sets the comparison mode to be used for all equals-operations. Note that this may not work with wide chars!
	void SetComparisonMode(int mode) { comparisonMode = mode; };
	/// Returns a number of strings by splitting it's current contents using the provided tokens
	List<String> Tokenize(const char * charTokens) const;
	/// Returns a number of strings by dividing them by '\n' and '\r', which are removed in the process(!).
	List<String> GetLines() const;

	/// Modification functions
	/// Converts all characters to uppercase if possible
	void ToUpper();

	/// Returns the string as a char array pointer.
	const char * c_str();
	const wchar_t * wc_str();
	char At(int index) const;
	/// Reference extractor.
	char & CharAt(int index);
	/// Make the string a wchar_t primary type instead of char.
	void ConvertToWideChar();
	/// Make the string a char primary type instead of wchar_t.
	void ConvertToChar();
	/// Returns it's type..
	int Type() const {return type;};
	enum types {
		NULL_TYPE,
		CHAR,
		WIDE_CHAR, WCHAR = WIDE_CHAR,
	};

	/// Amount of characters (not counting ending NULL-character)
	int Length() const;
	/// Current allocation size of the array. May be used in order to access e.g. the null-character for special parsers.
	int ArraySize() const;

	/// Removes all whitespace characters up until first non-whitespace character.
	int RemoveInitialWhitespaces();
    
    /// Prints the contents of the string both in integer and character form
    void PrintData() const;

	/// Reads data from file stream
	virtual bool ReadFrom(std::fstream& file);
	/// Write data to file stream
	virtual bool WriteTo(std::fstream& file) const;

private:
//    char * testbuff;
	/// Calculation functions for array size needed for common procedures (generally string length + 1).
	static int Size(const String & string);
	static int Size(const char * c_str);
	static int Size(const wchar_t * wc_str);

	/// Sets all pointers to NULL and all sizes to 0.
	void Nullify();
	/// Deletes and nullifies all arrays.
	void Delete();
	/// Allocates a larger array and copies over the previous data.
	void Reallocate(int size);
	/// Copies data from other string, using method determined by ofType.
	void Copy(const String & fromTargetString, int ofType);

	/// Regular char array
	char * arr;
	/// Wide-char array
	wchar_t * warr;
	/// Current size of string. Deprecated as it is no good use at the moment.
//	int size;
	/// Current max array size
	int arraySize;
	/// If the string is now multibyte (any adding operation that adds a wchar_t), or otherwise
	int type;
	/// Mode used for == operations
	char comparisonMode;
};

/// Concatenation operators, left-hand-string and right-hand-strings respectively
String operator + (const String & lhs, const String & rhs);
String operator + (const String & lhs, const char * rhs);	// char*
String operator + (const char * lhs, const String & rhs);
String operator + (const String & lhs, const wchar_t * rhs);	// Wide-char string!
String operator + (const wchar_t * lhs, const String & rhs);
/// Comparison operators/functions
bool operator == (const String & lhs, const String & rhs);
bool operator == (const String & lhs, const char * rhs);	// char*
bool operator == (const char * lhs, const String & rhs);
bool operator == (const String & lhs, const wchar_t * rhs);		// Wide-char string!
bool operator == (const wchar_t * lhs, const String & rhs);
bool operator == (const String & lhs, const int value);
/// Inequality operators
bool operator != (const String & lhs, const String & rhs);
bool operator != (const char * lhs, const String & rhs);
bool operator != (const String & lhs, const char * rhs);


#endif
