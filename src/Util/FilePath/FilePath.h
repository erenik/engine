// Emil Hedemalm
// 2013-03-20


#ifndef FILEPATH_H 
#define FILEPATH_H 

#include "String/AEString.h"

namespace PathType {
enum {
	NULL_TYPE,
	ABSOLUTE_PATH,
	RELATIVE_PATH
};};

class FilePath {
public:
	FilePath();
	FilePath(String path);

	static String workingDirectory;

	/// Creates a relative path using given absolute path.
	static String MakeRelative(String absolutePath);
	static String MakeAbsolute();
	/// Checks if the string contains certain substrings used by the OS
	static bool IsAbsolutePath(String pathToCheck);
	
	/// Returns the last parts after the dots, pretty much.
	static String FileEnding(String path);

	/// Returns the string component from the last slash and onwards.
	static String GetFileName(String filePath);

	/// Gets the relative path
	String RelativePath() { return relativePath; };
private:
	

	String absolutePath;
	String relativePath;

	/// Checks the path type and sets pathType afterward
	void CheckType();

	int pathType;
};


#endif
