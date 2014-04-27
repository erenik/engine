// Emil Hedemalm
// 2013-07-03, Linuxifying!

#include <GL/glew.h>

#include "UIElement.h"
#include "UITypes.h"

#include "OS/Sleep.h"

#ifdef WINDOWS
    #include <Windows.h>
#endif

#include "Graphics/GraphicsManager.h"
#include "../Input/InputManager.h"
#include "TextureManager.h"
#include "Graphics/Fonts/Font.h"
#include "Mesh/Square.h"
#include "UserInterface.h"
#include "GraphicsState.h"
#include "MathLib/Rect.h"

extern InputManager input;
extern UserInterface ui[MAX_GAME_STATES];

int UIElement::idEnumerator = 0;
String UIElement::defaultTextureSource; //  = "80Gray50Alpha.png";
Vector4f UIElement::defaultTextColor = Vector4f(1,1,1,1);

/// Called when this UI is made active (again).
void UIElement::OnEnterScope(){
	// Do nothing in general.
	for (int i = 0; i < childList.Size(); ++i)
		childList[i]->OnEnterScope();
}
/// Called once this element is no longer visible for any reason. E.g. switching game states to display another UI.
void UIElement::OnExitScope(){
	// Do nothing in general.
	for (int i = 0; i < childList.Size(); ++i)
		childList[i]->OnExitScope();
}

/** Called by OS-functions to query if the UI wants to process drag-and-drop files. If so the active element where the mouse is hovering may opt to do magic with it.
	If no magic, or action, is taken, it will return false, at which point the game state should be called to handle general drag-and-drop files.
*/
bool UIElement::HandleDADFiles(List<String> files)
{
	if (parent)
		return parent->HandleDADFiles(files);
	return false;
}

/// Set default values.
void UIElement::Nullify(){
	/// ID
	id = idEnumerator++;
	/// Hierarchy
	parent = NULL;
	ui = NULL;

	// Graphical properties
	mesh = NULL;
	texture = NULL;
	vboBuffer = -1;
	vboVertexCount = 0;
	zDepth = 0;

	alignment = NULL_ALIGNMENT;	// Alignment relative to parent
	scalable = DEFAULT_SCALABILITY;		// Allow scaling depending on window size?
	ratio = 1.0;
	keepRatio = true;

	alignmentX = 0.5;
	alignmentY = 0.5;

	sizeRatioX = 1.0f;		// Size ratio compared to parent(!)
	sizeRatioY = 1.0f;		// Size ratio compared to parent(!)

	posX = posY = 0;
	sizeX = sizeY = 0;
	type = UIType::BASIC;

	padding = 0;

	state = UIState::IDLE;
	selected = false;
	toggled = false;
	visible = true;

	removeOnPop = false;

    isSysElement = false;

	hoverable = false;
	highlightOnHover = true;
	highlightOnActive = true;
	axiomatic = false;
	selectable = false;
	activateable = false;
	moveable = false;


	/// Wether NavigateUI should be enabled when this element is pushed.
	navigateUIOnPush = false;
	/// If force navigate UI should be applied for this element.
	forceNavigateUI = false;
	/// Previous state before pushing this UI. 0 for none. 1 for regular, 2 for force.
	previousNavigateUIState = 0;

	/// Exit-properties.
	exitable = true;
	onExit = "NullMessage";

	// Text.
	label = NULL;
	textSizeRatio = 1.0f;
	currentTextSizeRatio = -1.0f;

	// Function pointer for activation
	onActivate = NULL;

	fontSource = TextFont::defaultFontSource;
	font = NULL;

	/** Will enable/disable cyclicity of input navigation when this element is pushed. When popped, the next element in the stack will determine cyclicity. */
	cyclicY = true;

	/// When true, re-directs all (or most) keyboard input to the target element for internal processing when active. Must be subclass of UIInput as extra functions there are used for this.
	demandInputFocus = false;

	/// Neighbour pointers
	upNeighbour = rightNeighbour = leftNeighbour = downNeighbour = NULL;

	/// GL related.
	vertexArray = -1;

	color = Vector4f(1,1,1,1);
}

UIElement::UIElement(){
	Nullify();
}

/// Copy-cosntructor.
UIElement::UIElement(const UIElement & ref){
	Nullify();
	zDepth = ref.zDepth;
	alignment = ref.alignment;	// Alignment relative to parent
	scalable = ref.scalable;		// Allow scaling depending on window size?
	ratio = ref.ratio;
	keepRatio = ref.keepRatio;
	alignmentX = ref.alignmentX;
	alignmentY = ref.alignmentY;
	sizeRatioX = ref.sizeRatioX;
	sizeRatioY = ref.sizeRatioY;
	type = ref.type;
	hoverable = ref.hoverable;
	axiomatic = ref.axiomatic;
	selectable = ref.selectable;
	activateable = ref.activateable;
	visible = ref.visible;
	moveable = ref.moveable;
	textSizeRatio = ref.textSizeRatio;
	// Function pointer for activation
	onActivate = ref.onActivate;
}

UIElement::~UIElement()
{
//	std::cout<<"\nUIElement destructor";
	/// Hierarchy
	parent = NULL;
	/// Use for-loop instead of crazy recursion for deallocating the children
	childList.ClearAndDelete();

	/// Deallocate texture and mesh if needed, as well as vbo, we should never do that here though!
	assert(vboBuffer == -1 && "vboBuffer not released in UIElement::~UIElement()!");
	/// Textures will be deallocated by the texture manager...
	/// But take care of the mesh!
	if (mesh){
		delete mesh;
		mesh = NULL;
	}
	/// Inform the input manager that we delete the element.
	// Input.OnElementDeleted(this);
}

/// Callback-function for sub-classes to implement own behaviour to run within the UI-class' code.
void UIElement::Proceed()
{
}

/// Sets text, queueing recalculation of the rendered variant.
void UIElement::SetText(Text newText, bool force){
	/// Check that it's not currently being edited. If so cancel this setting.
	if (this->demandInputFocus && state & UIState::ACTIVE && !force){
		return;
	}
	this->text = newText;
	/// Reset text-variables so that they are re-calculated before rendering again.
	currentTextSizeRatio = -1.0f;
}


void UIElement::DeleteElement(int targetID){
	UIElement* target = NULL;
	childList.ClearAndDelete();
}

/** Deletes target element if it is found.
	It will also unbufferize and free resources as necessary. Should ONLY be called from the render-thread!
*/
bool UIElement::Delete(UIElement * element){
	/// If found it, delete it.
	if (childList.Exists(element))
	{
		childList.Remove(element);
		/// Free GL buffers and deallocate it!
		element->FreeBuffers();
		element->DeleteGeometry();
		delete element;
		return true;
	}
	/// If not, continue recursive among all children.
	else {
		for (int i = 0; i < childList.Size(); ++i){
			/// Remove it
			if (childList.Remove(element))
				return true;
		}
	}
	return false;
}

/// Deletes all children and content inside.
void UIElement::Clear() {
	assert(false && "This should only be accessed inside those subclasses that actually use it, like UIList!");
};

