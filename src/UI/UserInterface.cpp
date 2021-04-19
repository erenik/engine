/// Emil Hedemalm
/// 2014-06-12
/// User interface class.

#include "UserInterface.h"
#include "InputState.h"
#include "../Globals.h"

#include "../StateManager.h"
#include "../TextureManager.h"
#include "Message/MessageManager.h"
#include "Message/Message.h"
#include "Mesh/Square.h"
#include "../GraphicsState.h"
#include <iomanip>

#include "Graphics/Fonts/TextFont.h"
#include "Window/AppWindowManager.h"
#include "Viewport.h"
#include "Color.h"

#include "File/LogFile.h"
#include "String/StringUtil.h"

/// Mutex for interacting with the UI. Any UI. Used to make sure the graphics isn't deleting anything the input is touching and vice-versa.
Mutex uiMutex;

void CreateUIMutex()
{
	uiMutex.Create("UIMutex");
}
void DeleteUIMutex()
{
	uiMutex.Destroy();
}


/// Fetches the global UI, taking into consideration active AppWindow.
UserInterface * GlobalUI()
{
	AppWindow * activeWindow = WindowMan.GetCurrentlyActiveWindow();
	if (!activeWindow)
		return NULL;
	return activeWindow->GetGlobalUI();
}

/// Fetches active/current UI, taking into consideration active AppWindow.
UserInterface * ActiveUI()
{
	AppWindow * activeWindow = WindowMan.GetCurrentlyActiveWindow();
	if (!activeWindow)
	{
		LogMain("ActiveUI(): No active AppWindow!", WARNING);
		return NULL;
	}
	return activeWindow->GetUI();
}

UserInterface * MainUI()
{
	AppWindow * mainWindow = MainWindow();
	if (!mainWindow)
	{
		LogMain("MainUI(): No main AppWindow!", WARNING);
		return NULL;
	}
	return mainWindow->GetUI();
}

/// UI which the mouse is currently hovering over, which may be any AppWindow.
UserInterface * HoverUI()
{
	AppWindow * hoverWindow = WindowMan.HoverWindow();
	if (!hoverWindow)
		return NULL;
	return hoverWindow->GetUI();
}


/// Fetches either the Global or Active UI, taking into consideration both active AppWindow and if there exist any valid content in the Global UI.
UserInterface * RelevantUI()
{
	AppWindow * window = WindowMan.GetCurrentlyActiveWindow();
	if (!window)
		return NULL;
	UserInterface * globalUI = window->GetGlobalUI();
	if (globalUI && globalUI->HasActivatableElement())
		return globalUI;
	
	return window->GetUI();
}

UserInterface * GetRelevantUIForWindow(AppWindow * window)
{
	if (!window)
		return NULL;
	UserInterface * globalUI = window->GetGlobalUI();
	if (globalUI && globalUI->HasActivatableElement())
		return globalUI;
	
	return window->GetUI();	
}



bool UserInterface::printDebug = true;

List<UserInterface*> UserInterface::userInterfaces;

UserInterface::UserInterface()
{
	this->root = NULL;
	isGeometryCreated = false;
	isBuffered = false;
	hoverElement = NULL;
	demandHovered = true;
//	defaultTextColor = Vector4f(1.f,1.f,1.f,1.f);
	defaultTextureSource = "80Gray50Alpha.png";
	userInterfaces.Add(this);
}

UserInterface::~UserInterface()
{
	// All buffers are removed via gl later. just send a warning for now?
	if (isBuffered)
		std::cout<<"\nWARNING: UserInterface buffered while deallocating it?";
	if (isGeometryCreated)
		DeleteGeometry();
	assert(!isGeometryCreated);
	if (root != NULL)
		delete root;
	root = NULL;
	userInterfaces.RemoveItemUnsorted(this);
}

/// checks that it's not actually deleted.
bool UserInterface::IsGood(UserInterface * ui)
{
	return userInterfaces.Exists(ui);
}



