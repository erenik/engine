// Emil Hedemalm
// 2013-07-10

#include "FileUtil.h"
#include "OS/OS.h"

#ifdef WINDOWS
	#include <windows.h>
#elif defined LINUX | defined OSX
	#include <dirent.h>
	#include <sys/stat.h>
#endif

#include <fstream>

/// For converting 4-byted endianness!
int FourSwap (int i){
  unsigned char b1, b2, b3, b4;
  b1 = i & 255;
  b2 = ( i >> 8 ) & 255;
  b3 = ( i>>16 ) & 255;
  b4 = ( i>>24 ) & 255;
  return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}


/// Returns true if given path exists, false if not. This is mainly intended to be used with folder and directories.
bool PathExists(String path)
{
	List<String> result;
	int exists = GetDirectoriesInDirectory(path, result);
//	std::cout<<"\nPath exists: "<<(exists? "Yes" : "No");
	return exists > 0;
}


/// Returns true if given path exists, false if not.
bool FileExists(String path)
{
	std::fstream f;
	f.open(path.c_str(), std::ios_base::in);
	bool fileExists = false;
	if (f.is_open())
		fileExists = true;
	f.close();
	return fileExists;
}

/// Eased usage function of GetDirectoriesInDirectory
List<String> GetFolders(String inDirectory)
{
	List<String> folders;
	bool result = GetDirectoriesInDirectory(inDirectory, folders);
	assert(result);
	return folders;
}


/** Fetching directories~
	Returns 1 upon success, 0 if there is no such directory.
	Results will be added to the result-list provided.
*/
int GetDirectoriesInDirectory(String directory, List<String> & dirs){
#ifdef WINDOWS
	// http://msdn.microsoft.com/en-us/library/aa364418%28VS.85%29.aspx
	// The directory or path, and the file name, which can include wildcard characters, for example, an asterisk (*) or a question mark (?).
	WIN32_FIND_DATA data;
	if (directory.Type() == String::CHAR)
		directory.ConvertToWideChar();
	if (directory.wc_str()[directory.Length()] != '/')
		directory += '/';
	if (!directory.ContainsChar('*'))
		directory += '*';
	HANDLE findHandle = NULL;
	findHandle = FindFirstFileW(directory.wc_str(), &data);
	if (findHandle == INVALID_HANDLE_VALUE){
		std::cout<<"\nInvalid Handle Value? ";
		int e = GetLastError();
		std::cout<<e;
		if (e== ERROR_FILE_NOT_FOUND){
			std::cout<<"\nERROR: GetFilesInDirectory: No such file or directory \""<<directory<<"\"";
			return 0;
		}
		return 0;
	}

	// lolwat is this..no need to check file info here, yo?
//	BY_HANDLE_FILE_INFORMATION fileInfo;
//	GetFileInformationByHandle(findHandle, &fileInfo);

	while (findHandle != INVALID_HANDLE_VALUE)
	{
		// Get next file..?
		BOOL result = FindNextFile(findHandle, &data);
		/// No more files if fail here.
		if (result == 0)
			break;
		String wName = data.cFileName;
		if (wName == "."  || wName == "..")
			continue;
		if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;
//		std::cout<<"\nFound file: "<<wName;
		dirs.Add(wName);
	}
	// Close ze file..?
	FindClose(findHandle);
	return 1;
#elif defined LINUX | defined OSX
	DIR *dp;
	struct dirent *dirp;
//	std::cout<<"\nTryin to open dir: "<<directory;
//	std::cout<<"\nTryin to open dir: "<<directory;
	int len = directory.Length();
//	std::cout<<"\nlen: "<<len;
//	char * c = directory.c_str();
//	directory.PrintData();
//	std::cout<<"\nTryin to open dir: "<<directory;
//	std::cout<<"\nTryin to open dir: "<<directory;
//	std::cout<<"\nGet directories in dir: "<<directory;
	dp = opendir(directory.c_str());
//	std::cout<<"\nlalll o.o: "<<directory;
//	std::cout<<"\nlalll: "<<directory;
	if (dp == NULL)
	{
//		std::cout<<"Error";
		return 0;
	}	
//	std::cout<<"\nCould open dir.";
	struct stat fileInfo;
	while((dirp = readdir(dp)) != NULL)
	{
//		std::cout<<"\nFilename? "<<dirp->d_name;
		String file = dirp->d_name;
		String fullPath = directory + "/" + dirp->d_name;
		// Check if the "file" is a directory or not.
		lstat(fullPath.c_str(), &fileInfo);
		// Checks for dir, see http://linux.die.net/man/2/lstat
		if (!S_ISDIR(fileInfo.st_mode)){
			continue;
		}
		// Skip special dirs (beginning with dot)
		if (file.c_str()[0] == '.')
		    continue;
	//	std::cout<<"\nFound dir: "<<file;
		dirs.Add(file);
	}
	closedir(dp);
//	std::cout<<"\nGetDirectoriesInDirectory ending.";
	return 1;
#endif
}

