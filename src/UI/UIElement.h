/// Emil Hedemalm
/// 2014-04-22
/// Main UI element class. Created first time sometime in 2012 probably.

#ifndef UIELEMENT_H
#define UIELEMENT_H

#include "Graphics/OpenGL.h"
#include "UIAction.h"
#include "UITypes.h"
#include "UI/Input/UIInputResult.h"
#include "NavigateDirection.h"

#include <cstdlib>
#include <ctime>
#include <cmath>
/*
#include "../globals.h"
#include "../Texture.h"
#include "../Square.h"
*/
class Message;
class UserInterface;
class Square;
class Texture;
class GraphicsState;
class TextFont;
class UserInterface;
class UIInput;
class UIToggleButton;
class UILabel;

#include <Util.h>
#include <String/Text.h>
#include <MathLib.h>

namespace UIState {
	/// To enable bit-wise operations.
	const int IDLE		=	0x00000001; /// Flag when nothing happens, or should be. Note that it may or may not be flagged with other active flags.
	const int HOVER		=	0x00000002;	/// Hover flag for when cursor is above an element. Only one element should be in this state at any time.
	const int ACTIVE	=	0x00000004;	/// Active flag for when holding down mouse/cursor button, or for active input-elements.
	const int DISABLED	=	0x00000008; /// Not sure if this is in use...
	const int INVISIBLE	=	0x00000010; /// Not sure if this is in use either...
	const int DIALOGUE	=	0x00000020; /// Used to require further action from the user! This flag is used to easier find which elements are important for when processing them.
};

enum UIFlag {
	VISIBLE		=	0x00000001,
	HOVERABLE	=	0x00000002, 
	ACTIVATABLE	=	0x00000004,
};

enum UIFilter {
	None,
	Visible,
};

class UIElement
{
	friend class UserInterface;
	friend class UIParser;
	friend class InputManager;
	friend class GraphicsManager;
	friend class UIList;
	friend class UIColumnList;
	friend class GMDeleteUI;
	friend class GMSetUIs;
	friend class GMBufferUI;
	friend class GMSetUIp;
	friend class UIAction;
public:
	// UI it belongs to. Usually only need to set this for the root-element for automatic resizing etc.
	UserInterface * ui;

	UIElement();	// Default empty constructor
	virtual ~UIElement();
	/// Copy-cosntructor.
	UIElement(const UIElement & reference);

	void SetRootDefaults(UserInterface * ui);

	UIElement * Parent() {return parent;};

	virtual int CenteredContentParentPosX() const;
	// If parent is e.g. List, available size will vary depending on if scrollbar is present or not.
	virtual int AvailableParentSizeX() const;

	/// Sets the bufferized flag. Should only be called before program shutdown. Ensures less assertions will fail.
	void SetBufferized(bool bufferizedFlag);
	
	/// Creates a deep copy of self and all child elements (where possible).
	virtual UIElement * Copy();
	virtual void CopyChildrenInto(UIElement * copyOfSelf) const;

	/// Callback-function for sub-classes to implement own behaviour to run within the UI-class' code.
	virtual void Proceed();
	virtual void StopInput();

	// Used for handling things like drag-n-drop and copy-paste operations, etc. as willed.
	virtual void ProcessMessage(Message * message);

	/// Sets text, queueing recalculation of the rendered variant. If not force, will ignore for active ui input elements.
	virtual void SetText(CTextr newText, bool force = false);

	/// Recalculates and sets highlighting factors used when rendering the UI (dedicated shader settings)
	void UpdateHighlightColor();
	/** Fetches texture, assuming the textureSource has been set already. Binds and bufferizes, so call only from graphics thread. 
		Returns false if no texture could be find, bind or bufferized. */
	virtual bool FetchBindAndBufferizeTexture();
	/// Called after FetchBindAndBufferizeTexture is called successfully. (may also be called other times). Only called from the graphics thread.
	virtual void OnTextureUpdated();

	/// Public variablessss
	String name;
	/// For when loading from .gui file into ui element.
	String source; 
	/// Used by certain input-elements, such as RadioButtons.
	String displayText;
	/// Divider, used for input-elements.
	Vector2f divider;