/** Mouse interactions with the UI, in x and y from 0.0 to 1.0, with 0.0 being in the Upper Left corner. Returns the element currently hovered over.
	If allUi is specified the current stack order will be ignored to a certain extent, meaning that ui below the current stack top will be made available too.
	Search is conducted starting at the highest stack element and going down until an element is found that we can hover over.
*/
UIElement * UserInterface::Hover(int x, int y, bool allUi)
{
	UIElement * stackTop = GetStackTop();
	if (!stackTop)
		return NULL;
	UIElement * previousHoverElement = NULL;
	if (!previousHoverElement)
	{			
		previousHoverElement = stackTop->GetElementByState(UIState::HOVER);
//		std::cout<<"\nPrevious hover element: "<<previousHoverElement;
//		if (previousHoverElement)
//			std::cout<<previousHoverElement->name;
	}
	UIElement * result = NULL;
	/// Search taking into consideration the stack but searching until an element is found.
	if(allUi)
	{
		for (int i = stack.Size()-1; i >= 0; --i)
		{
			UIElement * stackElement = stack[i];
			/// Remove the hover state before re-hovering.
			stackElement->RemoveState(UIState::HOVER, true);
			result = stackElement->Hover(x,y);
			if (result) {
				break;
			}
		}
		/// If still no result, try the root.
		if (!result)
		{
			root->RemoveState(UIState::HOVER, true);
			result = root->Hover(x,y);
		}
		/// Demand hover will have to be investigated how it could work in this mode, if at all.
		hoverElement = result;	
	}
	/// Old search using only the stack-top.
	else {
		UIElement * previous = stackTop->GetElementByState(UIState::HOVER);
		/// Remove the hover flag before re-hovering.
		stackTop->RemoveState(UIState::HOVER, true);
		result = stackTop->Hover(x,y);
		hoverElement = result;
		/// If we always want a hovered element (for whatever reason).
		if (!hoverElement && demandHovered && previous){
			hoverElement = stackTop->Hover(previous->posX, previous->posY);
		}
	}

	if (inputState->demandHoverElement && result == NULL)
	{
		/// Try reverting to previous element?
		if (previousHoverElement)
		{
			result = previousHoverElement;
			result->state |= UIState::HOVER;
		}
		if (result == NULL)
		{
//			std::cout<<"\nProblema.";
		}
	}
	return result;
}
UIElement * UserInterface::Hover(Vector2i xy, bool allUi)
{
	return Hover(xy.x, xy.y, allUi);
}

/// If allUi is specified, the action will try, similar to hover, and go through the entire stack from the top down until it processes an element.
UIElement * UserInterface::Click(int x, int y, bool allUi){
	UIElement * elementClicked = NULL;
	if (allUi){
		for (int i = stack.Size()-1; i >= 0; --i)
		{
			UIElement * stackElement = stack[i];
			elementClicked = stackElement->Click(x,y);
			if (elementClicked)
				break;
		}
		/// If still no result, try the root.
		if (!elementClicked)
		{
			elementClicked = root->Click(x,y);
		}
		/// Demand hover will have to be investigated how it could work in this mode, if at all.
	}
	/// Old code for just the stack-top
	else {
		UIElement * stackTop = GetStackTop();
		if (!stackTop)
			return NULL;
		/// Clear the active-flag?
		elementClicked = stackTop->Click(x,y);
	}
	return elementClicked;
}
UIElement * UserInterface::Activate(){
	UIElement * stackTop = GetStackTop();
	if (!stackTop)
		return NULL;
	UIElement * result = NULL;
	result = stackTop->Activate(nullptr);
	if (result)
	{
		if (result->activationMessage.Length() == 0){
		//	assert(false && "Activatable UI element has no valid activation message string!");
			return NULL;
		}
		if (result->activationMessage.Length() != 0){
			if (result->activationMessage.Type() == String::WIDE_CHAR)
				result->activationMessage.ConvertToChar();
			List<String> msgs = result->activationMessage.Tokenize("&");
			for (int i = 0; i < msgs.Size(); ++i){
				Message * message = new Message(msgs[i]);
				message->element = result;
				MesMan.QueueMessage(message);
			}
			return result;
		}
		else {
			std::cout<<"\nonActivate and activationMessage both NULL in element: "<<result->name;
			return NULL;
		}
	}
	return result;
}

