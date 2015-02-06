/// Emil Hedemalm
/// 2013-12-06
/// Class that handles saving to file, but also finding suitable folders for saving preferences, game-saves, etc.

#include "File.h"
#include "FileUtil.h"
#include "Timer/Timer.h"

// o.o
File::File()
{
	Nullify();
}

// Constructor
File::File(String cPath)
{
	Nullify();
	this->path = cPath;
}

// Resets file handles etc.
void File::Nullify()
{
	fileHandle = 0;
	open = false;
}

File::~File()
{
	if (fileHandle)
		CloseHandle(fileHandle);
	fileHandle = 0;
	if (fileStream.is_open())
		fileStream.close();
}

/// Assignment operator. Similar to constructor.
void File::operator = (String assignedPath)
{
	Nullify();
	this->path = assignedPath;
}

/// Last time this file was modified. Returns -1 if the file does not exist and -2 if the function fails.
Time File::LastModified()
{
#ifdef WINDOWS
	if (!OpenFileHandleIfNeeded())
	{
		return -1;
	}

	FILETIME creationTime, lastAccessTime, lastWriteTime;
	bool result = GetFileTime(fileHandle, &creationTime, &lastAccessTime, &lastWriteTime);
	if (!result)
	{
		int error = GetLastError();
		assert(error && false);
	}
	
	ULARGE_INTEGER uli;
	uli.HighPart = lastWriteTime.dwHighDateTime;
	uli.LowPart = lastWriteTime.dwLowDateTime;
	Time lastModified(uli.QuadPart, TimeType::WIN32_100NANOSEC_SINCE_JAN1_1601);
	return lastModified;
#endif
}

/// Sets path.
void File::SetPath(String filePath)
{
	// Close previous file streams and handles if any.
	Close();
	path = filePath;
	editTimeWhenReadLast = Time();
}


/// Ensures that the file handle has been opened successfully. Returns false if it fails.
bool File::OpenFileHandleIfNeeded()
{
#ifdef WINDOWS
	// http://msdn.microsoft.com/en-us/library/windows/desktop/aa363858%28v=vs.85%29.aspx
	fileHandle = CreateFile(path.wc_str(),  GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle ==  INVALID_HANDLE_VALUE)
	{
		int error = GetLastError();
		if (error = ERROR_FILE_NOT_FOUND)
			std::cout<<"\nError: File not found: "<<path;
		return false;
	}
#endif
	return true;
}



std::fstream * File::Open()
{
	/// Add /save/ unless it already exists in the path.
	fileStream.open(path.c_str(), std::ios_base::out | std::ios_base::in | std::ios_base::binary);
	bool success = fileStream.is_open();
	if (!success){
		/// Try another path?
		return NULL;
	}	
	return &fileStream;
}

/// Fetches contents of this file.
String File::GetContents()
{
	/// Contains the LastModified time when we last accessed this file.
	editTimeWhenReadLast = LastModified();
	return GetContents(path);
}

/// Static function to fetch all contents of a file as if it were just one big string.
String File::GetContents(String fromFile)
{
	String fileContents;
	std::fstream file;
	file.open(fromFile.c_str(), std::ios_base::in | std::ios_base::binary);
	if (file.is_open())
	{	
		fileContents = GetContents(file);
	}
	else 
	{
		std::cout<<"\nFile::GetLines: Unable to open file: "<<fromFile;
	}
	file.close();
	return fileContents;
}

String File::GetContents(std::fstream & fileStream)
{
	assert(fileStream.is_open());
	fileStream.seekg( 0, std::ios::beg);
	int start  = (int) fileStream.tellg();
	fileStream.seekg( 0, std::ios::end );
	int fileSize = (int) fileStream.tellg();
	// Empty file?
	if (fileSize == 0)
		return String();
	assert(fileSize);
	char * data = new char [fileSize+2];
	memset(data, 0, fileSize+1);
	fileStream.seekg( 0, std::ios::beg);
	fileStream.read((char*) data, fileSize);
	String strData(data);
	delete[] data;
	return strData;
}

/// Fetches contents from file in form of lines. (\r\n removed and used as tokenizers)
List<String> File::GetLines()
{
	String contents = GetContents(path);
	editTimeWhenReadLast = LastModified();
	List<String> lines = contents.GetLines();
	return lines;
}


/// Static function to fetch all lines of text from a given file. 
List<String> File::GetLines(String fromFile)
{
	String fileContents = GetContents(fromFile);
	List<String> lines = fileContents.GetLines();
	return lines;
}

void File::Close()
{
	if (fileHandle)
		CloseHandle(fileHandle);
	fileHandle = NULL;
	fileStream.close();
}

bool File::IsOpen()
{
	if (fileHandle)
		return true;
	else if (fileStream.is_open())
		return true;
	return false;
}

/// Returns true if the last write time has changed compared to the last time that we extracted contents from this file.
bool File::HasChanged()
{
	Time lastEdit = LastModified();
	if (lastEdit == editTimeWhenReadLast)
		return false;
	return true;
}


const String File::Path() const {
	return path;
}