//******************************************************************************//
// Activation functions
//******************************************************************************//

// Returns true once the highest-level appropriate element has been selected,
// or it has been determined that the selected child was not the active one.
// A value of true will thus make the algorithm return true and ending it's calculations.
UIElement* UIElement::Hover(float & mouseX, float & mouseY)
{

	UIElement* result = NULL;

	// Don't process invisible UIElements, please.
	if (visible == false)
		return false;

	// Check if the mouse is outside the element's boundaries.
	if (mouseX > right || mouseX < left ||
		mouseY > top || mouseY < bottom){
		// Return false if we are outside of the boundaries,
		// since we haven't found the selected element.
		RemoveFlags(UIState::HOVER);
	//	if(child != NULL)
		return NULL;
	}

	// Check axiomaticness
	if (axiomatic){
		if (hoverable){
			state |= UIState::HOVER;
			std::cout<<"\nAXIOMATICNESS?! "<<name<<" is hover.";
			return this;
		}
		return NULL;
	}

	// Alright, the mouse is inside this element!
	// Do we have children?
	for (int i = childList.Size()-1; i >= 0; --i){
	    UIElement * child = childList[i];
	    if (!child->visible)
            continue;
		result = child->Hover(mouseX, mouseY);
		// If so, check if they are the active element.
		if (result != NULL){
			// The active element has been found further down the tree,
			// so we can return true.
			state &= ~UIState::HOVER;
			return result;
		}
	}
	// If the Element is not hoverable, return now.
	if (!hoverable)
		return NULL;

	// If the UIElement is being clicked (left mouse button)
	// Return and don't change the StateMan.
	if (state == UIState::ACTIVE)
		return this;

	// If the UIElement is already depressed, don't highlight it.
//	if (selected)
//		return false;

	// The mouse is inside this element,
	// and our children (if any) were not the active Entity.
	// This means that this element must be the active Entity!
	state |= UIState::HOVER;
//	std::cout<<"\nElelelelement "<<name<<" is hover.";
	/*if(parent){
		if(type== UI_TYPE_BASIC || type== UI_TYPE_LABEL)
			parent->state = UIState::HOVER;
	}*/
	return this;
}


// Returns true once the highest-level appropriate element has been found.
// No co-ordinates are required since we will instead require the element to already
// be highlighted/hovered above.
UIElement* UIElement::Click(float & mouseX, float & mouseY)
{
	UIElement* result = 0;
	// Don't process invisible UIElements, please.
	if (visible == false)
		return false;

	// Check if the mouse is outside the element's boundaries.
	if (mouseX > right || mouseX < left ||
		mouseY > top || mouseY < bottom){
			// Return false if we are outside of the boundaries,
			// since we haven't found the selected element.
			state = UIState::IDLE;
			//	if(child != NULL)
			return NULL;
	}

	// Check axiomaticness (direct-activation without further processing)
	if (axiomatic){
		if (activateable){
			state |= UIState::ACTIVE;
			return this;
		}
		return NULL;
	}

	// Alright, the mouse is inside this element!
	// Do we have children?
	for (int i = childList.Size()-1; i >= 0; --i){
		UIElement * child = childList[i];
	    if (!child->visible)
            continue;
		result = child->Click(mouseX, mouseY);
		if (result != NULL){
			// The active element has been found further down the tree,
			// so we can return true.
			state = UIState::IDLE;
			return result;
		}
	}
	// Check the element's StateMan. If it is hovered over, we've found it.
	if (this->activateable && state & UIState::HOVER){
		state |= UIState::ACTIVE;
		return this;
	}
	// If not, return false, since we haven't foun the right element.
	return NULL;
}

// Returns a non-0 message once the highest-level appropriate element has been found.
// No co-ordinates are required since we will instead require the element to already
// clicked.
UIElement* UIElement::Activate(){
	UIElement* result = 0;
	// Don't process invisible UIElements, please.
	if (visible == false)
		return 0;
	// Alright, the mouse is inside this element!
	// Do we have children?
	for (int i = childList.Size()-1; i >= 0; --i){
		UIElement * child = childList[i];
	    if (!child->visible)
            continue;
		result = child->Activate();
		if (result != 0){
			// The active element has been found further down the tree,
			// so we return it's message.
			state = UIState::IDLE;
			return result;
		}
	}
	// Check the element's state. If it is active, we've found it. Dialogues work too, or?
	if (state & UIState::ACTIVE){
		if (type == UIType::INPUT_FIELD || type == UIType::TEXT_FIELD)
			state |= UIState::ACTIVE; // Input-fields remain active as they begin input upon activation!
		/// Just unflag the active state, try ensure that the hover-state remains!
		else
			state &= ~UIState::ACTIVE;
		// Now return our message!
		if (selectable == true){
			if (type== UIType::CHECKBOX){
				selected = !selected;
			}
			else if (type== UIType::RADIOBUTTON){
				parent->DeselectAll();
				selected = true;
			}
			else {
			//	selected = true;
			}
		}
		else {
			// Element not activatable
		}
		return this;
	}
	// If not, return 0, since we haven't found the right element.
	return 0;
}

/// Wosh.
UIElement * UIElement::GetElement(float & mouseX, float & mouseY){
    UIElement* result = 0;
	// Don't process invisible UIElements, please.
	if (visible == false)
		return false;

	// Check if the mouse is outside the element's boundaries.
	if (mouseX > right || mouseX < left ||
		mouseY > top || mouseY < bottom)
    {
        return NULL;
	}
    // Do we have children?
	for (int i = 0; i < childList.Size(); ++i){
		UIElement * child = childList[i];
	    if (!child->visible)
            continue;
		result = child->GetElement(mouseX, mouseY);
		if (result != NULL){
			return result;
		}
	}
	return this;
}

UIElement * UIElement::GetElement(String byName, int andType)
{
	if (type == andType)
	{
		if (name == byName)
			return this;
	}
	for (int i = 0; i < childList.Size(); ++i)
	{
		UIElement * child = childList[i];
		UIElement * found = child->GetElement(byName, andType);
		if (found)
			return found;
	}
	return NULL;
}


/// For mouse-scrolling. By default calls it's parent's OnScroll.
bool UIElement::OnScroll(float delta){
    if (parent)
        return parent->OnScroll(delta);
    return false;
}


/// Returns the root, via parent-chain.
UIElement * UIElement::GetRoot()
{
	if(parent)
		return parent->GetRoot();
	return this;
}

/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UIElement::OnInputUpdated(UIInput * inputElement)
{
	if (parent)
		parent->OnInputUpdated(inputElement);
}

/// Callback sent to parents once an element is toggled, in order to act upon it. Used by UIMatrix.
void UIElement::OnToggled(UICheckBox * box)
{
	if (parent)
		parent->OnToggled(box);
}