/// Primarily used by the system-global UI. This queries if there is any currently visible UI which may respond to user input available.
bool UserInterface::HasActivatableElement()
{
	if (root == nullptr)
		return false;
	List<UIElement * > list;
	root->GetElementsByFlags(UIFlag::ACTIVATABLE | UIFlag::VISIBLE, list);
	return list.Size() > 0;
}

/// Sets target element as hovered one, removing the flag from all other elements.
void UserInterface::SetHoverElement(GraphicsState* graphicsState, UIElement * targetElement)
{
	/// Remove the hover flag from all other UIs in the same stack.
	RemoveState(UIState::HOVER);
	if (!targetElement)
		return;
	/// Then add it to our specified one.
	targetElement->AddState(UIState::HOVER);
	targetElement->EnsureVisibility(graphicsState);
	targetElement->OnHover();
}



/** Function called when an element is activated,
	by mouse-clicking, pressing enter on selction or otherwise. */
void UserInterface::Activate(UIElement* activeElement)
{
	assert(false);
}

// Goes through the active stack until an element is found which corresponds to the given (visible) co-ordinates.
UIElement * UserInterface::GetElementByPosition(Vector2f posXY) {
	return GetElementByPosition(posXY.x, posXY.y);
}
UIElement * UserInterface::GetElementByPosition(float x, float y)
{
	// Goes through the active stack until an element is found which corresponds to the given (visible) co-ordinates.
	for (int i = stack.Size() - 1; i >= 0; --i)
	{
		UIElement * element = stack[i];
		UIElement * result = element->GetElement(x, y);
		if (result)
			return result;
	}
    if (!root)
		return NULL;
    return root->GetElement(x,y);
}

/// Woo!
UIElement * UserInterface::GetElement(String byName, UIType andType)
{
	if (!root)
		return NULL;
	return root->GetElement(byName, andType);
}


/// Creates the content of the UI. Overloaded by subclasses.
void UserInterface::CreateGeometry(){
	assert(!isGeometryCreated);
	std::cout<<"UserInterface::CreateGeometry called";
	root->CreateGeometry();
	isGeometryCreated = true;
}
void UserInterface::ResizeGeometry()
{
	if (!isGeometryCreated)
		CreateGeometry();
	assert(isGeometryCreated);
	root->ResizeGeometry();
}
void UserInterface::DeleteGeometry()
{
	if (!isGeometryCreated)
		return;
	assert(isGeometryCreated);
	assert(!isBuffered);
	root->DeleteGeometry();
	width = 0;
	height = 0;
	isGeometryCreated = false;
}

/** Attempts to adjust the UI to the provided width and height.
	If the values of width and height are the same as they were prior to the call
	no change will occur and a false will be returned, otherwise it will return true.
*/
bool UserInterface::AdjustToWindow(Vector2i size)
{
	if (root == nullptr)
		return false;
	// Check if we need to do anything at all first!
	if (width == size[0] && height == size[1])
		return false;
	width = size[0]; height = size[1];
	root->AdjustToWindow(0, width, 0, height);
	return true;
}

/// Creates/updates VBOs for all UI elements.
void UserInterface::Bufferize()
{
//	if (isBuffered)
//		return;
	// Allow multiple bufferizations, since they do not necessarily generate new memory usage, like textures?
//	assert(!isBuffered);
	assert(this->isGeometryCreated);
	root->Bufferize();
	isBuffered = true;
//	std::cout<<"\nUI "<<this->source<<" bufferized.";
}

/// Releases GL resources
bool UserInterface::Unbufferize()
{
	if (!isBuffered)
		return false;
	if (root)
		root->FreeBuffers();
	else
		return false;
	isBuffered = false;
	return true;
}

