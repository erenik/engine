/// Emil Hedemalm
/// 2013-12-06
/// Class that handles saving to file, but also finding suitable folders for saving preferences, game-saves, etc.

#include "File.h"
#include "FileUtil.h"
#include "Timer/Timer.h"

std::fstream * File::Open(String path){
	/// Add /save/ unless it already exists in the path.
	fileStream.open(path.c_str(), std::ios_base::out | std::ios_base::binary);
	bool success = fileStream.is_open();
	if (!success){
		/// Try another path?
		return NULL;
	}
	return &fileStream;
}

void File::Close(){
	fileStream.close();
}

const String File::Path() const {
	return path;
}

/// SaveFile
String SaveFile::saveFolder = "sav";
std::fstream * SaveFile::Open(String saveName, String gameName, String customHeaderData, bool overwriteIfNeeded){
	String path = saveFolder + "/" + gameName + "/" + saveName;
	String pathPreExtension = path;
	String extension = ".sav";
	if (!path.Contains(extension))
		path += extension;
	EnsureFoldersExistForPath(path);
	/// Check if it already exists.
	path = GetFirstFreePath(pathPreExtension, extension);
	std::fstream * stream = File::Open(path);
	SaveFileHeader header;
	header.gameName = gameName;
	header.saveName = saveName;
	header.dateSaved = Timer::GetCurrentTimeMs();
	header.customHeaderData = customHeaderData;
	if (stream){
		header.WriteTo(*stream);
	}
	return stream;
}

/// Returns list of all saves
List<SaveFileHeader> SaveFile::GetSaves(String gameName){
	String path = saveFolder + "/" + gameName + "/";
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


String versionHeader = "Aeonic Game Engine - By: Emil Hedemalm/Erenik\nSave File.";
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