	/// Called when this UI is made active (again).
	virtual void OnEnterScope();
	/// Called once this element is no longer visible for any reason. E.g. switching game states to display another UI, or when this or a parent has been popped from the ui.
	virtual void OnExitScope(bool forced);

	/** Called by OS-functions to query if the UI wants to process drag-and-drop files. If so the active element where the mouse is hovering may opt to do magic with it.
		If no magic, or action, is taken, it will return false, at which point the game state should be called to handle general drag-and-drop files.
	*/
	virtual bool HandleDADFiles(List<String> files);

	/// For aggregate types. Creates children elements. Called upon attaching to ui tree so that all settings may be set after initial construction but before contents are finalized.
	virtual void CreateChildren(GraphicsState* graphicsState);
	/// Set default values.
	void Nullify();
	/** Deletes element of target ID and all it's children.
		DO NOTE that this function does NOT try to release any GL buffers!
	*/
	void DeleteElement(int targetID);
	/** Deletes target element if it is found.
		It will also unbufferize and free resources as necessary. Should ONLY be called from the render-thread!
	*/
	bool Delete(UIElement * element);
	/// Deletes all children and content inside.
	virtual void Clear();

	/// Activation functions
	// Hovers over this element. calling OnHover after setting the UIState::HOVER flag.
	virtual UIElement * Hover();
	// Recursive hover function which will return the element currently hovered over using given co-ordinates.
	virtual UIElement * Hover(int mouseX, int mouseY);	// Skickar hover-meddelande till UI-objektet.
	virtual UIElement * Click(int mouseX, int mouseY);						// Skicker Genererar meddelande ifall man tryckt på elementet
	virtual UIElement * Activate(GraphicsState * graphicsState);						// When button is released.
	/// GEtttererrr
	virtual UIElement * GetElement(float & mouseX, float & mouseY);

	/// Returns all children recursively.
	List<UIElement*> AllChildrenR();
	bool HasActivatables();
	UIElement * GetElement(String byName, UIType andType);

	/// Used by e.g. ScrollBarHandle's in order to slide its content according to mouse movement, even when the mouse extends beyond the scope of the element.
	virtual void OnMouseMove(Vector2i activeWindowCoords);

    /** For mouse-scrolling. By default calls it's parent's OnScroll. Returns true if the element did anything because of the scroll.
		The delta corresponds to amount of "pages" it should scroll.
	*/
	virtual bool OnScroll(GraphicsState* graphicsState, float delta);

	/// Returns the root, via parent-chain.
	UIElement * GetRoot();


	/// UI events!
	/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
	virtual void OnInputUpdated(GraphicsState* graphicsState, UIInput * inputElement);
	/// Callback sent to parents once an element is toggled, in order to act upon it. Used by UIMatrix.
	virtual void OnToggled(UIToggleButton * toggleButton);

	/// Yup.
	void InheritNeighbours(UIElement * fromElement);
	/** Suggests a neighbour which could be to the right of this element. 
		Meant to be used for UI-navigation support. The reference element 
		indicates the element to which we are seeking a compatible or optimum neighbour, 
		and should be NULL for the initial call.

		If searchChildrenOnly is true, the call should not recurse to any parents. 
		This is set by special classes such as UIList and UIColumnList when they know
		a certain element should be or contain the correct neighbour element.
	*/
	virtual UIElement * GetUpNeighbour(GraphicsState* graphicsState, UIElement * referenceElement, bool & searchChildrenOnly);
	virtual UIElement * GetRightNeighbour(UIElement * referenceElement, bool & searchChildrenOnly);
	virtual UIElement * GetDownNeighbour(GraphicsState* graphicsState, UIElement * referenceElement, bool & searchChildrenOnly);
	virtual UIElement * GetLeftNeighbour(UIElement * referenceElement, bool & searchChildrenOnly);

	// Is it navigatable?
	bool IsNavigatable();
	
	/// Works recursively.
	UIElement * GetElementClosestTo(Vector3f & position, bool mustBeActivatable);
	UIElement * GetElementClosestToInX(Vector3f & position, bool mustBeActivatable);
	UIElement * GetElementClosestToInY(Vector3f & position, bool mustBeActivatable);

	// Sets the selected flag to false for the element and all beneath it.
	void DeselectAll();