/** Renders the whole UIElement structure.
	Overloaded by subclasses in order to enable custom perspective or other stuff for the UI.
*/
void UserInterface::Render(GraphicsState & graphicsState)
{
	/// Disable depth-test.
	glDisable(GL_DEPTH_TEST);
	
	/// Render the tree.
	// root->Render(graphicsState);

	// Render the stack instead! Tree may be outdated. o-o;;
	for (int i = 0; i < stack.Size(); ++i) {
		stack[i]->Render(graphicsState);
	}

	lastRenderTime = Timer::GetCurrentTimeMs();
}

/// Prints the UI's tree recursively. The level parameter is in order to display the tree properly.
void UserInterface::Print(int level/* = -1*/){
    root->Print(level);
}

void UserInterface::PrintStack() {
	String toLog;
	toLog += "\nUI::PrintStack for UI: " + name;
	toLog += "\nStack size: " + String(stack.Size());
	for (int i = 0; i < stack.Size(); ++i) {
		toLog += "\n "+String(i)+": " + stack[i]->name+", visible: "+ String(stack[i]->visible);
	}
	std::cout << toLog;
}

/// lol?
UIElement * UserInterface::GetElementById(int ID){
	if (root)
		return root->GetElementWithID(ID);
	return NULL;
}

/// Returns a  pointer to element with specified name. Returns NULL if it cannot be found.
UIElement * UserInterface::GetElementByName(const char * name, UIFilter filter /*= UIFilter::None*/){
	if (name == NULL)
		return NULL;
	if (root)
		return root->GetElementByName(name, filter);
	return NULL;
};

/// Tries to fetch element by source, for when loaded from a .gui file straight into an element.
UIElement * UserInterface::GetElementBySource(String source)
{
	if (source.Length() == 0)
		return NULL;
	if (root)
		return root->GetElementBySource(source);
	return NULL;
}

/// Gets the currently active element (for input, probably!)
UIElement * UserInterface::GetActiveElement()
{
	if (root)
		return root->GetActiveElement();
	return NULL;
}

/// Returns active input focus element, if any. Based on the GetActiveElement but performs additional checks.
UIElement * UserInterface::ActiveInputFocusElement()
{
	UIElement* stackTop = GetStackTop();
	if (stackTop == nullptr)
		return nullptr;
	UIElement * e = GetStackTop()->GetActiveElement();
	if (!e)
		return NULL;
	if (e->demandInputFocus)
		return e;
	return NULL;
}

/// Getter for element by state, where the stateFlag will be bitwise anded (&) to fetch the correct element.
UIElement * UserInterface::GetElementByState(int stateFlag){
	if (root)
		return root->GetElementByState(stateFlag);
	return NULL;
}

/// Fetches all elements conforming to the bitwise and'ed (&) state flags provided.
bool UserInterface::GetElementsByState(int stateFlag, List<UIElement*> & listToFill){
	UIElement * e = this->GetStackTop();
	if (!e)
		return false;
	e->GetElementsByState(stateFlag, listToFill);
	return true;
}

/// Checks for visibility, activateability, etc. Also works bit-wise!
UIElement * UserInterface::GetElementByFlag(int uiFlag){
	if (root)
		return root->GetElementByFlag(uiFlag);
	return NULL;
}

bool UserInterface::GetElementsByFlags(int uiFlags, List<UIElement*> & listToFill){
	/// If stack is active, use it as a guide instead!
	if (stack.Size())
		return stack.Last()->GetElementsByFlags(uiFlags, listToFill);
	if (root)
		return root->GetElementsByFlags(uiFlags, listToFill);
	return NULL;
}

// Fetches all hoverable and/or activatable/interactable elements in this UI.
List<UIElement*> UserInterface::GetRelevantElements() {
	List<UIElement*> elements;
	List<UIElement*> uiList;
	this->GetElementsByFlags(UIFlag::ACTIVATABLE | UIFlag::VISIBLE, uiList);
	/// Perform filtering by some other kind too. Like a priority variable?
	elements = uiList;
	return elements;
}