/** Getting files in target directory.
	The Result list will contain the names of each file in the directory, not including the directory path.
	Returns 1 upon success, 0 if there is no such directory.
*/
int GetFilesInDirectory(String directory, List<String> & files)
{
	if (directory.Length() == 0)
		directory = ".";
#ifdef WINDOWS
	// http://msdn.microsoft.com/en-us/library/aa364418%28VS.85%29.aspx
	// The directory or path, and the file name, which can include wildcard characters, for example, an asterisk (*) or a question mark (?).
	WIN32_FIND_DATA data;
	if (directory.Type() == String::CHAR)
		directory.ConvertToWideChar();
	if (directory.wc_str()[directory.Length()] != '/')
		directory += '/';
	if (!directory.ContainsChar('*'))
		directory += '*';

	HANDLE findHandle =  FindFirstFile(directory, &data);
	if (findHandle == INVALID_HANDLE_VALUE){
		std::cout<<"\nInvalid Handle Value? ";
		int e = GetLastError();
		std::cout<<e;
		if (e== ERROR_FILE_NOT_FOUND){
			std::cout<<"\nERROR: GetFilesInDirectory: No such file or directory \""<<directory<<"\"";
			return 0;
		}
		return 0;
	}

//	String dirName = data.cFileName;
//	BY_HANDLE_FILE_INFORMATION fileInfo;
//	GetFileInformationByHandle(findHandle, &fileInfo);

	while (findHandle != INVALID_HANDLE_VALUE){
		// Get next file..?
		BOOL result = FindNextFile(findHandle, &data);
		/// No more files if fail here.
		if (result == 0)
			break;
		String wName = data.cFileName;
		if (wName == "."  || wName == "..")
			continue;
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;
	//	std::cout<<"\nFound file: "<<wName;
		files.Add(wName);
	}

	// Close ze file..?
	FindClose(findHandle);
	return 1;
#elif defined LINUX | defined OSX
	DIR *dp;
	struct dirent *dirp;
	std::cout<<"\nDirectory: "<<directory;
	if ((dp = opendir(directory.c_str())) == NULL)
	{
		std::cout<<"Error";
		return 0;
	}
	struct stat fileInfo;
	while((dirp = readdir(dp)) != NULL)
	{
		String file = dirp->d_name;
		// Checks for dir, see http://linux.die.net/man/2/lstat
		String fullPath = directory + "/" + dirp->d_name;
	//	std::cout<<"\nFullpath: "<<fullPath;
		lstat(fullPath.c_str(), &fileInfo);
		if (!S_ISREG(fileInfo.st_mode)){
			continue;
		}
		if (file.c_str()[0] == '.')
			continue;
		files.Add(file);
	}
	closedir(dp);
	return 1;
#endif
}

/// Tries to create a folder. Returns success or not.
bool CreateFolder(String withPath)
{
#ifdef WINDOWS
	String path = withPath;
	path.ConvertToWideChar();
	BOOL result = CreateDirectoryW(path.wc_str(), NULL);
	bool failed = false;
	if (!result){
		int error = GetLastError();
		switch(error){
			case ERROR_ALREADY_EXISTS:
				break;
			case ERROR_PATH_NOT_FOUND:
				failed = true;
				std::cout<<"\nERROR_PATH_NOT_FOUND";
				break;
			default:
				std::cout<<"\nUnknown error: "<<error;
				failed = true;
				break;
		}
	}
#else 
	int result = mkdir(withPath.c_str(), 0777);
	if (result != 0)
	{
		std::cout<<"\nUnable to create directory: "<<withPath;
		return false;
	}

#endif
	return true;	
}

