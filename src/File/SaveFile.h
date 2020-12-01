/// Emil Hedemalm
/// 2015-01-28
/// Used for helping setup the saving- and loading-procedures for players' gaming sessions.
/// Default save folder, usually "$Documents$/MyGames/$AppName$/saves/", set in SaveFile::saveFolder.

#ifndef SAVE_FILE_H
#define SAVE_FILE_H

#include "File.h"

class GameVariable;

struct SaveFileHeader {
	bool WriteTo(std::fstream & stream);
	bool ReadFrom(std::fstream & stream);
	/// Vars to save.
	String gameName;
	String saveName;
	Time dateSaved;
	/// Informative text to be parsed or displayed straight to the user. All data here should be saved/loaded in the stream as well!
	String customHeaderData;
	/// Just so we know where to load the rest from.
	String fileName;
};

/// Automatically detects a good folder to save it in.
class SaveFile : public File
{
public:
	/// Default constructor, doesn't open any streams.
	SaveFile(String gameName, String saveName);
	// File destructor closest the file-stream automatically?
	/** Opens a file stream to the targeted location, writing the a SaveFileHeader at the start, including the custom header data, then returns the stream so that all
		game-related data may be written as wanted. 
		NOTE: The custom header-data is only used for when browsing the saves and should not be used for actual data.
	*/
	bool OpenSaveFileStream(String customHeaderData, bool overwriteIfNeeded);
	/** Opens a file stream to the targeted save game, reading the a SaveFileHeader at the start, including the custom header data, then returns the stream so that all
		game-related data may be written as wanted.
		Header-data is set in headerData member variable upon loading.
	*/
	bool OpenLoadFileStream();

	bool SaveVars(List<GameVariable*> gameVars);
	bool LoadVars();

	/// Gets the stream. Should be called after having called OpenSaveFileStream or OpenLoadFileStream sucessfully.
	std::fstream & GetStream();
	/// Returns the header as it was saved or loaded.
	const SaveFileHeader & GetHeader();


	/// By default saves all GameVars.
	static bool AutoSave(String gameName, String customHeaderData);
	static bool LoadAutoSave(String gameName);

	bool Load();

	/// Default save folder, usually "$Documents$/MyGames/$AppName$/saves/"
	static String FolderPath(String gameName);

	/// Returns list of all saves, in the form of their SaveFileHeader objects, which should include all info necessary to judge which save to load!
	static List<SaveFileHeader> GetSaves(String gameName);
	String lastError;
	/// Header data, as loaded when calling OpenLoadFileStream.
	SaveFileHeader headerData;
	/// Builds path to the save-file.
	void BuildPath();
private:

	String gameName;
	String saveName;
	// Current file handle state.
	bool open;
};



#endif
