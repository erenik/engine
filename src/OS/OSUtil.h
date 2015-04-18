/// Emil Hedemalm
/// 2015-01-17
/// Utility functions for OS-specific tasks.

#include "OS.h"

#include "String/AEString.h"

// Windows-specifics
#ifdef WINDOWS
#include <Windows.h>
#include <shellapi.h>

List<String> GetFilesFromHDrop(HDROP hDrop);
#endif // WINDOWS

namespace OSUtil 
{
	void Copy();
	void Paste();
	String GetHomeDirectory();
};

