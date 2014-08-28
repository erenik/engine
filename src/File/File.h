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
class File {
public:
	// o.o
	File();
	/// Constructor
	File(String path);
	/// Last time this file was modified. Returns -1 if the file does not exist and -2 if the function fails.
	Time LastModified();
	void Close();

	/// Static function to fetch all contents of a file as if it were just one big string.
	static String GetContents(String fromFile);
	/// Static function to fetch all lines of text from a given file by name. 
	static List<String> GetLines(String fromFile);
	
protected:
	std::fstream * Open(String path);
	const String Path() const;
	
	// Private stuffs
private:

	/// Ensures that the file handle has been opened successfully. Returns false if it fails.
	bool OpenFileHandleIfNeeded();

	// Current file handle state.
	bool open;

#ifdef WINDOWS
	HANDLE fileHandle;	
#endif

	String path;
	std::fstream fileStream;
};

struct SaveFileHeader {
	bool WriteTo(std::fstream & stream);
	bool ReadFrom(std::fstream & stream);
	/// Vars to save.
	String gameName;
	String saveName;
	long long dateSaved;
	String customHeaderData;
	/// Just so we know where to load the rest from.
	String fileName;
};

/// Automatically detects a good folder to save it in.
class SaveFile : public File
{
public:
	/// Default constructor, doesn't open any streams.
	SaveFile();
	/// Default save folder, usually "$Documents$/MyGames/$AppName$/saves/"
	static String saveFolder;
	/** Opens a file stream to the targeted location, writing the a SaveFileHeader at the start, including the custom header data, then returns the stream so that all
		game-related data may be written as wanted.
	*/
	std::fstream * OpenSaveFileStream(String saveName, String gameName, String customHeaderData, bool overwriteIfNeeded);
	/** Opens a file stream to the targeted save game, reading the a SaveFileHeader at the start, including the custom header data, then returns the stream so that all
		game-related data may be written as wanted.
	*/
	std::fstream * OpenLoadFileStream(String saveName, String gameName, String & customHeaderData);

	/// Returns list of all saves, in the form of their SaveFileHeader objects, which should include all info necessary to judge which save to load!
	static List<SaveFileHeader> GetSaves(String gameName);
private:
	// Current file handle state.
	bool open;
	String path;
	std::fstream fileStream;

};

#endif