void UIElement::InheritNeighbours(UIElement * fromElement){
	if (!leftNeighbourName.Length())
		leftNeighbourName = fromElement->leftNeighbourName;
    if (!rightNeighbourName.Length())
		rightNeighbourName = fromElement->rightNeighbourName;
	if (!downNeighbourName.Length())
		downNeighbourName = fromElement->downNeighbourName;
	if (!upNeighbourName.Length())
		upNeighbourName = fromElement->upNeighbourName;
}


/// Suggests a neighbour which could be to the right of this element. Meant to be used for UI-navigation support. The reference element indicates the element to which we are seeking a compatible or optimum neighbour.
UIElement * UIElement::GetUpNeighbour(UIElement * referenceElement){
	UIElement * element = NULL;
	// Ok, we are the one,
	if (referenceElement == NULL){
		referenceElement = this;
	}
	/// If we have a pointer, just use it!
	if (upNeighbour){
		element = upNeighbour;
	}
	/// If not, fetch it if possible.
	else {
		/// Check if we got a preferred neighbour.
		if (upNeighbourName.Length())
			element = this->GetRoot()->GetElementByName(upNeighbourName);
		/// If still haven't found a decent one, consult our parent.
		if (!element && parent){
			element = parent->GetUpNeighbour(referenceElement);
		}
	}
	/// All-right, so we found a suitable neighbour hopefully. Make sure that it is hoverable?
	if (element && !element->hoverable){
		/// First query if the element has any valid hoverable children, if so select them.
		UIElement * childElement = element->GetElementByFlag(UIFlag::ACTIVATABLE);
		if (childElement)
			element = childElement;
		/// If no valid children could be found, continue searching.
		else
			element = element->GetUpNeighbour(NULL);
	}
	return element;
}

/// Suggests a neighbour which could be to the right of this element. Meant to be used for UI-navigation support. The reference element indicates the element to which we are seeking a compatible or optimum neighbour.
UIElement * UIElement::GetRightNeighbour(UIElement * referenceElement)
{
	UIElement * element = NULL;
	// Ok, we are the one,
	if (referenceElement == NULL){
		referenceElement = this;
	}
	/// If we have a pointer, just use it!
	if (rightNeighbour){
		element = rightNeighbour;
	}
	/// If not, fetch it if possible.
	else {
		/// Check if we got a preferred neighbour.
		if (rightNeighbourName.Length())
			element = this->GetRoot()->GetElementByName(rightNeighbourName);
		/// If still haven't found a decent one, consult our parent.
		if (!element && parent){
			element = parent->GetRightNeighbour(referenceElement);
		}
	}
	/// All-right, so we found a suitable neighbour hopefully. Make sure that it is hoverable?
	if (element && !element->hoverable){
		/// First query if the element has any valid hoverable children, if so select them.
		UIElement * childElement = element->GetElementByFlag(UIFlag::HOVERABLE);
		if (childElement)
			element = childElement;
		/// If no valid children could be found, continue searching.
		else
			element = element->GetRightNeighbour(NULL);
	}
	return element;
}

/// Suggests a neighbour which could be to the right of this element. Meant to be used for UI-navigation support. The reference element indicates the element to which we are seeking a compatible or optimum neighbour, and should be NULL for the initial call.
UIElement * UIElement::GetDownNeighbour(UIElement * referenceElement)
{
	UIElement * element = NULL;
	// Ok, we are the one,
	if (referenceElement == NULL){
		referenceElement = this;
	}
	/// If we have a pointer, just use it!
	if (downNeighbour){
		element = downNeighbour;
	}
	/// If not, fetch it if possible.
	else {
		/// Check if we got a preferred neighbour.
		if (downNeighbourName.Length())
			element = this->GetRoot()->GetElementByName(downNeighbourName);
		/// If still haven't found a decent one, consult our parent.
		if (!element && parent){
			element = parent->GetDownNeighbour(referenceElement);
		}
	}
	/// All-right, so we found a suitable neighbour hopefully. Make sure that it is hoverable?
	if (element && !element->hoverable){
		/// First query if the element has any valid hoverable children, if so select them.
		UIElement * childElement = element->GetElementByFlag(UIFlag::HOVERABLE);
		if (childElement)
			element = childElement;
		/// If no valid children could be found, continue searching.
		else
			element = element->GetDownNeighbour(NULL);
	}
	return element;

}
/// Suggests a neighbour which could be to the right of this element. Meant to be used for UI-navigation support. The reference element indicates the element to which we are seeking a compatible or optimum neighbour, and should be NULL for the initial call.
UIElement * UIElement::GetLeftNeighbour(UIElement * referenceElement){
	UIElement * element = NULL;
	// Ok, we are the one,
	if (referenceElement == NULL){
		referenceElement = this;
	}
	/// If we have a pointer, just use it!
	if (leftNeighbour){
		element = leftNeighbour;
	}
	/// If not, fetch it if possible.
	else {
		/// Check if we got a preferred neighbour.
		if (leftNeighbourName.Length())
			element = this->GetRoot()->GetElementByName(leftNeighbourName);
		/// If still haven't found a decent one, consult our parent.
		if (!element && parent){
			element = parent->GetLeftNeighbour(referenceElement);
		}
	}
	/// All-right, so we found a suitable neighbour hopefully. Make sure that it is hoverable?
	if (element && !element->hoverable){
		/// First query if the element has any valid hoverable children, if so select them.
		UIElement * childElement = element->GetElementByFlag(UIFlag::HOVERABLE);
		if (childElement)
			element = childElement;
		/// If no valid children could be found, continue searching.
		else
			element = element->GetLeftNeighbour(NULL);
	}
	return element;
}


// Sets the selected flag to false for all children.
void UIElement::DeselectAll(){
	for (int i = 0; i < childList.Size(); ++i){
		childList[i]->DeselectAll();
	}
	selected = false;
}

UIElement* UIElement::GetElementByPos(int i_posX, int i_posY){

	UIElement* result = NULL;
	// Don't process invisible UIElements, please.
	if (visible == false)
		return false;
	// Check if the mouse is outside the element's boundaries.
	if (i_posX < posX || i_posX > posX + sizeX ||
		i_posY < posY || i_posY > posY + sizeY){
		// Return false if we are outside of the boundaries,
		// since we haven't found the selected element.
		return NULL;
	}

	// Alright, the mouse is inside this element!
	// Do we have children?
	for (int i = 0; i < childList.Size(); ++i){
		result = childList[i]->GetElementByPos((int)(i_posX - posX), (int)(i_posY - posY));
		// If so, check if they are the active element.
		if (result != NULL){
			// The active element has been found further down the tree,
			// so we can return true.
			return result;
		}
	}
	// The mouse is inside this element,
	// and our children (if any) were not the active Entity.
	// This means that this element must be the active Entity!
	return this;
}

UIElement* UIElement::GetActiveElement(){
	if (state & UIState::ACTIVE)
		return this;
	UIElement * result = NULL;
	for (int i = 0; i < childList.Size(); ++i){
		result = childList[i]->GetActiveElement();
		if (result)
			return result;
	}
	return NULL;
}

