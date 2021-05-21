/// Emil Hedemalm
/// 2014-06-12
/// User interface class.

#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <cstdlib>
#include "UIElement.h"
#include "Mutex/Mutex.h"

class Square;
class GraphicsState;
class UIElement;
class AppWindow;

/// Mutex for interacting with the UI. Any UI. Used to make sure the graphics isn't deleting anything the input is touching and vice-versa.
extern Mutex uiMutex;

void CreateUIMutex();
void DeleteUIMutex();

/// Fetches the global UI, taking into consideration active AppWindow.
UserInterface * GlobalUI();
/// Fetches active/current UI, taking into consideration active AppWindow.
UserInterface * ActiveUI();
// UI of main AppWindow.
UserInterface * MainUI();
/// UI which the mouse is currently hovering over, which may be any AppWindow.
UserInterface * HoverUI();
/// Fetches either the Global or Active UI, taking into consideration both active AppWindow and if there exist any valid content in the Global UI.
UserInterface * RelevantUI();
UserInterface * GetRelevantUIForWindow(AppWindow * AppWindow);

class UserInterface 
{
	// List of all existing UIs
	static List<UserInterface*> userInterfaces;
public:
	UserInterface();
	~UserInterface();
	/// checks that it's not actually deleted.
	static bool IsGood(UserInterface * ui);
	/// Deletes all UIs that have not already been deleted so far. Called at end of program.
	static void DeleteAll();
	/// Sets the bufferized flag. Should only be called before program shutdown. Ensures less assertions will fail.
	void SetBufferized(bool bufferizedFlag);
	/** Reloads all existing UserInterfaces based on their respective source-files. Should only be called from RENER THREAD! As it will want to deallocate stuff.
		Use Graphics.QueueMessage(new GraphicsMessage(GM_RELOAD_UI));
	*/
	static void ReloadAll(GraphicsState* graphicsState);

	// Creates the root element. Will not create another if it already exists.
	UIElement * CreateRoot();
	/// Returns the UI root element that should be handled with care...!
	UIElement * GetRoot(){return root;};

	/** Attempts to load the UI from file. Returns false if it failed.
		If any elements exist before loading they will be deleted first.
	*/
	bool Load(GraphicsState * graphicsState, String fromFile);
	/** Loads target UI from file, storing it as a single UIElement so that it may be inserted into any other UI of choice. 
		Caller is responsible for inserting it into a proper UI or deallocation if it fails.
	*/
	static UIElement * LoadUIAsElement(GraphicsState * graphicsState, String uiSrcLocation);

	/// Attempts to delete target element from the UI. Should only be called by the render-thread!
	bool Delete(GraphicsState* graphicsState, UIElement * element);

	/** Mouse interactions with the UI, in x and y from 0.0 to 1.0, with 0.0 being in the Upper Left corner. Returns the element currently hovered over.
		If allUi is specified the current stack order will be ignored to a certain extent, meaning that ui below the current stack top will be made available too.
		Search is conducted starting at the highest stack element and going down until an element is found that we can hover over.
	*/
	UIElement * Hover(GraphicsState* graphicsState, int x, int y, bool allUi = false);
	UIElement * Hover(GraphicsState* graphicsState, Vector2i xy, bool allUi = false);
	/// If allUi is specified, the action will try, similar to hover, and go through the entire stack from the top down until it processes an element.
	UIElement * Click(GraphicsState* graphicsState, int x, int y, bool allUi = false);
	UIElement * Activate();
	// Goes through the active stack until an element is found which corresponds to the given (visible) co-ordinates.
	UIElement * GetElementByPosition(Vector2f posXY);
	UIElement * GetElementByPosition(float x, float y);

	/// Woo!
	UIElement * GetElement(String byName, UIType andType);


	/// Primarily used by the system-global UI. This queries if there is any currently visible UI which may respond to user input available.
	bool HasActivatableElement();

	/// Sets target element as hovered one, removing the flag from all other elements.
	void SetHoverElement(GraphicsState* graphicsState, UIElement * targetElement);

	/** Function called when an element is activated,
		by mouse-clicking, pressing enter on selction or otherwise. */
	virtual void Activate(UIElement* activeElement);
	/** Attempts to adjust the UI to the provided width and height.
		If the values of width and height are the same as they were prior to the call
		no change will occur and a false will be returned, otherwise it will return true.
	*/
	bool AdjustToWindow(GraphicsState& graphicsState, Vector2i size);
	/// Creates the geometry needed before bufferization and rendering can be done.
	void CreateGeometry(GraphicsState* graphicsState);
	void ResizeGeometry(GraphicsState* graphicsState);
	void DeleteGeometry();
	/// Creates/updates VBOs for all UI elements.
	void Bufferize(GraphicsState* graphicsState);
	/// Releases GL resources
	bool Unbufferize(GraphicsState* graphicsState);
	/** Renders the whole UIElement structure.
		Overloaded by subclasses in order to enable custom perspective for the UI.
	*/
	virtual void Render(GraphicsState & graphicsState);


