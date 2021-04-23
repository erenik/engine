/// Emil Hedemalm
/// 2014-01-14
/// A file-browser extention of the UI-classes.
/**
	Example usage of the file browser is as follows:

	UIFileBrowser * browser = new UIFileBrowser(title, action, filter);
	browser->
*/

#ifndef UI_FILE_BROWSER_H
#define UI_FILE_BROWSER_H

#include "UILists.h"

class UIInput;

#define UIFileDialogue UIFileBrowser

/// File browser class.
class UIFileBrowser : public UIList {
public:
	UIFileBrowser(String title, String action, String fileFilter);
	virtual ~UIFileBrowser();
	/// Creates ze children!
	void CreateChildren(GraphicsState* graphicsState) override;
	/// Clears and creates new list of directory-contents. Use false on first usage, rest should be true.
	void LoadDirectory(bool fromRenderThread);
	/// Call to update path, using given argument to add to the path.
	void UpdatePath(String cat, bool fromRenderThread);
	/// Sets browser active directory path.
	void SetPath(String path, bool fromRenderThread);
	/// Sets file in the input-field for later evaluation.
	void SetActiveFile(GraphicsState* graphicsState, String file);

	void SetActiveFileFromInput(GraphicsState* graphicsState);

	/// Returns the list of currently selected files in the browser.
	List<String> GetFileSelection();

	void ConfirmSelection();
	
	void SetFileFilter(String filter);

	/// Rendering
	virtual void Render(GraphicsState & graphicsState);

	/// Action to be taken on files once OK is pressed. Will be set to the msg parameter of the FileEvent message that is generated.
	String action;
	
	/// Woo.
	String CurrentPath() {return currentPath;};

private:
	// Update stuff.
	void OnDirPathUpdated(bool fromRenderThread);

	/// Old file browsers and their paths, using "name;path" syntax
	static List<String> oldFileBrowsers;

	UIInput * fileInput;
	UIInput * dirInput;
	UIList * dirList;
	String title;
	String fileFilter;
	String currentPath;

	/// For dynamism
	bool directoryLoaded;
	bool childrenCreated;

	/// List of all selected files. This will most often only include one file, but may contain more.
	List<String> fileSelection;
};

#endif