/// Getter for element by state, where the stateFlag will be bitwise anded (&) to fetch the correct element.
UIElement* UIElement::GetElementByState(int stateFlag){
	if (state & stateFlag)
		return this;
	UIElement * result = NULL;
	for (int i = 0; i < childList.Size(); ++i){
		if (!childList[i]->visible)
			continue;
		result = childList[i]->GetElementByState(stateFlag);
		if (result)
			return result;
	}
	return NULL;
}

/// Fetches all elements with the given states, bitwise-and'ed (&)
bool UIElement::GetElementsByState(int stateFlags, List<UIElement*> & listToFill){
	// Check stuff
	if (state & stateFlags)
		listToFill.Add(this);
	UIElement * result = NULL;
	for (int i = 0; i < childList.Size(); ++i){
		/// Skip invisible children by default.
		if (!childList[i]->visible)
			continue;
		childList[i]->GetElementsByState(stateFlags, listToFill);
	}
	return true;
}

/// Checks for visibility, activateability, etc. Also works bit-wise!
UIElement * UIElement::GetElementByFlag(int uiFlags){
	// Check stuff
	if (ConformsToFlags(uiFlags))
		return this;
	UIElement * result = NULL;
	for (int i = 0; i < childList.Size(); ++i){
		/// Skip invisible children by default.
		if (!childList[i]->visible)
			continue;
		result = childList[i]->GetElementByFlag(uiFlags);
		if (result)
			return result;
	}
	return NULL;
}

bool UIElement::GetElementsByFlags(int uiFlags, List<UIElement*> & listToFill){
	// Check stuff
	if (ConformsToFlags(uiFlags))
		listToFill.Add(this);
	UIElement * result = NULL;
	for (int i = 0; i < childList.Size(); ++i){
		/// Skip invisible children by default.
		if (!childList[i]->visible)
			continue;
		childList[i]->GetElementsByFlags(uiFlags, listToFill);
	}
	return true;
}

/// Checks if all flags are true. See UIFlag namespace. Flags can be binary &-ed.
bool UIElement::ConformsToFlags(int uiFlags){
	bool isThisOne = true;
	if (uiFlags & UIFlag::VISIBLE)
		isThisOne = isThisOne & visible;
	if (uiFlags & UIFlag::HOVERABLE)
		isThisOne = isThisOne & hoverable;
	if (uiFlags & UIFlag::ACTIVATABLE)
		isThisOne = isThisOne & activateable;
	if (state & UIState::DISABLED)
		return false;
	if (isThisOne)
		return true;
	return false;
}

UIElement* UIElement::GetElementWithID(int elementID){
	if (id == elementID)
		return this;
	UIElement * result = NULL;
	for (int i = 0; i < childList.Size(); ++i){
		result = childList[i]->GetElementWithID(elementID);
		if (result)
			return result;
	}
	return NULL;
}

bool UIElement::IsVisible(int elementId){
	// If this is the sought ID: return visibility StateMan.
	if (elementId == id){
		return visible;
	}

	// Don't process invisible UIElements, please.
	if (visible == false)
		return false;

	// Do we have children?
	for (int i = 0; i < childList.Size(); ++i){
		// If so, check if they are the active element.
		if (childList[i]->IsVisible(elementId) == true){
			// The active element has been found further down the tree,
			// so we can return true.
			return true;
		}
	}
	// Nothing procced, return false.
	return false;
}

/// Gets absolute position and stores them in the pointers' variables, in pixels, relative to upper left corner
void UIElement::GetAbsolutePos(int * i_posX, int * i_posY){
	int resX = (int) floor(posX+0.5f);
	int resY = (int) floor(posY+0.5f);
	UIElement* pp = parent;
	// While parent pointer is non-NULL
	while(pp){
		// Increment result
		resX += (int)floor(pp->posX + 0.5f);
		resY += (int)floor(pp->posY + 0.5f);
		pp = pp->parent;
	}
	// Increment positions
	if (i_posX)
		(*i_posX) = resX;
	if (i_posY)
		(*i_posY) = resY;
}
/// Absolute positions, in pixels, relative to upper left corner
int UIElement::GetAbsolutePosX(){
	int result = (int)posX;
	if (parent)
		result += parent->GetAbsolutePosX();
	return result;
}
int UIElement::GetAbsolutePosY(){
	int result = (int)posY;
	if (parent)
		result += parent->GetAbsolutePosY();
	return result;
}

#include <iomanip>

/// For debugging, prints info and tree-structure.
void UIElement::Print(int level){
    std::cout<<"\n";
    if (!visible)
        std::cout<<"Not visible ";
    std::cout<<std::setfill(' ')<<std::setw(level)<<level<<" - "<<name<<", "<<textureSource;
    if (!visible)
        return;
    for (int i = 0; i < childList.Size(); ++i)
        childList[i]->Print(level+1);
}

// Movement functions. These take into consideration the parent element and UIType
void UIElement::MoveX(int distance){
	switch(type){
		case UIType::SLIDER_HANDLE:
			posX += distance;
			if (posX > parent->sizeX - sizeX)
				posX = (float)parent->sizeX - sizeX;
			if (posX < 0)
				posX = 0;
			break;
		case UIType::SCROLL_HANDLE:
			break;
		default:
			posX += distance;
	}
}

void UIElement::MoveY(int distance){
	switch(type){
		case UIType::SLIDER_HANDLE:
			break;
		case UIType::SCROLL_HANDLE:
			posY += distance;
			if (posY > parent->sizeY - sizeY)
				posY = (float)parent->sizeY - sizeY;
			if (posY < 0)
				posY = 0;
			break;
		default:
			posY += distance;
	}
}

// Moves to specified position. Takes addresses as parameters for given values in order to confirm validity.
// Ignores value if pointer is NULL
void UIElement::MoveTo(int * x, int * y){
	int absX = 0, absY = 0;
	GetAbsolutePos(&absX, &absY);
	if (x)
		posX = *x - (absX - posX);
	if (y)
		posY = *y - (absY - posY);
	// If certain UITypes, constrain movement to parent node
	switch(type){
		case UIType::SLIDER_HANDLE:{
			posX -= sizeX/2;
			if (posX > parent->sizeX - sizeX)
				posX = (float)parent->sizeX - sizeX;
			if (posX < 0)
				posX = 0;
			break;
		}
		case UIType::SCROLL_HANDLE: {
			posY -= sizeY/2;
			if (posY > parent->sizeY - sizeY)
				posY = (float)parent->sizeY - sizeY;
			if (posY < 0)
				posY = 0;
			break;
		}
	}
}





/// Sets name of the element
void UIElement::SetName(const String i_name){
	name = i_name;
}

/// Returns a pointer to specified UIElement
UIElement * UIElement::GetElementWithName(String i_name){
	if (i_name == name)
		return this;
	UIElement * result;
	for (int i = 0; i < childList.Size(); ++i){
        UIElement * child = childList[i];
        assert(child && "NULL-child? wtf?");
   //     std::cout<<"\nChild name: "<<child->name;
		String childName = childList[i]->name;
		result = childList[i]->GetElementWithName(i_name);
		if (result)
			return result;
	}
	return NULL;
}

