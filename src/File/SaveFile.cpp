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
SaveFile::SaveFile()
{

}

/// Header currently being used.
SaveFileHeader header;
	
bool SaveFile::OpenSaveFileStream(String saveName, String gameName, String customHeaderData, bool overwriteIfNeeded)
{
	// Close old file stream.
	fileStream.close();

	// Ensure folder exists.
	String folderPath = String(saveFolder) + "/" + gameName + "/";
	EnsureFoldersExistForPath(folderPath);

	// Update path.
	path = folderPath + saveName;
	String pathPreExtension = path;
	String extension = ".sav";
	if (!path.Contains(extension))
		path += extension;
	/// Check if it already exists.
	path = GetFirstFreePath(pathPreExtension, extension);

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
	header.dateSaved = Timer::GetCurrentTimeMs();
	header.customHeaderData = customHeaderData;
	header.WriteTo(fileStream);
	return &fileStream;
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


String versionHeader; // = "Aeonic Game Engine - By: Emil Hedemalm/Erenik\nSave File.";

bool SaveFileHeader::WriteTo(std::fstream & stream){
	versionHeader.WriteTo(stream);
	gameName.WriteTo(stream);
	saveName.WriteTo(stream);
	int size = sizeof(long long);
	stream.write((char*) &dateSaved, sizeof(long long));
	customHeaderData.WriteTo(stream);
	return true;
}
bool SaveFileHeader::ReadFrom(std::fstream & stream){
	String header;
	header.ReadFrom(stream);
	gameName.ReadFrom(stream);
	saveName.ReadFrom(stream);
	stream.read((char*) &dateSaved, sizeof(long long));
	customHeaderData.ReadFrom(stream);
	return true;
}

