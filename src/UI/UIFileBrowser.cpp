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
#include "Message/FileEvent.h"
#include "Message/MessageManager.h"

List<String> UIFileBrowser::oldFileBrowsers;

UIFileBrowser::UIFileBrowser(String name, String action, String fileFilter)
: UIList(), title(name), action(action), fileFilter(fileFilter)
{
	type = UIType::FILE_BROWSER;
	this->textureSource = "black.png";
	// Set the title to be the name of this whole thingy!
	this->name = title;
	this->exitable = true;
	// Specify that this dialogue should be deallocated after popping it.
	//removeOnPop = true;
	// Disable cyclicity here, it annoys.
	cyclicY = false;

	dirInput = NULL;
	dirList = NULL;
	padding = 0.05f;
	sizeRatioX = sizeRatioY = 0.9f;

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
	InheritDefaults(label);
	label->SetText(title.Length() > 0? title : name);
	label->sizeRatioY = 0.1f;
	label->SetTextColors(Color(1, 1, 1, 1));
	AddChild(nullptr, label);

	// Create path/dir input
	dirInput = new UIStringInput(this->name+"DirInput", "SetFileBrowserDirectory(" + this->name + ",this)");
	InheritDefaults(dirInput);
	//dirInput->name = ;
	//dirInput->onTrigger = ;
	dirInput->sizeRatioY = 0.1f;
	dirInput->CreateChildren(graphicsState);
	dirInput->SetText("Dir: ");
	dirInput->SetValue(currentPath);
	AddChild(nullptr, dirInput);

	// Create file-list
	dirList = new UIList();
	InheritDefaults(dirList);
	dirList->sizeRatioY = 0.6f;
	dirList->name = this->name+"DirList";
	AddChild(nullptr, dirList);
	
	// Create file-name input
	fileInput = new UIStringInput(this->name + "FileInput", "SetFileBrowserFile(" + this->name + ",this)");
	InheritDefaults(fileInput);
	//fileInput->name = ;
	fileInput->sizeRatioY = 0.1f;
	//fileInput->onTrigger = ;
	fileInput->onTriggerActions.Add(UIAction(UIAction::SET_FILE_BROWSER_FILE_FROM_INPUT, this));
	fileInput->CreateChildren(graphicsState);
	fileInput->SetText("File: ");
	AddChild(nullptr, fileInput);

	// Create OK/Cancel buttons
	UIColumnList * cList = new UIColumnList();
	InheritDefaults(cList);
	cList->sizeRatioY = 0.1f;
	AddChild(nullptr, cList);
	
	UIButton * cancelButton = new UIButton();
	InheritDefaults(cancelButton);
	cancelButton->name = "Cancel";
	cancelButton->SetText("Cancel");
	cancelButton->sizeRatioX = 0.4f;
//	cancelButton->activationMessage = "PopUI("+this->name+")";
	cancelButton->activationActions.AddItem(UIAction(UIAction::POP_UI, this));
	cList->AddChild(nullptr, cancelButton);

	UIButton * okButton = new UIButton();
	InheritDefaults(okButton);
	okButton->name = "OK";
	okButton->SetText("OK");
	okButton->sizeRatioX = 0.4f;
	//okButton->activationMessage = "EvaluateFileBrowserSelection("+this->name+")&PopUI("+this->name+")";
	okButton->activationActions.Add(UIAction(UIAction::CONFIRM_FILE_BROWSER_SELECTION, this));
	okButton->activationActions.AddItem(UIAction(UIAction::POP_UI, this));
	cList->AddChild(nullptr, okButton);
	
	/// Bind neighbours for proper ui navigation...
	dirInput->downNeighbour = dirList;
	//dirList->downNeighbour = fileInput;
	//fileInput->downNeighbour

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
void UIFileBrowser::LoadDirectory(GraphicsState* graphicsState)
{
	assert(dirList);
	// Clear contents
	if (graphicsState)
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
		InheritDefaults(dirButton);
		dirButton->textureSource = "";
		dirButton->sizeRatioY = 0.1f;
		dirButton->SetText(dirs[i]+"/");
#define LOW	0.15f
#define MID	0.35f
#define HIGH 0.6f
		dirButton->SetTextColors(TextColors(Vector3f(LOW,MID, HIGH)));
		//dirButton->activationMessage = "UpdateFileBrowserDirectory("+this->name+","+dirs[i]+")";
		dirButton->activationActions.Add(UIAction(UIAction::SELECT_FILE_BROWSER_DIRECTORY, this, dirs[i]));
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
		InheritDefaults(fileButton);
		fileButton->textureSource = "";
		fileButton->sizeRatioY = 0.1f;
		fileButton->SetText(files[i]);
		fileButton->SetTextColors(TextColors(Vector3f(LOW, HIGH,LOW)));
		// fileButton->activationMessage = "SetFileBrowserFile("+this->name+","+files[i]+")";
		fileButton->activationActions.Add(UIAction(UIAction::SELECT_FILE_BROWSER_FILE, this, files[i]));
		dirList->AddChild(nullptr, fileButton);
	}
	// Hover to the element at once!
	firstDir->AddState(graphicsState, UIState::HOVER);
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
			Graphics.QueueMessage(new GMSetUIs(dirInput->name, GMUI::STRING_INPUT, currentPath, ui));
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
void UIFileBrowser::SetActiveFile(GraphicsState* graphicsState, String file){
	fileSelection.Clear();
	fileSelection.Add(currentPath + "/" + file);

	if (graphicsState) {
		//fileInput->SetText(file);
		fileInput->SetValue(file);
	}
	else {
		Graphics.QueueMessage(new GMSetUIs(fileInput->name, GMUI::TEXT, file, ui));
	}
}

void UIFileBrowser::SetActiveFileFromInput(GraphicsState* graphicsState) {
	fileSelection = currentPath + "/" + fileInput->GetValue();
}


/// Rendering
void UIFileBrowser::Render(GraphicsState & graphicsState)
{
	// Create children first if not done so already!!
	if (!childrenCreated)
		CreateChildren(&graphicsState);
	// Load directory if not done so already.
	if (!directoryLoaded)
		LoadDirectory(&graphicsState);
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

void UIFileBrowser::ConfirmSelection() {
	FileEvent * message = new FileEvent();
	UIFileBrowser * fb = (UIFileBrowser*)ui;
	message->msg = fb->action;
	if (message->msg.Length() == 0)
		message->msg = name;
	message->files = fileSelection;
	// Queue the new message.
	MesMan.QueueMessage(message);
}

void UIFileBrowser::SetFileFilter(String filter) {
	fileFilter = filter;
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