/// Tries to fetch element by source, for when loaded from a .gui file straight into an element.
UIElement * UIElement::GetElementBySource(String i_source)
{
	if (i_source == source ||
		i_source.Contains(source) ||
		source.Contains(i_source))
		return this;
	UIElement * result = NULL;
	for (int i = 0; i < childList.Size() && result == NULL; ++i)
	{
		result = childList[i]->GetElementBySource(i_source);
	}
	return result;
}

/// Getterrr
const UIElement * UIElement::GetChild(int index){
	assert(index >= 0 && index < childList.Size());
	return childList[index];
}

// Structurization
void UIElement::AddChild(UIElement *in_child){
    assert(in_child);
    if (in_child == NULL)
        return;
	if (in_child->name.Length() == 0)
	{
		in_child->name = name + "Child";
	}
	/// Replace name if already taken.
	int i = 1;
	String baseName = in_child->name;
	while(GetRoot()->GetElementByName(in_child->name)){
		++i;
		in_child->name = baseName +"_"+ String::ToString(i);
	}

	childList.Add(in_child);
	// Set it's parent to this.
	in_child->parent = this;
}


bool UIElement::AddToParent(String parentName, UIElement * child){
    UIElement * padre = GetElementWithName(parentName);
    if (padre){
        padre->AddChild(child);
        return true;
    }
    return false;
}

void UIElement::SetParent(UIElement *in_parent){
	parent = in_parent;
}

/// Queues the UIElement to be buffered via the GraphicsManager
void UIElement::QueueBuffering(){
	Graphics.QueueMessage(new GMBufferUI(this));
}

/// Bufferizes the UIElement mesh into the vbo buffer
void UIElement::Bufferize(){
	/// Check for mesh-Entity
	if (mesh){
		if (!name)
			;//std::cout<<"\nBuffering un-named UIElement.";
		else
			;//std::cout<<"\nBuffering UIElement \""<<name<<"\"";

		/// Create VAO

		// Check for errors before we proceed.
		GLuint err2 = glGetError();
		if (err2 != GL_NO_ERROR)
			std::cout<<"\nINFO: Old GLError in UIElement::Bufferize";

		if (Graphics.GL_VERSION_MAJOR >= 3){
		// Generate VAO and bind it straight away if we're above GLEW 3.0
			if (vertexArray == -1)
				glGenVertexArrays(1, &vertexArray);
			glBindVertexArray(vertexArray);
		}

		// Check for errors before we proceed.
		GLuint err = glGetError();
		if (err != GL_NO_ERROR)
			std::cout<<"\nGLError buffering UI in UIElement::Bufferize";

		// Count total vertex/texture point pairs without any further optimization.
		unsigned int vertexCount = mesh->faces * 3;
		unsigned int floatsPerVertex = 3 + 3 + 2;  // Pos + Normal + UV
		unsigned int vboVertexDataLength = vertexCount * floatsPerVertex;
		float * vboVertexData = new float[vboVertexDataLength];

		// Generate And bind The Vertex Buffer
		/// Check that the buffer isn't already generated
		if (vboBuffer == -1)
			glGenBuffers(1, &vboBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vboBuffer);

		// Load all data in one big fat array, yo Data
		unsigned int vertexDataCounted = 0;
		// Reset vertex count
		vboVertexCount = 0;
		for (int i = 0; i < mesh->faces; ++i){
			// Count vertices in all triangles
			for (int j = 0; j < 3; ++j){
				int currentVertex = mesh->face[i].vertex[j];
				// Position
				vboVertexData[vertexDataCounted + 0] = mesh->vertex[currentVertex].x;
				vboVertexData[vertexDataCounted + 1] = mesh->vertex[currentVertex].y;
				vboVertexData[vertexDataCounted + 2] = mesh->vertex[currentVertex].z;
				// Normal
				int currentNormal = mesh->face[i].normal[j];
				vboVertexData[vertexDataCounted + 3] = mesh->normal[currentNormal].x;
				vboVertexData[vertexDataCounted + 4] = mesh->normal[currentNormal].y;
				vboVertexData[vertexDataCounted + 5] = mesh->normal[currentNormal].z;
				// UV
				int currentUV = mesh->face[i].uv[j];
				vboVertexData[vertexDataCounted + 6] = mesh->u[currentUV];
				vboVertexData[vertexDataCounted + 7] = mesh->v[currentUV];
				vertexDataCounted += 8;
				++vboVertexCount;
			}
		}
		if ((unsigned int)vertexDataCounted > vboVertexDataLength)
			std::cout<<"\nERROR: vertexDataCounted exceeds vertxCount * floatsPerVertex";

		// Enter the data too, fucking moron Emil-desu, yo!
		glBufferData(GL_ARRAY_BUFFER, vertexDataCounted*sizeof(float), vboVertexData, GL_STATIC_DRAW);

		// Check for errors before we proceed.
		GLuint error = glGetError();
		if (error != GL_NO_ERROR)
			std::cout<<"\nGLERROR: "<<error<<" in UIElement::Bufferize";
		delete[] vboVertexData;
		vboVertexData = NULL;
	}

	;//std::cout<<" Buffering successful.";

	// Bufferize children too!
	for (int i = 0; i < childList.Size(); ++i){
		childList[i]->Bufferize();
	}

	// Bufferize textures straight away if needed too
	if (this->texture && texture->glid == -1)
		TexMan.BufferizeTexture(texture);

}
/// Releases resources used by the UIElement. Should only be called by a thread with valid GL context!
void UIElement::FreeBuffers(){
	if (vboBuffer){
		glDeleteBuffers(1, &vboBuffer);
		vboBuffer = -1;
	}
	if (vertexArray){
		glDeleteVertexArrays(1, &vertexArray);
		vertexArray = -1;
	}
	for (int i = 0; i < childList.Size(); ++i){
		childList[i]->FreeBuffers();
	}
}


/// Render, public function that calls internal render functions.
void UIElement::Render(GraphicsState& graphics){
    // Push matrices
	Matrix4d tmp = graphics.modelMatrixD;

    // Render ourself and maybe children.
    RenderSelf(graphics);
    if (childList.Size())
        RenderChildren(graphics);

	// Pop matrices
	graphics.modelMatrixF = graphics.modelMatrixD = tmp;
}

/// Sets disabled-flag.
void UIElement::Disable()
{
	state |= UIState::DISABLED;
}

/// Checks state flag for you!
bool UIElement::IsDisabled(){
	return (state & UIState::DISABLED) > 0;
}

