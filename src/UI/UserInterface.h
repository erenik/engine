#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <cstdlib>
#include "UIElement.h"

class Square;
struct GraphicsState;
class UIElement;

class UserInterface {
public:
	UserInterface();
	~UserInterface();

	// Creates the root element. Will not create another if it already exists.
	UIElement * CreateRoot();
	/// Returns the UI root element that should be handled with care...!
	UIElement * GetRoot(){return root;};

	/** Attempts to load the UI from file. Returns false if it failed.
		If any elements exist before loading they will be deleted first.
	*/
	bool Load(String fromFile);
	/** Loads target UI from file, storing it as a single UIElement so that it may be inserted into any other UI of choice. 
		Caller is responsible for inserting it into a proper UI or deallocation if it fails.
	*/
	static UIElement * LoadUIAsElement(String uiSrcLocation);

	/// Attempts to delete target element from the UI. Should only be called by the render-thread!
	bool Delete(UIElement * element);

	/** Mouse interactions with the UI, in x and y from 0.0 to 1.0, with 0.0 being in the Upper Left corner. Returns the element currently hovered over.
		If allUi is specified the current stack order will be ignored to a certain extent, meaning that ui below the current stack top will be made available too.
		Search is conducted starting at the highest stack element and going down until an element is found that we can hover over.
	*/
	UIElement * Hover(float x, float y, bool allUi = false);
	/// If allUi is specified, the action will try, similar to hover, and go through the entire stack from the top down until it processes an element.
	UIElement * Click(float x, float y, bool allUi = false);
	UIElement * Activate();
	/// Get element by position. Will skip invisible elements.
	UIElement * GetElement(float x, float y);

	/// Woo!
	UIElement * GetElement(String byName, int andType);

	/** Function called when an element is activated,
		by mouse-clicking, pressing enter on selction or otherwise. */
	virtual void Activate(UIElement* activeElement);
	/** Attempts to adjust the UI to the provided width and height.
		If the values of width and height are the same as they were prior to the call
		no change will occur and a false will be returned, otherwise it will return true.
	*/
	bool AdjustToWindow(int width, int height);
	/// Creates the geometry needed before bufferization and rendering can be done.
	void CreateGeometry();
	void ResizeGeometry();
	void DeleteGeometry();
	/// Creates/updates VBOs for all UI elements.
	void Bufferize();
	/// Releases GL resources
	bool Unbufferize();
	/** Renders the whole UIElement structure.
		Overloaded by subclasses in order to enable custom perspective for the UI.
	*/
	virtual void Render(GraphicsState& graphics);


    /// Prints the UI's tree recursively. The level parameter is in order to display the tree properly.
	void Print(int level = -1);

	/// Returns flag if the UI is properly allocated or not.
	bool IsCreated() { if (root) return true; return false; };
	/// Returns a pointer to element with given ID. Returns NULL if it cannot be found.
	UIElement * GetElementById(int ID);
	/// Returns a  pointer to element with specified name. Returns NULL if it cannot be found.
	UIElement * GetElementByName(const char * name);
	/// Tries to fetch element by source, for when loaded from a .gui file straight into an element.
	UIElement * GetElementBySource(String source);
	/// Gets the currently active element (for input, probably!)
	UIElement * GetActiveElement();
	UIElement * GetHoverElement();
	/// Returns active input focus element, if any. Based on the GetActiveElement but performs additional checks.
	UIElement * ActiveInputFocusElement();
	/// Getter for element by state, where the stateFlag will be bitwise anded (&) to fetch the correct element. GetActiveElement & GetHoverElement work similarly.
	UIElement * GetElementByState(int stateFlag);
	/// Fetches all elements conforming to the bitwise and'ed (&) state flags provided.
	bool GetElementsByState(int stateFlag, List<UIElement*> & listToFill);
	/// Checks for visibility, activateability, etc. Also works bit-wise!
	UIElement * GetElementByFlag(int uiFlag);
	bool GetElementsByFlags(int uiFlags, List<UIElement*> & listToFill);
	/// Returns a pointer to the root element
	UIElement * Root() {return root;};
	const int Width(){ return width;};
	const int Height(){ return height;};
	bool IsBuffered() { return isBuffered; };
	bool IsGeometryCreated() { return isGeometryCreated;};
	static bool printDebug;
	String Source(){ return source;};

	/// Statistics for newly created elements that may be applied if wished.
	Vector4f defaultTextColor;

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
	void PopFromStack(String elementName);
	/// Pops target element from the interaction/display stack. If force is true it will do it no matter what the element's exitable property says.
	bool PopFromStack(UIElement * element, bool force = false);
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

protected:
	/// Loads from target file, using given root as root-element in the UI-hierarchy.
	static bool LoadFromFile(String filePath, UIElement * intoRoot);

	/// Removes references to target element
	void OnElementDeleted(UIElement * element);

	String source;
	String uiName;
	bool isGeometryCreated;
	bool isBuffered;
	/** If true, there must always (if possible) be one element that is hovered over. 
		What this means is that it will retain previous elements as hovered even if the mouse hovers over invalid elements.
	*/
	bool demandHovered;
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