/// Creates directories until the entire path is valid.
bool CreateDirectoriesForPath(String dirPath, bool skipLast)
{
	std::cout<<"\nCreateDirectoriesForPath: "<<dirPath;
	List<String> directories = dirPath.Tokenize("/");
	String entirePath;
	for (int i = 0; i < directories.Size(); ++i)
	{
		std::cout<<"\ni "<<i;
		// p=p
		if (skipLast && i == directories.Size() - 1)
			return true;
		String dir = directories[i];
//		std::cout<<"\ndir "<<dir;
		entirePath += dir+"/";
		/// Check if it contains a colon, if so skip it since it isn't really a folder.
//		std::cout<<"\ncontains :? ";
		if (dir.Contains(":"))
			continue;
//		std::cout<<"\nExists? "<<entirePath;
		if (PathExists(entirePath))
			continue;
		// Create subfolder!
//		std::cout<<"\nCreating subfolder "<<entirePath;
		bool result = CreateFolder(entirePath);
		if (!result)
			return false;

	}
	std::cout<<"\nCreateDirectoriesForPath: "<<dirPath<<" done.";
	return true;
}


/// Builds a path of folders so that the given path can be used. Returns false if it fails to meet the path-required.
bool EnsureFoldersExistForPath(String path)
{
	List<String> folders = path.Tokenize("/");
	/// Remove the last one, assume it's a file at the end.... No.
	// folders.RemoveIndex(folders.Size()-1);
#ifdef WINDOWS
	wchar_t buf[5000];
	GetCurrentDirectoryW(5000, buf);
	String cDir(buf);
	std::cout<<"\nCdir: "<<cDir;
	String previousUrl;
	for (int i = 0; i < folders.Size(); ++i)
	{
		String f = folders[i];
		previousUrl += f;
		std::cout<<"\nChecking if "<<previousUrl<<" exists.";
		const wchar_t * p = previousUrl.wc_str();
		BOOL result = CreateDirectoryW(p, NULL);
		bool failed = false;
		if (!result){
			int error = GetLastError();
			switch(error){
				case ERROR_ALREADY_EXISTS:
					// Good. Go to the next folder?
					break;
				case ERROR_PATH_NOT_FOUND:
					failed = true;
					std::cout<<"\nERROR_PATH_NOT_FOUND";
					break;
				case ERROR_ACCESS_DENIED:
					/// No rights here? Is it a root device identifier?
					if (previousUrl.Length() < 3 && previousUrl.Contains(":"))
						break;
				default:
					std::cout<<"\nUnknown error: "<<error;
					failed = true;
					break;
			}
		}
		if (failed){
			std::cout<<"\nERROR: unable to create folder: "<<previousUrl;
			return false;
		}
		previousUrl += "/";
	}
	return true;
#elif defined LINUX | defined OSX
	bool exists = PathExists(path);
	if (!exists)
	{
		std::cout<<"\nPath "<<path<<" not valid. Creating it.";
		exists = CreateDirectoriesForPath(path);
	}
	return exists;
#endif
}


/// For example "var/bra/da", ".awe", might return "var/bra/da15.awe" if 14 occurences already exist.
String GetFirstFreePath(String pathPreExtension, String extension){
	String path = pathPreExtension + extension;
	std::fstream file;
	file.open(path.c_str(), std::ios_base::in);
	/// Ok path?
	if (!file.is_open())
		return path;
	file.close();
	for (int i = 1; i < 1000; ++i){
		path = pathPreExtension + String::ToString(i) + extension;
		file.open(path.c_str(), std::ios_base::in);
		if (!file.is_open())
			return path;
		file.close();
	}
	assert(false && "FileUtil.cpp, GetFirstFreePath: No valid path found, inform the user");
	return path;
}