/// Splitting up the rendering.
void UIElement::RenderSelf(GraphicsState & graphics)
{
    PrintGLError("GLError before UIElement::Render?!");
	/// If not buffered, do it nau!
	if (vboBuffer == -1){
		/// If not created geometry, do that too.
		if (!this->mesh)
			CreateGeometry();
		// Adjust values.
		if (parent)
			AdjustToWindow((int)parent->left, (int)parent->right, (int)parent->bottom, (int)parent->top);
		/// If parent is null, this means this is the root-element, so use current screen size?
		else 
			AdjustToWindow(0, ui->Width(), 0, ui->Height());
		/// Resize the square-object.
		ResizeGeometry();
		/// Buffer it.
		Bufferize();
		/// Ensure we've got a buffer now!
	}
	assert(vboBuffer && "No valid vboBuffer in UIElement::Render!");

	bool validTexture = true;

	// Set texture
	if (texture && texture->glid){
		glBindTexture(GL_TEXTURE_2D, texture->glid);
	}
	else if (texture) {
		TexMan.BufferizeTexture(texture);
	}
	else if (textureSource){
		texture = TexMan.GetTexture(textureSource);
		if (!texture)
            texture = TexMan.GetTextureByName(textureSource);
		/// Unable to fetch target-texture, so skip it.
		if (!texture){
            glBindTexture(GL_TEXTURE_2D, NULL);
            validTexture = false;
		}
		else {
            if (texture->glid == -1)
                TexMan.BufferizeTexture(texture);
            glBindTexture(GL_TEXTURE_2D, texture->glid);
		}
	}
	// No texture at all? Flag it and avoid rendering this element.
	else  {
		// Use standard black texture if so, won't matter much anyway.
		texture = TexMan.GetTexture("NULL");
		glBindTexture(GL_TEXTURE_2D, texture->glid);
		validTexture = false;
	}
	if (texture != NULL && texture->glid == -1){
		TexMan.BufferizeTexture(texture);
	}


	/// Set mip-map filtering to closest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    PrintGLError("GLError Binding texture");

	// Check for valid buffer before rendering,
	// Also check for valid texture...!
	if (vboBuffer && validTexture){

		Vector3f baseColor = color;
		// If greyed out: activatable but not currently selectable/toggleable, grey it out.
		if (this->IsDisabled()){
			baseColor *= 0.5f;
		}
		/// Set color if highlighted!
		Vector3f highlightColor(1,1,1);
		if (this->state & UIState::ACTIVE && highlightOnActive)
			highlightColor *= 0.4f;
		else if (this->state & UIState::HOVER && highlightOnHover){
		//	std::cout<<"\nHoverElement: "<<name;
			highlightColor *= 0.3f;
		}
		else {
			highlightColor *= 0.f;
			// Default color
		}
		if (this->toggled){
			float toggledFloat = 0.30f;
			highlightColor += Vector3f(toggledFloat, toggledFloat, toggledFloat);
		}
		// Duller colors for temporarily disabled buttons.
		if (this->IsDisabled())
		{
			highlightColor *= 0.75f;
		}

		assert(graphics.activeShader->uniformPrimaryColorVec4 != -1);
		glUniform4f(graphics.activeShader->uniformPrimaryColorVec4,
			baseColor.x, baseColor.y, baseColor.z, color.w);
		assert(graphics.activeShader->uniformHighlightColorVec4 != -1);
		glUniform4f(graphics.activeShader->uniformHighlightColorVec4,
			highlightColor.x, highlightColor.y, highlightColor.z, 0.0f);

		// Set material?	-	Not needed for UI!?
		// Just set a light-parameter to be multiplied to the texture?
		// Nothing for now... TODO

		// Set VBO and render
		// Bind vertices
		glBindBuffer(GL_ARRAY_BUFFER, vboBuffer);
		glVertexAttribPointer((GLuint) 0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, 0);		// Position
		glEnableVertexAttribArray(0);

		// Bind Normals
		static const GLint offsetN = 3 * sizeof(GLfloat);		// Buffer already bound once at start!
		glVertexAttribPointer((GLuint) 2, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void *)offsetN);		// UVs
		glEnableVertexAttribArray(2);

		// Bind UVs
		static const GLint offsetU = 6 * sizeof(GLfloat);		// Buffer already bound once at start!
		glVertexAttribPointer((GLuint) 1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void *)offsetU);		// UVs
		glEnableVertexAttribArray(1);

        PrintGLError("GLError Binding Buffers");
		int vertices = vboVertexCount;



		// If moveable, translate it to it's proper position!
		if (moveable){
			///
			if (graphics.activeShader->uniformModelMatrix != -1){
				/// TRanslatem power !
				Matrix4d * model = &graphics.modelMatrixD;
				float transX = alignmentX * parent->sizeX;
				float transY = alignmentY * parent->sizeY;
				model->translate(transX,transY,0);
				graphics.modelMatrixF = graphics.modelMatrixD;
			}
		}

		/// Load in ze model matrix
		glUniformMatrix4fv(graphics.activeShader->uniformModelMatrix, 1, false, graphics.modelMatrixF.getPointer());

        PrintGLError("GLError glUniformMatrix in UIElement");
		// Render normally
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, vboVertexCount);        // Draw All Of The Triangles At Once
		PrintGLError("GLError glDrawArrays in UIElement");

		// Unbind buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError();

	}
	/// Bind correct font if applicable.
	if (this->text.Length()){
		if (this->font){
			graphics.currentFont = this->font;
		}
		else if (this->fontSource && !this->font){
			this->font = Graphics.GetFont(this->fontSource);
			if (this->font)
				graphics.currentFont = this->font;
		}
	}
	// Render text if applicable!
	if (this->text.Length() && graphics.currentFont){
		TextFont * currentFont = graphics.currentFont;
		Matrix4d tmp = graphics.modelMatrixD;
		graphics.modelMatrixD.translate(this->left, this->top,(this->zDepth+0.05));
		float pixels = sizeY * textSizeRatio; // Graphics.Height();
	//	pixels *= this->sizeRatioY;

        if (currentTextSizeRatio <= 0)
		{
			textToRender = text;
			/// Rows available
			int rowsAvailable = (int)(1 / textSizeRatio);
			currentTextSizeRatio = 1.0f;
			/// Returns the size required by a call to RenderText if it were to be done now. In... pixels? or units
            float lengthRequired = currentFont->CalculateRenderSizeX(text, graphics) * pixels;
			if (lengthRequired > rowsAvailable * sizeX){
				// assert(false && "Too much text!");
//				std::cout<<"\nNOTE: Too much text for given space and size, scaling down text to fit!";
				currentTextSizeRatio = sizeX / lengthRequired;
				// Scale it down, yes.
			}
			else if (lengthRequired > sizeX){
			//	assert(false && "Add thingy to enter new-lines automagically.");
			//	std::cout<<"\nINFO: Length exceeding size, calculating and inserting newlines as possible.";
			//	std::cout<<"\nTokenizing text: "<<text;
				/// Tokenize into words based on spaces.
				List<String> words = text.Tokenize(" ");
			//	std::cout<<"\nWords: "<<words.Size();
				textToRender = String();
				String line = String(), line2 = String();
				for (int i = 0; i < words.Size(); ++i){
					/// Assume word fits?
					String word = words[i];
				//	std::cout<<"\nBlubb ";
					float lengthRequiredWord = currentFont->CalculateRenderSizeX(word, graphics) * pixels;
				//	std::cout<<"\nBlubb ";
				//	std::cout<<"\nLengthRequiredWord: "<<lengthRequiredWord<<" sizeX: "<<sizeX;
					if (lengthRequiredWord >= sizeX && i == 0){
				//		std::cout<<"\nWord too long to split into multiple rows.. scaling down text as a last resort :(";
						float divider = sizeX / lengthRequiredWord;
						pixels *= divider;
					}
				//	std::cout<<"\nBlubb ";
					float lengthRequiredLine = currentFont->CalculateRenderSizeX(line, graphics) * pixels;
				//	std::cout<<"\nBlubb ";
					/// Check if catenated line will exceed bounds.
					line2 = line + " " + word;
					float lengthRequiredLine2 = currentFont->CalculateRenderSizeX(line2, graphics) * pixels;
				//	std::cout<<"\nBlubb ";
					if (lengthRequiredLine2 > sizeX){
						/// Add first line to textToRender + new line
						textToRender += line + "\n";
						line = String();
					}
				//	std::cout<<"\nBlubb ";
					line += word + " ";
				}

				/// Add the final line to the text to render here
				textToRender += line;
			//	std::cout<<"\nTextToRender: "<<textToRender;
			}
        }

		pixels *= currentTextSizeRatio; //this->textSizeRatio;
//		std::cout<<"\nTextToRender size in pixels: "<<pixels;
		graphics.modelMatrixD.Scale(pixels);	//Graphics.Height()
		graphics.modelMatrixF = graphics.modelMatrixD;
		Vector4f textColorToRender = this->textColor;
		// If disabled, dull the color! o.o
		if (this->IsDisabled())
			textColorToRender *= 0.55f;
	//	color.w *= 0.5f;
		graphics.currentFont->SetColor(textColorToRender);
//		std::cout<<"\nTextToRender: "<<textToRender;
		graphics.currentFont->RenderText(this->textToRender, graphics);
		graphics.modelMatrixF = graphics.modelMatrixD = tmp;
	}
}

