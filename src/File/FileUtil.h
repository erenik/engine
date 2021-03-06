// Emil Hedemalm
// 2013-07-10

#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <String/AEString.h>

#define SWAPFOUR(c) {c = FourSwap(c);}

/// For converting 4-byted endianness!
int FourSwap (int i);

/// Returns true if given path exists, false if not. This is mainly intended to be used with folder and directories.
bool PathExists(String path);
/// Returns true if given path exists, false if not.
bool FileExists(String path);

/// Eased usage function of GetDirectoriesInDirectory
List<String> GetFolders(String inDirectory);

/** Fetching directories~
	Returns 1 upon success, 0 if there is no such directory.
	Results will be added to the result-list provided.
*/
int GetDirectoriesInDirectory(String directory, List<String> & result);
/** Getting files in target directory.
	The Result list will contain the names of each file in the directory, not including the directory path.
	Returns 1 upon success, 0 if there is no such directory.
*/
int GetFilesInDirectory(String directory, List<String> & result);
/** Gets list of file-paths (paths for each file) in target directory. The result list will contain 1 path/file per string.
	Returns 1 upon success, 0 if there is no such directory.
*/
int GetFilePathsInDirectory(String directory, List<String> & result);

/// Tries to create a folder. Returns success or not.
bool CreateFolder(String withPath);
/// Creates directories until the entire path is valid.
bool CreateDirectoriesForPath(String dirPath, bool skipLast = false);	
/// Builds a path of folders so that the given path can be used. Returns false if it fails to meet the path-required. NOTE: Only works with relative directories!
bool EnsureFoldersExistForPath(String path);

/// For example "var/bra/da", ".awe", might return "var/bra/da15.awe" if 14 occurences already exist.
String GetFirstFreePath(String pathPreExtension, String extension);

#endif