	/** Old shit. Use messages only! !!!
		Function pointer for activation function upon activating the element.
		Function pointer will default to NULL! Argument provided should be the UIElement that was activated ^.^
	*/
//	int (*onActivate)(UIElement * );
	String activationMessage;
	// Action(s) to be performed upon activation. These will be done entirely/only on the Graphics-thread.
	List<UIAction> activationActions; 

	// When it has been pushed to screen or otherwise become relevant.
	String onEnterScope;
	/// Message sent when hovering over an element.
	String onHover;
	String onTrigger; // For "triggering" the element, e.g. pressing Enter for input fields
	String onPop; // Called when hitting "Back" or pressing Escape, usually
	String onForcePop; // Called no matter how the UI is popped, rarer usage?

	String labelText; /// o-o
	/// Called upon hovering on an element. By default queues the string set in onHover to be processed by the message manager and game state.
	virtual void OnHover();

	// Reference functions
	List<UIElement*> GetChildren() { return children; };
	UIElement* GetElementWithID(int elementId);
	UIElement* GetElementByPos(int posX, int posY);
	UIElement* GetActiveElement();
	/// Getter for element by state, where the stateFlag will be bitwise anded (&) to fetch the correct element. See UIState namespace for flag details.
	UIElement* GetElementByState(int stateFlag);
	/// Fetches all elements with the given states, bitwise-and'ed (&)
	bool GetElementsByState(int stateFlags, List<UIElement*> & listToFill);
	/// Checks for visibility, activateability, etc. Also works bit-wise! See UIFlag namespace for flag details.
	UIElement * GetElementByFlag(int uiFlag);
	/// Browses for elements conforming to UIFlag flags.
	bool GetElementsByFlags(int uiFlags, List<UIElement*> & listToFill);
	/// Checks if all flags are true. See UIFlag namespace. Flags can be binary &-ed.
	bool ConformsToFlags(int uiFlags);

	/// Checks if this element is visible, as it will depend on the parent UIs too.
	bool IsVisible();

	/// Gets absolute position and stores them in the pointers' variables, in pixels, relative to upper left corner
	Vector2i GetAbsolutePos();
	void GetAbsolutePos(int * posX, int * posY);
	/// Absolute positions, in pixels, relative to upper left corner
	int GetAbsolutePosX();
	int GetAbsolutePosY();

    /// For debugging, prints info and tree-structure.
	void Print(int level);

	// Movement functions. These take into consideration the parent element and UIType
	void MoveX(int distance);
	void MoveY(int distance);
	void MoveTo(int * x, int * y);

	/// Sets name of the element
	void SetName(const String name);
	const String GetName(){ return name; };
	/// Returns a pointer to specified UIElement
	UIElement * GetElementByName(const String name, UIFilter filter = UIFilter::None);
	/// Tries to fetch element by source, for when loaded from a .gui file straight into an element.
	UIElement * GetElementBySource(String source);
	/// Getterrr
	const UIElement * GetChild(int index);

    /// Returns false if it could nottur.
    bool AddToParent(GraphicsState* graphicsState, String parentName, UIElement * child);
	void SetParent(UIElement *in_parent);

	/** Adds a child/children
		If called with graphicsState non-NULL, it is from the render thread, and updates to the UI may be made.
	*/
	virtual bool AddChildren(GraphicsState* graphicsState, List<UIElement*> children);
	virtual bool AddChild(GraphicsState* graphicsState, UIElement* child); // Sets child pointer to child UI element, NULL if non
	/// Attempts to remove said child from this element. Returns false if it was not a valid child (thus action unnecessary). Does NOT delete anything!
	bool RemoveChild(GraphicsState* graphicsState, UIElement * element);

	/// Checks if the target element is somehow a part of this list. If it is, the function will return the index of the child it is or belongs to. Otherwise -1 will be returned.
	int BelongsToChildIndex(UIElement * ele);


	/// Queues the UIElement to be buffered via the GraphicsManager
	void QueueBuffering();
	/// Bufferizes the UIElement. Should only be called by a thread using a valid rendering context!
	void Bufferize();
	/// Releases resources used by the UIElement. Should only be called by a thread with valid GL context!
	void FreeBuffers();
	/// Rendering
	virtual void Render(GraphicsState & graphicsState);
	virtual void RenderText(GraphicsState& graphicsState);
	/// Called after resize of UI before RenderText.
	virtual void FormatText(GraphicsState * graphicsState); 

