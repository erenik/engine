/// Emil Hedemalm
/// 2013-12-06
/// Class that handles saving to file, but also finding suitable folders for saving preferences, game-saves, etc.

#include "File.h"
#include "FileUtil.h"
#include "Timer/Timer.h"
#include <cstring>
#include <ios>
#include "DataStream/DataStream.h"

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
#ifdef WINDOWS
	fileHandle = 0;
#endif
	open = false;
}

/// Reads x bytes into the file, continuing from previous location (or start if newly opened).
bool File::ReadBytes(DataStream & intoStream, int numBytes)
{
	if (!this->IsOpen())
		return false;
	if (numBytes <= 0)
		return false;
	uchar * bytes = intoStream.GetData();
	this->fileStream.read((char*)bytes, numBytes);
	intoStream.SetBytesUsed(numBytes);
	return true;
}


/// Static version which reads x bytes.
bool File::ReadBytes(String fromFile, DataStream & intoStream, int numBytes)
{
	File f;
	f.path = fromFile;
	bool ok = f.Open();
	if (!ok)
		return false;
	return f.ReadBytes(intoStream, numBytes);
}

File::~File()
{
	Close();
}

/// Assignment operator. Similar to constructor.
void File::operator = (String assignedPath)
{
	Nullify();
	this->path = assignedPath;
}

/// Last time this file was modified. Returns -1 if the file does not exist and -2 if the function fails.
bool File::LastModified(Time & lastModifiedTime)
{
#ifdef WINDOWS
	if (!OpenFileHandleIfNeeded())
	{
		return false;
	}

	FILETIME creationTime, lastAccessTime, lastWriteTime;
	BOOL result = GetFileTime(fileHandle, &creationTime, &lastAccessTime, &lastWriteTime);
	if (!result)
	{
		int error = GetLastError();
		assert(error && false);
	}
	
	ULARGE_INTEGER uli;
	uli.HighPart = lastWriteTime.dwHighDateTime;
	uli.LowPart = lastWriteTime.dwLowDateTime;
	lastModifiedTime = Time(TimeType::WIN32_100NANOSEC_SINCE_JAN1_1601, uli.QuadPart);
#endif
	return true;
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
	editTimeWhenReadLast;
	assert(LastModified(editTimeWhenReadLast));
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
	assert(LastModified(editTimeWhenReadLast));
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

/// Reads through the entire file to the end, printing every character along the way in std::cout
void File::PrintData()
{
	/// If not opened, open it.
	if (!fileStream.is_open())
	{
		Open();
	}
	while(true)
	{
		char c;
		int charSize = sizeof(char);
		assert(charSize == 1);
		fileStream.read((char*)&c, sizeof(char));
		std::cout<<"\nchar "<<fileStream.tellg()<<": (int) "<<(int)c<<" (char) "<<c;
		std::ios_base::iostate rdstate = fileStream.rdstate();
		std::ios_base::iostate state = fileStream.rdstate();
		if (fileStream.eof())
		{
			std::cout<<"\nAt end of file.";
			break;
		}
		else if (fileStream.fail() || fileStream.bad())
		{
			std::cout<<"\nOther bad bit flagged.";
			break;
		}
	}
}

void File::Close()
{
#ifdef WINDOWS
	if (fileHandle)
		CloseHandle(fileHandle);
	fileHandle = NULL;
#endif
	if (fileStream.is_open())
		fileStream.close();
}

bool File::IsOpen()
{
#ifdef WINDOWS
	if (fileHandle)
		return true;
#endif
	if (fileStream.is_open())
		return true;
	return false;
}

/// Returns true if the last write time has changed compared to the last time that we extracted contents from this file.
bool File::HasChanged()
{
	Time lastEdit;
	assert(LastModified(lastEdit));
	if (lastEdit == editTimeWhenReadLast)
		return false;
	return true;
}


const String File::Path() const {
	return path;
}

