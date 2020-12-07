/// Emil Hedemalm
/// 2014-01-14
/// A file-browser extention of the UI-classes.

#include "UITypes.h"
#include "UIInputs.h"
#include "UIFileBrowser.h"
#include "UIButtons.h"
#include "File/FileUtil.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"

List<String> UIFileBrowser::oldFileBrowsers;

UIFileBrowser::UIFileBrowser(String title, String action, String fileFilter)
: UIList(), title(title), action(action), fileFilter(fileFilter)
{
	type = UIType::FILE_BROWSER;
	this->textureSource = "black.png";
	// Set the title to be the name of this whole thingy!
	this->name = title+"FileBrowser";
	this->exitable = true;
	// Specify that this dialogue should be deallocated after popping it.
	removeOnPop = true;
	// Disable cyclicity here, it annoys.
	cyclicY = false;

	dirInput = NULL;
	dirList = NULL;

	/// For loading dynamically right before rendering.
	directoryLoaded = false;
	childrenCreated = false;

	// Since the file browser should cover EXACTLY 1 screen, disable any scroll-bars popping up and blocking big parts of the screen...
	createScrollBarsAutomatically = false;

	/// Scan if this browser has appeared before, and use the old path if so.
	for (int i = 0; i < oldFileBrowsers.Size(); ++i)
	{
		String oldFileBrowser = oldFileBrowsers[i];
		List<String> tokens = oldFileBrowser.Tokenize(";");
		String oldName = tokens[0];
		if (oldName == name)
		{
			if (tokens.Size() > 1)
				currentPath = tokens[1];
			break;
		}
	}
}

UIFileBrowser::~UIFileBrowser()
{	
	std::cout<<"\nUIFileBrowser destructor.";
}

/// Creates ze children!
void UIFileBrowser::CreateChildren(GraphicsState* graphicsState)
{
	if (childrenCreated)
		return;

	assert(this->children.Size()==0);
	
	// Create a title
	UILabel * label = new UILabel();
	label->SetText(title);
	label->sizeRatioY = 0.1f;
	AddChild(nullptr, label);

	// Create path/dir input
	dirInput = new UIInput();
	dirInput->name = this->name+"DirInput";
	dirInput->onTrigger = "SetFileBrowserDirectory("+this->name+",this)";
	dirInput->sizeRatioY = 0.1f;
	dirInput->SetText(currentPath);
	AddChild(nullptr, dirInput);
	// Create file-list
	dirList = new UIList();
	dirList->sizeRatioY = 0.6f;
	dirList->name = this->name+"DirList";
	AddChild(nullptr, dirList);
	
	// Create file-name input
	fileInput = new UIInput();
	fileInput->name = this->name+"FileInput";
	fileInput->sizeRatioY = 0.1f;
	fileInput->onTrigger = "SetFileBrowserFile("+this->name+",this)";
	AddChild(nullptr, fileInput);

	// Create OK/Cancel buttons
	UIColumnList * cList = new UIColumnList();
	cList->sizeRatioY = 0.1f;
	AddChild(nullptr, cList);
	
	UIButton * cancelButton = new UIButton();
	cancelButton->name = "Cancel";
	cancelButton->SetText("Cancel");
	cancelButton->sizeRatioX = 0.4f;
	cancelButton->activationMessage = "PopUI("+this->name+")";
	cList->AddChild(nullptr, cancelButton);

	UIButton * okButton = new UIButton();
	okButton->name = "OK";
	okButton->SetText("OK");
	okButton->sizeRatioX = 0.4f;
	okButton->activationMessage = "EvaluateFileBrowserSelection("+this->name+")&PopUI("+this->name+")";
	cList->AddChild(nullptr, okButton);
	
	/// Bind neighbours for proper ui navigation...
	/*
	cancelButton->rightNeighbourName = okButton->name;
	cancelButton->upNeighbourName = fileInput->name;
	okButton->leftNeighbourName = cancelButton->name;
	fileInput->downNeighbourName = okButton->name;
	dirList->downNeighbourName = fileInput->name;
	*/
	childrenCreated = true;
}

