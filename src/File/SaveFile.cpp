/// Emil Hedemalm
/// 2015-01-28
/// Used for helping setup the saving- and loading-procedures for players' gaming sessions.

#include "SaveFile.h"
#include "FileUtil.h"
#include "Timer/Timer.h"
#include "OS/OSUtil.h"

/// SaveFile
/// Default save folder, usually "$Documents$/MyGames/$AppName$/saves/"
String SaveFile::saveFolder = "saves";

/// Default constructor, doesn't open any streams.
SaveFile::SaveFile(String gameName, String saveName)
: gameName(gameName), saveName(saveName)
{

}

/// Header currently being used.
SaveFileHeader header;


void SaveFile::BuildPath()
{
	// Set folder to use for saves.
	String homeFolder = OSUtil::GetHomeDirectory();
	homeFolder.Replace('\\', '/');
	SaveFile::saveFolder = homeFolder;

	// Ensure folder exists.
	String folderPath = String(saveFolder) + "/" + gameName + "/";
	EnsureFoldersExistForPath(folderPath);

	// Update path.
	path = folderPath + saveName;
	String pathPreExtension = path;
	String extension = ".sav";
	if (!path.Contains(extension))
		path += extension;
	/// Check if it already exists. Overwrite!
//	path = GetFirstFreePath(pathPreExtension, extension);
}

bool SaveFile::OpenSaveFileStream(String customHeaderData, bool overwriteIfNeeded)
{
	// Close old file stream.
	if (fileStream.is_open())
		fileStream.close();


	// Builds the path to the save-file to be used/loaded from.
	BuildPath();

	// Open file stream
	fileStream.open(path.c_str(), std::ios_base::out | std::ios_base::binary);
	if (!fileStream.is_open())
	{
		lastError = "Unable to open file-stream to location: "+path;
		return NULL;
	}
	// Write header.
	header.gameName = gameName;
	header.saveName = saveName;
	header.dateSaved = Time::Now();
	header.customHeaderData = customHeaderData;
	header.WriteTo(fileStream);
	return true;
}

/** Opens a file stream to the targeted save game, reading the a SaveFileHeader at the start, including the custom header data, then returns the stream so that all
	game-related data may be written as wanted.
*/
bool SaveFile::OpenLoadFileStream()
{
	// Close old file stream.
	if (fileStream.is_open())
		fileStream.close();

	// Builds the path to the save-file to be used/loaded from.
	BuildPath();

	// Open file stream
	fileStream.open(path.c_str(), std::ios_base::in | std::ios_base::binary);
	if (!fileStream.is_open())
	{
		lastError = "Unable to open file-stream to location: "+path;
		return false;
	}
	// Read header.
	if (!headerData.ReadFrom(fileStream))
	{
		lastError = "Unable to read header-data. D:";
		return false;
	}
	// Stream should now be ready to be read further by the application at hand.
	return true;
}


/// Returns list of all saves
List<SaveFileHeader> SaveFile::GetSaves(String gameName)
{
	String path = String(saveFolder) + "/" + gameName + "/";
	List<SaveFileHeader> saveHeaders;
	List<String> files;
	bool worked = GetFilesInDirectory(path, files);
	if (!worked){
		std::cout<<"\nUnable to locate any files in path: "<<path;
		return saveHeaders;
	}
	for (int i = 0; i < files.Size(); ++i){
		String fileURL = path + files[i];
		std::fstream file;
		file.open(fileURL.c_str(), std::ios_base::in | std::ios_base::binary);
		if (!file.is_open()){
			file.close();
			continue;
		}
		SaveFileHeader sfh;
		bool success = sfh.ReadFrom(file);
		sfh.fileName = fileURL;
		if (!sfh.gameName.Equals(gameName))
			success = false;
		if (success){
			saveHeaders.Add(sfh);
		}
		file.close();
	}
	return saveHeaders;
}

std::fstream & SaveFile::GetStream()
{
	return fileStream;
}

/// Returns the header as it was saved or loaded.
const SaveFileHeader & SaveFile::GetHeader()
{
	return header;
}


String versionHeader = "Aeonic Game Engine - By: Emil Hedemalm/Erenik\nSave File.\n";

bool SaveFileHeader::WriteTo(std::fstream & stream){
	versionHeader.WriteTo(stream);
	gameName.WriteTo(stream);
	saveName.WriteTo(stream);
	dateSaved.WriteTo(stream);
	customHeaderData.WriteTo(stream);
	return true;
}
bool SaveFileHeader::ReadFrom(std::fstream & stream){
	String header;
	if (!header.ReadFrom(stream)) 
		return false;
	gameName.ReadFrom(stream);
	saveName.ReadFrom(stream);
	dateSaved.ReadFrom(stream);
	customHeaderData.ReadFrom(stream);
	return true;
}