bool UserInterface::IsNavigatable(UIElement * element)
{
	if (!element)
		return false;
	return element->IsNavigatable();
}


/// If false, will use current stack top to get hover element. If using mouse you should pass true as argument.
UIElement * UserInterface::GetHoverElement(bool fromRoot /* = false */)
{
	if (fromRoot)
		return root->GetElementByState(UIState::HOVER);
	UIElement * stackTop = GetStackTop();
	if (stackTop)
		return stackTop->GetElementByState(UIState::HOVER);
	return NULL;
}

/// Attempts to delete target element from the UI. Should only be called by the render-thread!
bool UserInterface::Delete(UIElement * element){
	if(!this->IsInStack(element->name)){
		// std::cout<<"\nUserInterface::Delete: No such element "<<element->name<<" within current UI tree!";
	}
	if (root){
		/// Remove any references we might have to the element.
		OnElementDeleted(element);
		bool result = root->Delete(element);
		return result;
	}
	return false;
}

/// Deletes all UIs that have not already been deleted so far.
void UserInterface::DeleteAll()
{
	while(userInterfaces.Size())
	{
		UserInterface * ui = userInterfaces[0];
		ui->SetBufferized(false);
		delete ui;
	}
}

/// Sets the bufferized flag. Should only be called before program shutdown. Ensures less assertions will fail.
void UserInterface::SetBufferized(bool bufferizedFlag)
{
	this->isBuffered = bufferizedFlag;
	if (root)
		root->SetBufferized(bufferizedFlag);
}	

/** Reloads all existing UserInterfaces based on their respective source-files. Should only be called from RENER THREAD! As it will want to deallocate stuff.
	Use Graphics.QueueMessage(new GraphicsMessage(GM_RELOAD_UI));
*/
void UserInterface::ReloadAll()
{
	for (int i = 0; i < userInterfaces.Size(); ++i)
	{
		UserInterface * ui = userInterfaces[i];
		// Reload only currently visible ones?
		if ( ui->lastRenderTime < Timer::GetCurrentTimeMs() - 1000)
			continue;
		// Reload!
		ui->Reload();
	}		
}

// Creates the root element. Will not create another if it already exists.
UIElement * UserInterface::CreateRoot()
{
	root = NewA(UIElement);
	root->SetRootDefaults(this);
	return root;
}

/// Directory for the UI relative to root (bin/)
String UserInterface::rootUIDir; //  = "gui/";

/** Attempts to load the UI from file. Returns false if it failed.
	If any elements exist before loading they will be deleted first.
*/
bool UserInterface::Load(String fromFile)
{
	std::cout<<"\nLoading UI from file "<<fromFile<<"...";
    if (root)
		delete root;
	/// Create the root node.
	CreateRoot();

	name = fromFile;
	/// Load into root.
	root = LoadFromFile(fromFile, this);
	if (root != nullptr){
		source = fromFile;
		std::cout<<" done.";
	}
	else 
		std::cout<<" failed.";
	return false;
}


/** Loads target UI from file, storing it as a single UIElement so that it may be inserted into any other UI of choice.
	Caller is responsible for inserting it into a proper UI or deallocation if it fails.
*/
/* static */ UIElement * UserInterface::LoadUIAsElement(String uiSrcLocation){
	UIElement * newElement = LoadFromFile(uiSrcLocation, nullptr);
	/// Check that it worked as intended.
	if (newElement == nullptr)
	{
		std::cout<<"\nUserInterface::LoadUIAsElement: Failed.";
		delete newElement;
		newElement = NULL;
	}
	return newElement;
}


/// Checks if it is in the stack already. Can be good to avoid duplicates...
bool UserInterface::InStack(UIElement * element){
	for (int i = 0; i < stack.Size(); ++i){
		UIElement * s = stack[i];
		if (s == element)
			return true;
	}
	return false;
}