/// Clears and creates new list of directory-contents. Use false on first usage, rest should be true.
void UIFileBrowser::LoadDirectory(bool fromRenderThread)
{
	assert(dirList);
	// Clear contents
	if (fromRenderThread)
	{
		dirList->Clear();
	}
	// Find dir contents.
	if (currentPath.Length() == 0)
		currentPath = ".";
	List<String> dirs, files;
	dirs.AddItem("..");
	int result = GetDirectoriesInDirectory(currentPath, dirs);
	result = GetFilesInDirectory(currentPath, files);
	UIElement * firstDir = NULL;
	// Add dirs!
	for (int i = 0; i < dirs.Size(); ++i)
	{
		UIButton * dirButton = new UIButton();
		dirButton->sizeRatioY = 0.1f;
		dirButton->SetText(dirs[i]+"/");
#define LOW	0.5f
#define MID	0.7f
		dirButton->GetText().color = Color(Vector3f(LOW,MID,2.0f));
		dirButton->activationMessage = "UpdateFileBrowserDirectory("+this->name+","+dirs[i]+")";
		dirList->AddChild(nullptr, dirButton);
		// Save first directory so we may hover to it.
		if (i == 0)
			firstDir = dirButton;
	}
	// Add files!
	String filter = fileFilter;
	for (int i = 0; i < files.Size(); ++i){
		// Check if the file conforms to the requested file-type (if specified)
		if (!files[i].Contains(filter))
			continue;
		
		UIButton * fileButton = new UIButton();
		fileButton->sizeRatioY = 0.1f;
		fileButton->SetText(files[i]);
		fileButton->GetText().color = Color(Vector3f(LOW,2.0f,LOW));
		fileButton->activationMessage = "SetFileBrowserFile("+this->name+","+files[i]+")";
		dirList->AddChild(nullptr, fileButton);
	}
	// Hover to the element at once!
	firstDir->AddState(UIState::HOVER);
	directoryLoaded = true;
}


/// Call to update path, using given argument to add to the path.
void UIFileBrowser::UpdatePath(String cat, bool fromRenderThread)
{
	String newPath;
	if (cat == "..")
	{
		List<String> dirParts = currentPath.Tokenize("/");
		String lastPart = dirParts[dirParts.Size()-1];
		if (!(lastPart == ".." ||
			lastPart == "."))
		{
			dirParts.RemoveIndex(dirParts.Size()-1);
			for (int i = 0; i < dirParts.Size(); ++i)
				newPath += dirParts[i]+"/";
			currentPath = newPath;
		}
		else {
			for (int i = 0; i < dirParts.Size(); ++i)
				newPath += dirParts[i]+"/";
			currentPath = newPath + cat;
		} 
	}
	else {
		List<String> dirParts = currentPath.Tokenize("/");
		for (int i = 0; i < dirParts.Size(); ++i)
			newPath += dirParts[i]+"/";
		currentPath = newPath + cat;
	}
	// Update stuff
	OnDirPathUpdated(fromRenderThread);
}

/// Sets full path.
void UIFileBrowser::SetPath(String path, bool fromRenderThread)
{
	this->currentPath = path;
	OnDirPathUpdated(fromRenderThread);
}	

// Update stuff.
void UIFileBrowser::OnDirPathUpdated(bool fromRenderThread)
{
	// Update the gui too.
	if (fromRenderThread)
	{
		dirInput->name = currentPath;
	}
	else
	{
		if (dirInput)
			Graphics.QueueMessage(new GMSetUIs(dirInput->name, GMUI::TEXT, currentPath, ui));
	}
	// Save path into the list of old file browsers.
	for (int i = 0; i < oldFileBrowsers.Size(); ++i){
		String oldFileBrowser = oldFileBrowsers[i];
		List<String> tokens = oldFileBrowser.Tokenize(";");
		String oldName = tokens[0];
		if (oldName == name){
			// Remove previous entry
			oldFileBrowsers.RemoveIndex(i);
			break;
		}
	}
	// Add new entry.
	String newEntry = name+";"+currentPath;
	oldFileBrowsers.Add(newEntry);
	// Queue reload of the directory!
	directoryLoaded = false;
}

/// Sets file in the input-field for later evaluation.
void UIFileBrowser::SetActiveFile(String file){
	Graphics.QueueMessage(new GMSetUIs(fileInput->name, GMUI::TEXT, file, ui));
	fileSelection.Clear();
	fileSelection.Add(file);
}


/// Rendering
void UIFileBrowser::Render(GraphicsState & graphicsState)
{
	// Create children first if not done so already!!
	if (!childrenCreated)
		CreateChildren(&graphicsState);
	// Load directory if not done so already.
	if (!directoryLoaded)
		LoadDirectory(true);
	UIList::Render(graphicsState);
}


/// Returns the list of currently selected files in the browser.
List<String> UIFileBrowser::GetFileSelection(){
	List<String> files;
	for (int i = 0; i < fileSelection.Size(); ++i){
		String file = this->currentPath+"/"+fileSelection[i];
		files.Add(file);
	}
	if (files.Size() == 0){
		String file = this->currentPath+"/"+fileInput->GetText();
		if (file.Length() == 0)
			file = "default";
		files.Add(file);
	}
	return files;
}


/*
private:
	UIList * dirList;
	String title;
	String action;
	String fileFilter;
	String currentPath;
};
*/