void UIElement::RenderChildren(GraphicsState & graphics){
	bool scissorDisabled = (graphics.settings & SCISSOR_DISABLED) > 0;
    // Render all children
	for (int i = 0; i < childList.Size(); ++i){
        UIElement * child = childList[i];
   //     std::cout<<"\nRendering UIElement "<<child<<" with name "<<(child?child->name : "NULL");
        if (!child->visible)
            continue;

        Rect previousScissor((int)graphics.leftScissor, (int)graphics.bottomScissor, (int)graphics.rightScissor, (int)graphics.topScissor);

        Vector3f relativePosition = graphics.modelMatrixF * Vector3f();
 //       std::cout<<"\nRelative position: "<<relativePosition;

		/// Do scissor calculations if not disabled and we're not root since it's stats are not necessarily updated.
		if (!scissorDisabled && parent){
			float currentLeft = left + relativePosition.x,
				currentRight = right + relativePosition.x,
				currentTop = top + relativePosition.y,
				currentBottom = bottom + relativePosition.y;

			graphics.leftScissor = graphics.leftScissor > currentLeft ? graphics.leftScissor : currentLeft;
			graphics.rightScissor = graphics.rightScissor < currentRight ? graphics.rightScissor : currentRight;
			graphics.bottomScissor = graphics.bottomScissor > currentBottom ? graphics.bottomScissor : currentBottom;
			graphics.topScissor = graphics.topScissor < currentTop ? graphics.topScissor : currentTop;
		}

	    int scissorWidth = (int) (graphics.rightScissor - graphics.leftScissor);
	    int scissorHeight = (int) (graphics.topScissor - graphics.bottomScissor);

	  //  assert(scissorWidth >= 0);
	  //  assert(scissorHeight >= 0);
	  /// If bad scissor do nothing.
        if (scissorWidth < 0 || scissorHeight < 0){
          //  std::cout<<"\nScissor NOT VISIBLE:  e "<<name<<" left: "<<left<<" right: "<<right<<" top: "<<top<<" bottom: "<<bottom<<" width: "<<scissorWidth<<" height: "<<scissorHeight;
        }
        /// If not, render.
        else {
            if (!scissorDisabled){
            //    std::cout<<"\nGLScissor: e "<<name<<" posX "<<posX<<" sizeX "<<sizeX<<" posY "<<posY<<" sizeY "<<sizeY;
                glScissor((GLint) (graphics.leftScissor + graphics.viewportX0),
                          (GLint) (graphics.bottomScissor + graphics.viewportY0),
                          scissorWidth,
                          scissorHeight);
            }
            child->Render(graphics);
        }
        /// And pop the scissor statistics back as they were.
		graphics.leftScissor = (float)previousScissor.x0;
	    graphics.rightScissor = (float)previousScissor.x1;
	    graphics.bottomScissor = (float)previousScissor.y0;
	    graphics.topScissor = (float)previousScissor.y1;
	}
}


/// Adjusts the UI element size and position relative to new window size
void UIElement::AdjustToWindow(int w_left, int w_right, int w_bottom, int w_top){
    /// Reset text-variables so that they are re-calculated before rendering again.
    currentTextSizeRatio = -1.0f;

	// Extract some attributes before we begin
    left = -1.0f, right = 1.0f, bottom = -1.0f, top = 1.0f;
	sizeX = 1, sizeY = 1;
	float z = 0;
	if (parent){
		left = parent->posX - parent->sizeX/2;
		right = parent->posX + parent->sizeX/2;
		bottom = parent->posY - parent->sizeY/2;
		top = parent->posY + parent->sizeY/2;
	}
	else {
		left = (float)w_left;
		right = (float)w_right;
		bottom = (float)w_bottom;
		top = (float)w_top;
	}

//	std::cout<<"\nUIElement::AdjustToWindow called for element "<<name<<": L"<<left<<" R"<<right<<" B"<<bottom<<" T"<<top;

	/// Center of the UI that we place, only to be used for relative alignments!
	float centerX;
	float centerY;
	/// Default for non-movable objects: place them where they should be.
	if (!moveable){
		centerX = ((float)right - left) * alignmentX + left,
		centerY = ((float)top - bottom) * alignmentY + bottom;
	}
	// Default for movable objects: place them at 0 and render them dynamically relative to their parents!
	else {
		centerX = left;
		centerY = bottom;
	}
	sizeX = (int) (right - left);
	sizeY = (int) (top - bottom);
	if (parent)
		z = parent->zDepth + 0.1f;
	zDepth = z;


	/// Check alignment
	switch(alignment){
		case NULL_ALIGNMENT: {
			/// Default behavior, just scale out from our determined center.
			left = centerX - sizeX * sizeRatioX / 2;
			right = centerX + sizeX * sizeRatioX / 2;
			bottom = centerY - sizeY * sizeRatioY / 2;
			top = centerY + sizeY * sizeRatioY / 2;
			break;
		}
		case MAXIMIZE:
			if (this->keepRatio){
				//float screenRatio = (w_right - w_left) / (float)(w_top - w_bottom);
				float newRatio = 1;//screenRatio / ratio;
				left = centerX - sizeX * newRatio / 2;
				right = centerX + sizeX * newRatio / 2;
				bottom = centerY - sizeY * newRatio / 2;
				top = centerY + sizeY * newRatio / 2;
			}
			else
				mesh->SetDimensions((float)w_left, (float)w_right, (float)w_bottom, (float)w_top, (float)zDepth);
			break;
		case CENTER:
			/// Do nothing, we start off using regular centering
			left = centerX - sizeX * sizeRatioX / 2;
			right = centerX + sizeX * sizeRatioX / 2;
			bottom = centerY - sizeY * sizeRatioY / 2;
			top = centerY + sizeY * sizeRatioY / 2;
			break;
		case TOP:
			/// Do nothing, we start off using regular centering
			left = centerX - sizeX * sizeRatioX / 2;
			right = centerX + sizeX * sizeRatioX / 2;
			bottom = centerY - sizeY * sizeRatioY;
			top = centerY;
			break;
		default:
			// Make half-size
			std::cout<<"UIElement "<<this->name<<" gets default half-size.";
			mesh->SetDimensions((left*3 + right)/4, (left + right*3/4), (bottom * 3 + top)/4, (bottom + top*3)/4, zDepth+1);
			break;
	}

	/// Set the new dimensions
	if (mesh)
		mesh->SetDimensions(left, right, bottom, top, z);

	// Save away the new sizes
	sizeX = (int) (right - left);
	sizeY = (int) (top - bottom);
	posX = sizeX/2 + left;
	posY = sizeY/2 + bottom;
	position = Vector3f(posX, posY, zDepth);
	// Correct the position for the movable objects! ^^
	if (moveable){
		posX += parent->sizeX * alignmentX;
		posY += parent->sizeY * alignmentY;
		left = posX - sizeX / 2.0f;
		right = posX + sizeX / 2.0f;
		top = posY + sizeY / 2.0f;
		bottom = posY - sizeY / 2.0f;
	}

	/// Adjust all children too
	for (int i = 0; i < childList.Size(); ++i){
		assert(childList[i] != this);
		childList[i]->AdjustToWindow((int)left, (int)right, (int)bottom,(int)top);
	}
}