	/// Adjusts the UI element size and position relative to new AppWindow size
	void AdjustToWindow(int left, int right, int bottom, int top);
	/// Calls AdjustToWindow for parent's bounds. Will assert if no parent is available.
	void AdjustToParent();

	// Positional variables (pre-resizing)
	enum generalAlignments{
		NULL_ALIGNMENT,
		MAXIMIZE, MAXIMIZED = MAXIMIZE,	// Have maximization as an alignment too to simplify it all
		TOP_LEFT, TOP, TOP_RIGHT,
		LEFT, CENTER, MIDDLE = CENTER, RIGHT,
		BOTTOM_LEFT, BOTTOM, BOTTOM_RIGHT,
		SCROLL_BAR_Y, // Default to the right of the centered content.
	};

	static int GetAlignment(String byName);

	// Free buffers and deletes all border elements and their references.
	void DeleteBorders();

	// Used for borders.
	UIElement * topBorder, 
		* rightBorder,
		* topRightCorner;
	String topBorderTextureSource,
		rightBorderTextureSource,
		topRightCornerTextureSource;

	/// Alignment relative to parent. If this is set all other alignment* variables will be ignored.
	char alignment;
	/// Text contents alignment relative to current size/etc. Defautlt left.
	char textAlignment; 

	/// Used by UILog, could be used by UIList also to over-ride child Y-ratio. If negative, does not override.
	float lineSizeRatio; 
	/// Children of content. - does not include scrollbars. - Used by UIList, may also be used by UILog etc.
	List<UIElement*> contentChildren;

	/// For aggregate types such as UIRadioButtons, etc., set this to true to skip the label before the actual value part. Default false.
	bool noLabel;
	/// Allow scaling depending on parent or AppWindow size?
	bool scalable;

	/// Defines default scalabilty with AppWindow adjustments.
	static const bool DEFAULT_SCALABILITY = true;

	/// Ratio of the texture and UI-size
	float ratio;
	/// Keep minimum-size ratio after scaling up?
	bool keepRatio;

	// Defines the origin (0,0) point when calculating position and size of the element.
	int origin;
	// Alignment relative to parent
	float alignmentX;
	float alignmentY;

	// Size ratios for relative sizing within UI.
	float sizeRatioX;		// Size ratio compared to parent(!)
	float sizeRatioY;		// Size ratio compared to parent(!)

	// If 0, not used. Used to limit stretching beyond some mark to make display nicer on bigger displays.
	int maxWidth = 0, maxHeight = 0;

	// Position, usage depends on the alightment type, scalability, etc.
	Vector3f position; // Vector, try and migrate to them instead, ya?
	int posX, posY;					// Position of the element, relative to parent and origin.
	int sizeX, sizeY;					// Dimensions of the element.
	bool lockSizeY, lockSizeX;
	int left, right, top, bottom;		// Dimensions in screen-space, from 0 to width and 0 to height, 0,0 being in the bottom-left.

	// Position-adjustment variables for advanced UI elements like UIList
	float padding;
	// Padding for text within elements. Default 0, or try 0.1?
	int textPaddingPixels;
	float textSizeY, textSizeX;

	UIType type;							// Type of the UIElement

	/** If true, the element and all its subchildren will be deleted when the element is popped from the UI. 
		Default false. Used for dynamically created UI that are not meant to be re-used.
	*/
#define removeOnPop deleteOnPop
	bool deleteOnPop;
    /// System elements, are treated slightly differently than content elements, like having prioritized rendering and interaction.
    bool isSysElement;
	bool selected;
	bool selectable;
	bool visible;						// Visibility flag
	bool axiomatic;						// If flagged: return straight away if hovered on without looking at children.
	bool hoverable;						// Toggles if the element responds to hover-messages
	bool highlightOnHover;				// Toggles the UI-highlighting for currently hovered UI-elements.
	bool activateable;					// Toggles whether it is CURRENTLY activatable.
	bool highlightOnActive;				// Toggles if the element should highlight when clicking or active. Default is True.
	/// Defines if the element is moveable in runtime, for example slider-handles
	bool moveable;
	