/// Pushes given element to the interaction/display stack.
int UserInterface::PushToStack(String elementName){
	return PushToStack(GetElementByName(elementName));
}
int UserInterface::PushToStack(UIElement * element)
{
	if (element->GetRoot() != root)
	{
		assert(false && "Trying to push element which doesn't belong to this ui.");
		return NULL_ELEMENT;
	}
	if (!element)
			return NULL_ELEMENT;
	if (InStack(element))
		return ALREADY_IN_STACK;
	/// This adds at the end, yes?
	stack.Add(element);
	element->visible = true;
	/// Mark as in the stack, so that navigation commands don't go outside it or to a parent-node.
	element->inStack = true;
	// Set up link so we can refer to it for pushing things, like drop-down lists.
	element->SetUI(this);
	return PUSHED_TO_STACK;
}
void UserInterface::PopFromStack(GraphicsState * graphicsState, String elementName){
	PopFromStack(graphicsState, GetElementByName(elementName));
}
bool UserInterface::PopFromStack(GraphicsState * graphicsState, UIElement * element, bool force)
{
	if (element->GetRoot() != root)
	{
		assert(false && "Trying to pop element which doesn't belong to this ui.");
		return false;
	}
	if (!element){
		std::cout<<"\nUserInterface::PopFromStack: NULL element";
		return false;
	}
	std::cout<<"\nUserInterface::PopFromStack for element: "<<element->name<<" and force="<<force;
	if (!element->exitable && !force){
		std::cout<<"\nUserInterface::PopFromStack: Element not exitable. Use force=true to override.";
		return false;
	}
	/// TODO: Assert that the element is the top-layer one?
	/// Option to ensure that it doesn't flip any other elements anyway.
	bool result = stack.RemoveItem(element) != NULL;
	assert(!stack.Exists(element));
	/// Call on exit scope for it!
	element->OnExitScope(force);
	element->visible = false; // Make invisible when popped from stack

	return result;
}

/// Pops the top element, returning a pointer to it.
UIElement * UserInterface::PopFromStack(){
	if (!stack.Size())
		return NULL;
	UIElement * last = stack.Last();
	stack.Remove(last);
	last->visible = false;
	last->inStack = false;
	return last;
}
int UserInterface::StackSize() const {
	return stack.Size();
};

/// Querier
bool UserInterface::IsInStack(String elementName) const{
	for (int i = 0; i < stack.Size(); ++i){
		UIElement * e = stack[i];
		if (e->name == elementName)
			return true;
	}
	return false;
}

/// Returns the stack top or root if empty.
UIElement * UserInterface::GetStackTop()
{
	if (stack.Size())
		return stack.Last();
	return root;
}

/// Removes target state flags from all elements in the active stack-level.
void UserInterface::RemoveState(int state){
	UIElement * currentElement = GetStackTop();
	if (!currentElement)
		return;
	currentElement->RemoveState(state, true);
}


/// Called when this UI is made active (again).
void UserInterface::OnEnterScope(){
	root->OnEnterScope();
}
/// Called once the UI is not active anymore. No state has to change, but it will not render to the user for now.
void UserInterface::OnExitScope()
{
	if (root)
		root->OnExitScope(false);
}

/// Deallocates UI, and reloads from base-file.
void UserInterface::Reload()
{
	std::cout<<"\nReloading UI "<<name;
	stack.Clear();
	this->Unbufferize();
	this->DeleteGeometry();
	delete root;
	root = LoadFromFile(source, this);
}


#include "UIParser.h"

/// Loads from target file, using given root as root-element in the UI-hierarchy.
/* static */ UIElement * UserInterface::LoadFromFile(String filePath, UserInterface * ui)
{
	String fromFile = filePath;
	if (!fromFile.Contains(rootUIDir)) {
		fromFile = rootUIDir + fromFile;
	}
	UIParser parser;
	return parser.LoadFromFile(fromFile, ui);
}


/// Removes references to target element
void UserInterface::OnElementDeleted(UIElement * element)
{
	stack.Remove(element);
}