int UIElement::GetAlignment(String byName){
	if (byName == "NULL_ALIGNMENT")
		return UIElement::NULL_ALIGNMENT;
	else if (byName.Contains("MAXIMIZE"))
		return UIElement::CENTER;
	else if (byName == "CENTER")
		return UIElement::CENTER;
	else if (byName == "TOP_LEFT")
		return UIElement::TOP_LEFT;
	else if (byName == "BOTTOM_LEFT")
		return UIElement::BOTTOM_LEFT;
	else {
		assert(false && "Unknown symbol in UIElement::GetAlignment(byName)");
	}
	return NULL_ALIGNMENT;
}

/** Sets slider level by adjusting it's child's position.
	Values should be within the range [0.0,1.0]. **/
void UISlider::SetLevel(float level){
	if(!childList.Size()){
		std::cout<<"\nSlider "<<this->name<<" lacking child (sliderHandle)!";
		return;
	}
	childList[0]->alignmentX = level;
	return;
}
/// Returns the slider's level
float UISlider::GetLevel(){
	if (!childList.Size()){
		std::cout<<"\nERROR: Unable to get slider level as it has no child (handle)!";
		return 0;
	}
	return childList[0]->alignmentX;
}

// Creates the Square mesh used for rendering the UIElement and calls SetDimensions with it's given values.
void UIElement::CreateGeometry(){
	Square * sq = new Square();
//	std::cout<<"\nResizing geometry "<<name<<": L"<<left<<" R"<<right<<" B"<<bottom<<" T"<<top<<" Z"<<this->zDepth;
	sq->SetDimensions(left, right, bottom, top, this->zDepth);
	this->mesh = sq;
	for (int i = 0; i < childList.Size(); ++i){
		childList[i]->CreateGeometry();
	}

}
void UIElement::ResizeGeometry(){
//    std::cout<<"\nResizing geometry: L"<<left<<" R"<<right<<" B"<<bottom<<" T"<<top<<" Z"<<this->zDepth;
	this->mesh->SetDimensions(left, right, bottom, top, this->zDepth);
	for (int i = 0; i < childList.Size(); ++i){
		childList[i]->ResizeGeometry();
	}
}
void UIElement::DeleteGeometry(){
//	assert(mesh);
	if (mesh == NULL)
		return;
	delete mesh;
	mesh = NULL;
	for (int i = 0; i < childList.Size(); ++i){
		childList[i]->DeleteGeometry();
	}
}

/// For example UIState::HOVER, not to be confused with flags! State = current, Flags = possibilities
void UIElement::AddState(int i_state){
	// Return if trying to add invalid state.
	if (!hoverable && i_state & UIState::HOVER)
		return;
	state |= i_state;
}

/// For example UIState::HOVER, if recursive will apply to all children.
void UIElement::RemoveState(int statesToRemove, bool recursive /* = false*/){
	state &= ~statesToRemove;
	if (recursive){
		for (int i = 0; i < childList.Size(); ++i){
			childList[i]->RemoveState(statesToRemove, recursive);
		}
	}
}


/// Take care when using!
void UIElement::SetFlags(int flags){
	assert(false);
	/// RETHINK MAN!
	// state |= flags;
}

// Works recursively.
void UIElement::RemoveFlags(int flags){
	int antiFlag = ~flags;
	if (state & flags){
//		std::cout<<"\nFound UIElement "<<name<<" with specified flags to remove: "<<flags;
	}
	state &= antiFlag;
	if (state == 0)
		state = UIState::IDLE;
	for (int i = 0; i < childList.Size(); ++i){
		childList[i]->RemoveFlags(flags);
	}
}

UILabel::UILabel(String name /*= ""*/)
: UIElement()
{
	this->name = name;
	text = name;
	type = UIType::LABEL;
	selectable = hoverable = activateable = false;
	/// Set text-color at least for labels!
	textColor = defaultTextColor;
};

UILabel::~UILabel()
{
//	std::cout<<"\nUILabel destructor";
}

UISliderHandle::UISliderHandle()
: UIElement()
{
	type = UIType::SLIDER_HANDLE;
	moveable = true;
};

UISliderHandle::~UISliderHandle()
{
//	std::cout<<"\nUISliderHandle destructor";
}

UISlider::UISlider()
: UIElement()
{
	type= UIType::SLIDER_BAR;
	progress = NULL;
};

UISlider::~UISlider()
{
//	std::cout<<"\nUISlider destructor";
}


/// Used by input-captuing elements. Should not be called for any base UI elements(?)
int UIElement::OnKeyDown(int keyCode, bool downBefore){
	assert(false);
	return 0;
}
/// Used for getting text. This will be local translated language key codes?
int UIElement::OnChar(int asciiCode){
	assert(false);
	return 0;
}


/// Called to ensure visibility of target element. First call should be made to the target element with a NULL-argument!
void UIElement::EnsureVisibility(UIElement * element){
	if (!element)
		element = this;
	if (parent)
		parent->EnsureVisibility(this);
}
