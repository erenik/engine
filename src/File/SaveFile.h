/// Emil Hedemalm
/// 2015-01-28
/// Used for helping setup the saving- and loading-procedures for players' gaming sessions.
/// Default save folder, usually "$Documents$/MyGames/$AppName$/saves/", set in SaveFile::saveFolder.

#ifndef SAVE_FILE_H
#define SAVE_FILE_H

#include "File.h"

struct SaveFileHeader {
	bool WriteTo(std::fstream & stream);
	bool ReadFrom(std::fstream & stream);
	/// Vars to save.
	String gameName;
	String saveName;
	Time dateSaved;
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
	// File destructor closest the file-stream automatically?
	/// Default save folder, usually "$Documents$/MyGames/$AppName$/saves/"
	static String saveFolder;
	/** Opens a file stream to the targeted location, writing the a SaveFileHeader at the start, including the custom header data, then returns the stream so that all
		game-related data may be written as wanted.
	*/
	bool OpenSaveFileStream(String saveName, String gameName, String customHeaderData, bool overwriteIfNeeded);
	/** Opens a file stream to the targeted save game, reading the a SaveFileHeader at the start, including the custom header data, then returns the stream so that all
		game-related data may be written as wanted.
	*/
	bool OpenLoadFileStream(String saveName, String gameName, String & customHeaderData);
	/// Gets the stream. Should be called after having called OpenSaveFileStream or OpenLoadFileStream sucessfully.
	std::fstream & GetStream();
	/// Returns the header as it was saved or loaded.
	const SaveFileHeader & GetHeader();

	/// Returns list of all saves, in the form of their SaveFileHeader objects, which should include all info necessary to judge which save to load!
	static List<SaveFileHeader> GetSaves(String gameName);
	String lastError;
private:
	// Current file handle state.
	bool open;
	String path;
};



#endif
