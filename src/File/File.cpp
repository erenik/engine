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

/// Static function to fetch all lines of text from a given file. 
List<String> File::GetLines(String fromFile)
{
	List<String> lines;
	std::fstream file;
	file.open(fromFile.c_str(), std::ios_base::in | std::ios_base::binary);
	if (file.is_open())
	{	
		int start  = (int) file.tellg();
		file.seekg( 0, std::ios::end );
		int fileSize = (int) file.tellg();
		char * data = new char [fileSize+2];
		memset(data, 0, fileSize+1);
		file.seekg( 0, std::ios::beg);
		file.read((char*) data, fileSize);
		file.close();
		String fileContents(data);
		lines = fileContents.GetLines();
	}
	else 
	{
		file.close();
		std::cout<<"\nFile::GetLines: Unable to open file: "<<fromFile;
	}
	return lines;
}

void File::Close(){
	fileStream.close();
}

const String File::Path() const {
	return path;
}

/// SaveFile
const char * SaveFile::saveFolder = "sav";

std::fstream * SaveFile::Open(String saveName, String gameName, String customHeaderData, bool overwriteIfNeeded)
{
	String path = String(saveFolder) + "/" + gameName + "/" + saveName;
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