	/// If in the UI-interaction/navigation stack, so that navigation commands don't go outside it or to a parent-node.
	bool inStack;

	/// Says that when pressing ESC (or similar) this menu/element can/will be closed. This should work recursively until an element that is closable is found.
	bool exitable;
	/// Message to be processed when exiting this menu (could be anything, or custom stuff).
	String onExit;


    /// To toggle internal formating if that is already being considered in the design-phase.
	bool formatX, formatY;

	/** Label child element. Adding here since many subclasses use a label, and it is far swifter to store the link in the base class then.
		If this element exists (non-NULL), all text is assumed to be rendered using solely it and not this (parent) element.
		Thusly all messages and parsing regarding text will be applied to this element instead.
		It is assumed that this label is a regular child.
	*/
	UILabel * label;
	
	// Size of text in pixels
	float textSize;
	/// Relative text size to AppWindow size
	float textSizeRatio;


	/// Sets color, but may also update corner/edge elements to use the same color multiplier as well.
	void SetColor(Vector4f color);
	/// Colors for the element.
	Vector4f color;
	/// Colors for the text
	static Vector4f defaultTextColor;
	/// If true, forces all string assignments to upper-case (used with some styles of games)
	static bool forceUpperCase;

	// When clicking on it.
	static float onActiveHightlightFactor; 
	// When hovering on it.
	static float onHoverHighlightFactor;
	// When not hovering, not clicking it.
	static float onIdleHighlightFactor;
	// If toggled (checkbox, radiobutton), additional highlight-factor
	static float onToggledHighlightFactor;

	// Image ID
	int image; // <- wat
	int sprite;	// <- more wat
	int texSizeX;						// Sprite texture size.
	int texSizeY;
	int spriteSize;


	char owner;							// Player who ownes this UI element, only he can interact
	int ID() { return id; };
	String textureSource;	// Name of the texture the UIelement will use
	static String defaultTextureSource;
	String fontSource;
	TextFont * font;
	
	/// Sets disabled-flag.
	void Disable();
	/// Checks state flag for you!
	bool IsDisabled();

	/// For example UIFlag::HOVERABLE, not to be confused with State! State = current, Flags = possibilities
	void SetFlags(UIFlag flag);
	void SetState(int newState);
	/** For example UIState::HOVER, not to be confused with flags! State = current, Flags = possibilities
		For operations controlling the HOVER flag, certain criteria may need to be met in order for the adder to succeed.
	*/
	bool AddState(int state);
	// For sub-classes to adjust children as needed (mainly for input elements).
	virtual void OnStateAdded(int state);
	// See UIState::
	bool HasState(int queryState);
	bool HasStateRecursive(int queryState);
	/// For example UIState::HOVER, if recursive will apply to all children.
	virtual void RemoveState(int state, bool recursive = false);

	// When navigating, either via control, or arrow keys or whatever.
	virtual void Navigate(NavigateDirection direction);

	/// Wether NavigateUI should be enabled when this element is pushed. Default is false.
	bool navigateUIOnPush;
	bool disableNavigateUIOnPop;
	/// If force navigate UI should be applied for this element.
	bool forceNavigateUI; 
	/// Previous state before pushing this UI. 0 for none. 1 for regular, 2 for force.
	int previousNavigateUIState;

	/** In order to override some annoying UI movements, you can specify the following. 
		If an entry exists here target element will be hovered to if it exists before using the general distance-dotproduct nearest algorithm.
	*/
	String leftNeighbourName, rightNeighbourName, upNeighbourName, downNeighbourName;
	/// Pointer-equivalents of the above. Should only be set and used with care!
	UIElement * leftNeighbour, * rightNeighbour, * upNeighbour, * downNeighbour;

	/** Will enable/disable cyclicity of input navigation when this element is pushed. When popped, the next element in the stack will determine cyclicity. */
	bool cyclicY;

	/// When true, re-directs all (or most) keyboard input to the target element for internal processing. Must be subclass of UIInput as extra functions there are used for this.
	bool demandInputFocus;

