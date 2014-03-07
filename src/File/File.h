/// Emil Hedemalm
/// 2013-12-06
/// Class that handles saving to file, but also finding suitable folders for saving preferences, game-saves, etc.

#ifndef FILE_H
#define FILE_H

#include <fstream>
#include "String/AEString.h"

class File {
public:
	void Close();
protected:
	std::fstream * Open(String path);
	const String Path() const;
private:
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
class SaveFile : public File{
public:
	/// Default save folder, usually "sav"
	static const char * saveFolder;
	std::fstream * Open(String saveName, String gameName, String customHeaderData, bool overwriteIfNeeded);
	/// Returns list of all saves
	static List<SaveFileHeader> GetSaves(String gameName);
private:
};

#endif