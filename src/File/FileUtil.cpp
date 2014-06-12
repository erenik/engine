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
bool PathExists(String path){
	List<String> result;
	int exists = GetDirectoriesInDirectory(path, result);
	return exists > 0;
}


/// Returns true if given path exists, false if not.
bool FileExists(String path){
	std::fstream f;
	f.open(path.c_str(), std::ios_base::in);
	bool fileExists = false;
	if (f.is_open())
		fileExists = true;
	f.close();
	return fileExists;
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

	String dirName = data.cFileName;
	BY_HANDLE_FILE_INFORMATION fileInfo;
	GetFileInformationByHandle(findHandle, &fileInfo);

	while (findHandle != INVALID_HANDLE_VALUE){
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
	std::cout<<"\nDirectory: "<<directory;
	if ((dp = opendir(directory.c_str())) == NULL){
		std::cout<<"Error";
		return 0;
	}
	struct stat fileInfo;
	while((dirp = readdir(dp)) != NULL){
		std::cout<<"\nFilename? "<<dirp->d_name;
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
		dirs.Add(file);
	}
	closedir(dp);
	return 1;
#endif
}

/** Getting directories~
	Returns 1 upon success, 0 if there is no such directory.
	Results will be added to the result-list provided.
*/
int GetFilesInDirectory(String directory, List<String> & files){
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

	String dirName = data.cFileName;
	BY_HANDLE_FILE_INFORMATION fileInfo;
	GetFileInformationByHandle(findHandle, &fileInfo);

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
	if ((dp = opendir(directory.c_str())) == NULL){
		std::cout<<"Error";
		return 0;
	}
	struct stat fileInfo;
	while((dirp = readdir(dp)) != NULL){
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
#endif
	return true;	
}

/// Creates directories until the entire path is valid.
bool CreateDirectoriesForPath(String dirPath)
{
	List<String> directories = dirPath.Tokenize("/");
	String entirePath;
	for (int i = 0; i < directories.Size(); ++i)
	{

		String dir = directories[i];
		entirePath += dir+"/";
		/// Check if it contains a colon, if so skip it since it isn't really a folder.
		if (dir.Contains(":"))
			continue;
		if (PathExists(entirePath))
			continue;
		// Create subfolder!
		bool result = CreateFolder(entirePath);
		if (!result)
			return false;

	}
	return true;
}


/// Builds a path of folders so that the given path can be used. Returns false if it fails to meet the path-required.
bool EnsureFoldersExistForPath(String path){
	List<String> folders = path.Tokenize("/");
	/// Remove the last one, assume it's a file at the end.... No.
	// folders.RemoveIndex(folders.Size()-1);
#ifdef WINDOWS
	wchar_t buf[5000];
	GetCurrentDirectoryW(5000, buf);
	String cDir(buf);
	std::cout<<"\nCdir: "<<cDir;
	String previousUrl;
	for (int i = 0; i < folders.Size(); ++i){
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
	assert(false);
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