	/** Used by input-captuing elements. Calls recursively upward until an element wants to respond to the input.
		Returns 1 if it processed anything, 0 if not.
	*/
	virtual UIInputResult OnKeyDown(GraphicsState* graphicsState, int keyCode, bool downBefore);
	/// Used for getting text. This will be local translated language key codes?
	virtual UIInputResult OnChar(int asciiCode);

	/// Called to ensure visibility of target element. First call should be made to the target element with a NULL-argument!
	virtual void EnsureVisibility(GraphicsState* graphicsState, UIElement * element = 0);

	/// Similar to UI, this checks if this particular element is buffered or not.
	bool IsBuffered() const { return isBuffered;};
	bool IsGeometryCreated() const { return isGeometryCreated; };

	// Creates the Square mesh used for rendering the UIElement and calls SetDimensions with it's given values.
	virtual void CreateGeometry();
	virtual void ResizeGeometry();
	void DeleteGeometry();

	Text& GetText() { return text; }
	const Text& GetText() const { return text; }

	// If true, will reduce width or height dynamically so that the texture's aspect ratio is retained when rendering.
	bool retainAspectRatioOfTexture = false;

// Some inherited for UI subclasses
protected:
	// Offset used for internal elements. Mainly used by lists.
	Vector2i pageBegin;

	/// Similar to UI, this checks if this particular element is buffered or not.
	bool isBuffered;
	bool isGeometryCreated;

	/// Called whenever an element is deleted externally. Sub-class in order to properly deal with references.
//	virtual void OnElementDeleted();

	// Text-variable that will contain the version of the text adapted for rendering (inserted newlines, for example).
	Text textToRender;
    /// Re-calculated depending on active UI size and stuff. Reset this to 0 or below in order to re-calculate it.
    float currentTextSizeRatio, previousTextSizeRatio;

    /// Splitting up the rendering.
    virtual void RenderSelf(GraphicsState & graphicsState);
    virtual void RenderChildren(GraphicsState & graphicsState);
	virtual void RenderBorders(GraphicsState & graphicsState);

	virtual UIElement * CreateBorderElement(String textureSource, char alignment);

	// Works recursively.
	void RemoveFlags(UIFlag flag);

	// Graphical members, relevant to buffering/rendering
	Square * mesh;		// Mesh Entity for this element
	Texture * texture;	// Texture to be mapped to the mesh
	GLuint vboBuffer;	// GL Vertex Buffer ID
	int vboVertexCount;	// Vertex buffer Entity vertex count
	float zDepth;		// Depth value for the element

	// Hierarchal pointers
	UIElement * parent;					// Pointer to parent UI element
	List<UIElement*> children;					// Pointer-array to a child.
	List<UIElement*> borderElements;
	bool childrenCreated; // For complex types, set to true once created (false by default).

	/// Id stuff
	int id;								// ID of the UI, used for changing properties in the element.
	static int idEnumerator;

private:

	// Text that will be rendered
	Text text;

	// Current state of the UI Element, See UIState above.
	int state;

	/// GL specific stuff
	GLuint vertexArray;
};

//void DrawText(glfont::GLFont * i_myfont, UIElement * i_element, char * i_text, float i_size);
//int FormatText(glfont::GLFont * myfont, const UIElement * i_element, char * i_text, float i_size);

class UILabel : public UIElement{
public:
	UILabel(String name = "");
	virtual ~UILabel();
};

class UISliderHandle : public UIElement{
public:
	UISliderHandle();
	virtual ~UISliderHandle();
private:
};

class UISlider : public UIElement {
public:
	UISlider();
	virtual ~UISlider();
	/** Sets slider level by adjusting it's child's position.
		Values should be within the range [0.0,1.0]. **/
	void SetLevel(float level);
	/// Returns the slider's level
	float GetLevel();
private:
	// Progress of scrollbar/slider, for instance
	double progress;
};

const char MSG_BUTTON_OK		= 0x01;
const char MSG_BUTTON_CANCEL	= 0x02;
const char MSG_INPUTFIELD		= 0x04;

// Creation functions
UIElement* createUIMessageBox(char * title = 0, char * body = 0, int settings = 0);
// Stuff
void appendTextToMessagebox(const char * i_message);


#endif