    /// Prints the UI's tree recursively. The level parameter is in order to display the tree properly.
	void Print(int level = -1);
	void PrintStack();

	UILayout GetLayout() const;

	/// Returns flag if the UI is properly allocated or not.
	bool IsCreated() { if (root) return true; return false; };
	/// Returns a pointer to element with given ID. Returns NULL if it cannot be found.
	UIElement * GetElementById(int ID);
	/// Returns a  pointer to element with specified name. Returns NULL if it cannot be found.
	UIElement * GetElementByName(const char * name, UIFilter filter = UIFilter::None);
	/// Tries to fetch element by source, for when loaded from a .gui file straight into an element.
	UIElement * GetElementBySource(String source);
	/// Gets the currently active element (for input, probably!)
	UIElement * GetActiveElement();
	/// If false, will use current stack top to get hover element. If using mouse you should pass true as argument.
	UIElement * GetHoverElement(bool fromRoot = false);
	/// Returns active input focus element, if any. Based on the GetActiveElement but performs additional checks.
	UIElement * ActiveInputFocusElement();
	/// Getter for element by state, where the stateFlag will be bitwise anded (&) to fetch the correct element. GetActiveElement & GetHoverElement work similarly.
	UIElement * GetElementByState(int stateFlag);
	/// Fetches all elements conforming to the bitwise and'ed (&) state flags provided.
	bool GetElementsByState(int stateFlag, List<UIElement*> & listToFill);
	/// Checks for visibility, activateability, etc. Also works bit-wise!
	UIElement * GetElementByFlag(int uiFlag);
	bool GetElementsByFlags(int uiFlags, List<UIElement*> & listToFill);

	// Fetches all hoverable and/or activatable/interactable elements in this UI.
	List<UIElement*> GetRelevantElements();

	// Is it navigatable?
	static bool IsNavigatable(UIElement * element);

	/// Returns a pointer to the root element
	UIElement * Root() {return root;};
	const int Width(){ return width;};
	const int Height(){ return height;};
	bool IsBuffered() { return isBuffered; };
	bool IsGeometryCreated() { return isGeometryCreated;};
	static bool printDebug;
	String Source(){ return source;};

	/// Statistics for newly created elements that may be applied if wished.
	//Vector4f defaultTextColor;

	/// Checks if it is in the stack already. Can be good to avoid duplicates...
	bool InStack(UIElement * element);
	enum {
		PUSHED_TO_STACK,
		ALREADY_IN_STACK,
		NULL_ELEMENT,
	};
	/** Pushes given element to the interaction/display stack. Returns false if already in there/failed.	
	*/
	int PushToStack(String elementName);
	int PushToStack(UIElement * element);
	void PopFromStack(GraphicsState * graphicsState, String elementName);
	/// Pops target element from the interaction/display stack. If force is true it will do it no matter what the element's exitable property says.
	bool PopFromStack(GraphicsState * graphicsState, UIElement * element, bool force = false);
	/// Returns the stack top or root if empty.
	UIElement * GetStackTop();
	/// Pops the top element, returning a pointer to it.
	UIElement * PopFromStack();
	int StackSize() const;
	/// Querier
	bool IsInStack(String elementName) const;
	/// Removes target state flags from all elements in the active stack-level.
	void RemoveState(int state);

	/// Called when this UI is made active (again).
	void OnEnterScope();
	/// Called once the UI is not active anymore. No state has to change, but it will not render to the user for now.
	void OnExitScope();

	String defaultTextureSource;
	/// Directory for the UI relative to root (bin/)
	static String rootUIDir;
	

	String name;
	// In milliseconds, used to determien which UIs to reload.
	int64 lastRenderTime;

protected:

	/// Deallocates UI, and reloads from base-file.
	void Reload(GraphicsState* graphicsState);
	/// Loads from target file, using given root as root-element in the UI-hierarchy.
	static UIElement * LoadFromFile(GraphicsState* graphicsState, String filePath, UserInterface * ui);

	/// Removes references to target element
	void OnElementDeleted(UIElement * element);

	String source;
//	String uiName;
	bool isGeometryCreated;
	bool isBuffered;
	/** If true, there must always (if possible) be one element that is hovered over. 
		What this means is that it will retain previous elements as hovered even if the mouse hovers over invalid elements.
	*/
	bool demandHovered;

	UILayout windowLayout;

	///  Root UIElement
	UIElement * root;
	/// To avoid recursiveness that doesn't seem to work...
	UIElement * hoverElement;
	/// Display/interaction stack. The top-most object is always the item that should get all focus!
	List<UIElement*> stack;
	/// UI Dimensions. Current width of the screen for the UI.
	int width;
	/// UI Dimensions. Current height of the screen for the UI.
	int height;
};

// Setter functions
bool setScrollBarLevel(UIElement * scrollBar, float level);
bool setSliderLevel(UIElement * slider, float level);

// Getter functions
float getSliderLevel(UIElement * slider);
float getScrollBarLevel(UIElement * scrollBar);

// Toggles a player ready in the UI for multiplayer lobby
void ToggleReady(char player, char ready = -1);

#endif
