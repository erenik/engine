/// Emil Hedemalm
/// 2013-12-06
/// Class that handles saving to file, but also finding suitable folders for saving preferences, game-saves, etc.

#ifndef FILE_H
#define FILE_H

#include <fstream>
#include "String/AEString.h"
#include "OS/OS.h"
#include "System/DataTypes.h"
#include "Time/Time.h"

// Perform windows-specific includes straight-away!
#if defined WINDOWS
	#include "OS/WindowsIncludes.h"
#endif


/// General file-class?
class File 
{
	File(const File & otherFile);
public:
	// o.o
	File();
	/// Constructor
	File(String path);
	/// Frees file handles.
	virtual ~File();
	// Resets file handles etc.
	void Nullify();

	/// Assignment operator. Similar to constructor.
	void operator = (String path);

	/// Last time this file was modified. Returns false if the file could not be accessed.
	bool LastModified(Time & lastModifiedDate);
	
	/// Sets path, closing any previously held file streams to the file at the previous path.
	void SetPath(String filePath);

	std::fstream * Open();
	void Close();
	bool IsOpen();

	/// Returns true if the last write time has changed compared to the last time that we extracted contents from this file.
	bool HasChanged();

	/// Fetches contents of this file.
	String GetContents();
	/// Static function to fetch all contents of a file as if it were just one big string.
	static String GetContents(String fromFile);
	/// Fetches contents from file in form of lines. (\r\n removed and used as tokenizers)
	List<String> GetLines();
	/// Static function to fetch all lines of text from a given file by name. 
	static List<String> GetLines(String fromFile);
	
	/// Reads through the entire file to the end, printing every character along the way in std::cout
	void PrintData();

	/// Contains the LastModified time when we last accessed this file.
	Time editTimeWhenReadLast;

	const String Path() const;

protected:
	/// Fetches contents from target stream.
	static String GetContents(std::fstream & fromFileStream);
	
	// The path to this file.
	String path;

	// Private stuffs
protected:

	/// Ensures that the file handle has been opened successfully. Returns false if it fails.
	bool OpenFileHandleIfNeeded();

	// Current file handle state.
	bool open;

#ifdef WINDOWS
	HANDLE fileHandle;	
#endif

	std::fstream fileStream;
};

#endif