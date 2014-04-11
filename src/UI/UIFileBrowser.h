/// Emil Hedemalm
/// 2014-01-14
/// A file-browser extention of the UI-classes.

#ifndef UI_FILE_BROWSER_H
#define UI_FILE_BROWSER_H

#include "UIInput.h"
#include "UIList.h"

#define UIFileDialogue UIFileBrowser

class UIFileBrowser : public UIList {
public:
	UIFileBrowser(String title, String action, String fileFilter);
	virtual ~UIFileBrowser();
	/// Creates ze children!
	void CreateChildren();
	/// Clears and creates new list of directory-contents. Use false on first usage, rest should be true.
	void LoadDirectory(bool fromRenderThread);
	/// Call to update path, using given argument to add to the path.
	void UpdatePath(String cat);
	/// Sets full path.
	void SetPath(String path);
	/// Sets file in the input-field for later evaluation.
	void SetActiveFile(String file);
	/// Returns the list of currently selected files in the browser.
	List<String> GetFileSelection();
	
	/// Action to be taken on files once OK is pressed. Will be set to the msg parameter of the FileEvent message that is generated.
	String action;
	
	/// Woo.
	String CurrentPath() {return currentPath;};

private:
	// Update stuff.
	void OnDirPathUpdated();

	/// Old file browsers and their paths, using "name;path" syntax
	static List<String> oldFileBrowsers;

	UIInput * fileInput;
	UIInput * dirInput;
	UIList * dirList;
	String title;
	String fileFilter;
	String currentPath;

	/// List of all selected files. This will most often only include one file, but may contain more.
	List<String> fileSelection;
};

#endif