/// Emil Hedemalm
/// 2015-01-17
/// Utility functions for OS-specific tasks.

#include "OS.h"

#ifdef WINDOWS

#include <Windows.h>
#include <shellapi.h>
#include "String/AEString.h"

List<String> GetFilesFromHDrop(HDROP hDrop);

namespace OSUtil 
{
	void Copy();
	void Paste();
	String GetHomeDirectory();
};

#endif

