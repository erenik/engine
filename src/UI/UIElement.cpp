// Emil Hedemalm
// 2013-07-03, Linuxifying!

#include <Graphics/OpenGL.h>

#include "InputState.h"
#include "UIElement.h"
#include "UITypes.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"

#include "OS/Sleep.h"

#ifdef WINDOWS
    #include <Windows.h>
#endif

#include "File/LogFile.h"
#include "../Input/InputManager.h"
#include "TextureManager.h"
#include "Mesh/Square.h"
#include "UserInterface.h"
#include "MathLib/Rect.h"

#include "GraphicsState.h"
#include "Graphics/Fonts/TextFont.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/OpenGL.h"
#include "Graphics/GLBuffers.h"
#include "Direction.h"

extern InputManager input;
extern UserInterface ui[GameStateID::MAX_GAME_STATES];

int UIElement::idEnumerator = 0;

/// Called when this UI is made active (again).
void UIElement::OnEnterScope(){
	// Do nothing in general.
	interaction.visible = true;
	for (int i = 0; i < children.Size(); ++i)
		children[i]->OnEnterScope();
	if (onEnterScope.Length())
		MesMan.QueueMessages(onEnterScope);
}
/// Called once this element is no longer visible for any reason. E.g. switching game states to display another UI.
void UIElement::OnExitScope(bool forced)
{
	/// Skip those which have already exited scope.
	if (!interaction.visible)
		return;
	// Do nothing in general.
	for (int i = 0; i < children.Size(); ++i)
		children[i]->OnExitScope(forced);
	if (interaction.onExit.Length())
		MesMan.QueueMessages(interaction.onExit);

	/// Check if the element has any onPop messages.
	if (onPop.Length())
		MesMan.QueueMessages(onPop);
	if (forced && onForcePop.Length())
		MesMan.QueueMessages(onForcePop);

	interaction.inStack = false;
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
void UIElement::Nullify()
{
	topBorder = nullptr;
	bottomBorder = nullptr;
	rightBorder = nullptr;
	topRightCorner = nullptr;

//	if (text.defaultTextColors != nullptr)
//		text.colors = new TextColors(*defaultTextColors);


	layout.Nullify();
	text.Nullify();


	childrenCreated = false;
	/// ID
	/// Hierarchy
	parent = NULL;
	ui = NULL;
	divider = Vector2f(0.5f, 0.5f);
	noLabel = false;

	scalable = DEFAULT_SCALABILITY;		// Allow scaling depending on AppWindow size?
	ratio = 1.0;
	keepRatio = true;

	type = UIType::BASIC;

	state = UIState::IDLE;

	removeOnPop = false;

    isSysElement = false;

	interaction.Nullify();




	// Text.
	label = NULL;
}

UIElement::UIElement() : id (idEnumerator++) {
	Nullify();
}

UIElement::UIElement(String name) : id(idEnumerator++) {
	Nullify();
	this->name = name;
}

void UIElement::CreateChildren(GraphicsState* graphicsState){}

/// Copy-cosntructor.
UIElement::UIElement(const UIElement & ref) : id(idEnumerator++) {
	Nullify();
	ref.CopyGeneralVariables(this);
	ref.CopySpecialVariables(this);
}

// Works recursively down to all children 
void UIElement::SetUI(UserInterface * ui) {
	this->ui = ui;
	for (int i = 0; i < children.Size(); ++i) {
		children[i]->SetUI(ui);
	}
}

UIElement::~UIElement()
{
//	std::cout<<"\nUIElement destructor";
	/// Hierarchy
	parent = NULL;
	/// Use for-loop instead of crazy recursion for deallocating the children
	children.ClearAndDelete();

	DeleteBorders(nullptr);

	SAFE_DELETE(text.onHoverTextColor);

	/// Inform the input manager that we delete the element.
	// InputMan.OnElementDeleted(this);
}


int UIElement::CenteredContentParentPosX() const {
	return layout.posX;
}

// If parent is e.g. List, available size will vary depending on if scrollbar is present or not.
int UIElement::AvailableParentSizeX() const {
	return layout.sizeX;
}

// Creates a deep copy of all child elements (where possible).
UIElement * UIElement::Copy() {
	UIElement * copy = new UIElement();
	CopyGeneralVariables(copy);
	CopySpecialVariables(copy);
	CopyChildrenInto(copy);
	return copy;	
}

void UIElement::CopyGeneralVariables(UIElement * intoElement) const {
	intoElement->layout = layout;
	intoElement->interaction = interaction;
	intoElement->text = text;

	intoElement->text.onHoverTextColor = nullptr; // Special case.

	intoElement->scalable = scalable;
	intoElement->ratio = ratio;
	intoElement->keepRatio = keepRatio;
	intoElement->type = type;
}

void UIElement::CopySpecialVariables(UIElement * intoElement) const {
	if (text.GetColors())
		intoElement->text.SetColors(*text.GetColors());
	if (text.onHoverTextColor)
		intoElement->text.onHoverTextColor = new Color(*text.onHoverTextColor);
}

void UIElement::CopyChildrenInto(UIElement * copyOfSelf) const {
	// Reset all/most pointers though?
	copyOfSelf->children.Clear();
	for (int i = 0; i < children.Size(); ++i) {
		UIElement * childCopy = children[i]->Copy();
		copyOfSelf->AddChild(nullptr, childCopy);
	}
}

/// Callback-function for sub-classes to implement own behaviour to run within the UI-class' code. Return true if it did something.
bool UIElement::OnProceed(GraphicsState* graphicsState)
{
	if (parent)
		return parent->OnProceed(graphicsState);
	return false;
}

void UIElement::StopInput() {}

// Used for handling things like drag-n-drop and copy-paste operations, etc. as willed.
void UIElement::ProcessMessage(Message * message)
{
}

/// Sets text, queueing recalculation of the rendered variant.
void UIElement::SetText(CTextr newText, bool force)
{
	/// Check that it's not currently being edited. If so cancel this setting.
	if (this->interaction.demandInputFocus && HasState(UIState::ACTIVE) && !force){
		return;
	}
	text.SetText(newText, force);
}

// If true, capitalizes that moment.
void UIElement::SetForceUpperCase(bool newValue) {
	text.forceUpperCase = newValue;
	if (newValue)
		text.SetText(text.GetText().ToUpperCase(), false);
}

/** Fetches texture, assuming the textureSource has been set already. Binds and bufferizes, so call only from graphics thread. 
	Returns false if no texture could be find, bind or bufferized. */
bool UIElement::FetchBindAndBufferizeTexture()
{
	bool result = visuals.FetchBindAndBufferizeTexture();
	if (result == true)
		OnTextureUpdated();
	return result;
}

/// Called after FetchBindAndBufferizeTexture is called successfully. (may also be called other times).
void UIElement::OnTextureUpdated()
{
}

/// Recalculates and sets highlighting factors used when rendering the UI (dedicated shader settings)
void UIElement::UpdateHighlightColor()
{
	visuals.UpdateHighlightColor(IsDisabled(), state & UIState::ACTIVE, state & UIState::HOVER);
}


void UIElement::DeleteElement(int targetID){
	UIElement* target = NULL;
	children.ClearAndDelete();
}

/** Deletes target element if it is found.
	It will also unbufferize and free resources as necessary. Should ONLY be called from the render-thread!
*/
bool UIElement::Delete(GraphicsState& graphicsState, UIElement * element){
	/// If found it, delete it.
	if (children.Exists(element))
	{
		children.Remove(element);
		/// Free GL buffers and deallocate it!
		element->FreeBuffers(graphicsState);
		element->DeleteGeometry();
		delete element;
		return true;
	}
	/// If not, continue recursive among all children.
	else {
		for (int i = 0; i < children.Size(); ++i){
			/// Remove it
			if (children.Remove(element))
				return true;
		}
	}
	return false;
}

/// Deletes all children and content inside.
void UIElement::Clear(GraphicsState& graphicsState)
{
	// Unbufferize first?
	if (visuals.IsBuffered())
		for (int i = 0; i < children.Size(); ++i)
		{
			UIElement * child = children[i];
			child->FreeBuffers(graphicsState);
		}
	// Grab mutex first?
	uiMutex.Claim();
	children.ClearAndDelete();
	uiMutex.Release();
};

//******************************************************************************//
// Activation functions
//******************************************************************************//

// Hovers over this element. calling OnHover after setting the UIState::HOVER flag.
UIElement * UIElement::Hover(GraphicsState* graphicsState)
{
	AddState(graphicsState, UIState::HOVER);
	this->OnHover();
	return this;
}	

// Returns true once the highest-level appropriate element has been selected,
// or it has been determined that the selected child was not the active one.
// A value of true will thus make the algorithm return true and ending it's calculations.
UIElement* UIElement::Hover(GraphicsState* graphicsState, int mouseX, int mouseY)
{

	UIElement* result = NULL;

	// Don't process invisible UIElements, please.
	if (interaction.visible == false)
		return false;

	// Check if the mouse is outside the element's boundaries.
	if (layout.IsOutside(mouseX, mouseY)) {
		// Return false if we are outside of the boundaries,
		// since we haven't found the selected element.
		RemoveState(UIState::HOVER);
	//	if(child != NULL)
		return NULL;
	}
	/// Process stuff that should happen when hovering on this element!
	OnHover();

	// Check axiomaticness
	if (interaction.axiomatic){
		if (interaction.hoverable){
			state |= UIState::HOVER;
			std::cout<<"\nAXIOMATICNESS?! "<<name<<" is hover.";
			return this;
		}
		return NULL;
	}

	// Alright, the mouse is inside this element!
	// Do we have children?
	for (int i = children.Size()-1; i >= 0; --i)
	{
	    UIElement * child = children[i];
		// so we can return true.
		child->RemoveState(UIState::HOVER);
		if (!child->interaction.visible)
            continue;
		if (result != nullptr)
			break;
		result = child->Hover(graphicsState, mouseX, mouseY);
	}

	// If we have a result now, exit
	if (result != NULL) {
		// The active element has been found further down the tree,
		return result;
	}

// If the Element is not hoverable, return now.
	if (!interaction.hoverable)
		return NULL;

	// If the UIElement is being clicked (left mouse button)
	// Return and don't change the StateMan.
	if (state == UIState::ACTIVE)
		return this;

	// If we weren't hovering over it before.
	if (! (HasState(UIState::HOVER)))
	{
		this->OnHover();
	}
	// The mouse is inside this element,
	// and our children (if any) were not the active Entity.
	// This means that this element must be the active Entity!
	if (!AddState(graphicsState, UIState::HOVER))
		return NULL;
	
	return this;
}


// Returns true once the highest-level appropriate element has been found.
// No co-ordinates are required since we will instead require the element to already
// be highlighted/hovered above.
UIElement* UIElement::Click(GraphicsState* graphicsState, int mouseX, int mouseY)
{
	UIElement* result = 0;
	if (HasState(UIState::DISABLED)) {
		LogGraphics("Ignoring element " + name + " as it's disabled", INFO);
		return nullptr;
	}

	// Don't process invisible UIElements, please.
	if (interaction.visible == false)
		return false;

	// Check if the mouse is outside the element's boundaries.
	if (layout.IsOutside(mouseX, mouseY)) {
			// Return false if we are outside of the boundaries,
			// since we haven't found the selected element.
			state |= UIState::IDLE;
			//	if(child != NULL)
			return 0;
	}

	// Check axiomaticness (direct-activation without further processing)
	if (interaction.axiomatic){
		if (interaction.activateable){
			state |= UIState::ACTIVE;
			return this;
		}
		return 0;
	}

	// Alright, the mouse is inside this element!
	// Do we have children?
	for (int i = children.Size()-1; i >= 0; --i){
		UIElement * child = children[i];
	    if (!child->interaction.visible)
            continue;
		result = child->Click(graphicsState, mouseX, mouseY);
		if (result != NULL){
			// The active element has been found further down the tree,
			// so we can return true.
			state |= UIState::IDLE;
			return result;
		}
	}
	// Check the element's StateMan. If it is hovered over, we've found it.
	if (this->interaction.activateable){
		state |= UIState::ACTIVE;
		return this;
	}
	// If not, return false, since we haven't foun the right element.
	return NULL;
}

// Returns a non-0 message once the highest-level appropriate element has been found.
// No co-ordinates are required since we will instead require the element to already
// clicked.
UIElement* UIElement::Activate(GraphicsState * graphicsState)
{
	UIElement* result = 0;
	// Don't process invisible UIElements, please.
	if (interaction.visible == false)
		return 0;
	// And ignore disabled elements and their children.
	if (HasState(UIState::DISABLED)) {
		LogGraphics("Ignoring disabled element " + name, INFO);
		return nullptr;
	}
	// Alright, the mouse is inside this element!
	// Do we have children?
	for (int i = children.Size()-1; i >= 0; --i){
		UIElement * child = children[i];
	    if (!child->interaction.visible)
            continue;
		result = child->Activate(graphicsState);
		if (result != 0){
			// The active element has been found further down the tree,
			// so we return it's message.
			state |= UIState::IDLE;
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
		if (interaction.selectable == true){
			if (type== UIType::CHECKBOX){
				interaction.selected = !interaction.selected;
			}
			else if (type== UIType::RADIOBUTTON){
				parent->DeselectAll();
				interaction.selected = true;
			}
			else {
			//	selected = true;
			}
		}
		else {
			// Element not activatable
		}
		bool didSomething = false;
		if (activationMessage.Length() != 0)
		{
			didSomething = true;
			List<String> msgs = activationMessage.Tokenize("&");
			for (int i = 0; i < msgs.Size(); ++i)
			{
				Message * message = new Message(msgs[i]);
				message->element = this;
				MesMan.QueueMessage(message);
			}
		}
		for (int i = 0; i < activationActions.Size(); ++i)
		{
			didSomething = true;
			UIAction & uia = activationActions[i];
			uia.Process(graphicsState, this);
		}
		if (!didSomething)
		{
			std::cout<<"\nonActivate and activationMessage both NULL in element: "<<name;
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
	if (interaction.visible == false)
		return 0;

	// Check if the mouse is outside the element's boundaries.
	if (layout.IsOutside(mouseX, mouseY))
    {
        return NULL;
	}
    // Do we have children?
	for (int i = 0; i < children.Size(); ++i){
		UIElement * child = children[i];
	    if (!child->interaction.visible)
            continue;
		result = child->GetElement(mouseX, mouseY);
		if (result != NULL){
			return result;
		}
	}
	return this;
}

/// Returns all children recursively.
List<UIElement*> UIElement::AllChildrenR()
{
	List<UIElement*> allChildren;
	allChildren = children;
	for (int i = 0; i < children.Size(); ++i)
	{
		UIElement * child = children[i];
		allChildren += child->AllChildrenR();
	}
	return allChildren;
}

bool UIElement::HasActivatables()
{
	for (int i = 0; i < children.Size(); ++i)
	{
		UIElement * child = children[i];
		if (child->interaction.activateable)
			return true;
		if (child->HasActivatables())
			return true;
	}
	return false;
}

UIElement * UIElement::GetElement(String byName, UIType andType)
{
	if (type == andType)
	{
		if (name == byName)
			return this;
	}
	for (int i = 0; i < children.Size(); ++i)
	{
		UIElement * child = children[i];
		UIElement * found = child->GetElement(byName, andType);
		if (found)
			return found;
	}
	return NULL;
}

/// o.o
void UIElement::OnMouseMove(Vector2i activeWindowCoords)
{
	// By default.. do nothing.
}


/// For mouse-scrolling. By default calls it's parent's OnScroll.
bool UIElement::OnScroll(GraphicsState * graphicsState, float delta)
{
    if (parent)
        return parent->OnScroll(graphicsState, delta);
    return false;
}


/// Returns the root, via parent-chain.
UIElement * UIElement::GetRoot()
{
	assert(parent != this);
	if (parent == this) // Shouldn't come here...
		parent = nullptr;
	if(parent)
		return parent->GetRoot();
	//assert(ui != nullptr);
	return this;
}

/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UIElement::OnInputUpdated(GraphicsState* graphicsState, UIInput * inputElement)
{
	if (parent)
		parent->OnInputUpdated(graphicsState, inputElement);
}

const Text& UIElement::GetText() const {
	return text.GetText();
}

/// Callback sent to parents once an element is toggled, in order to act upon it. Used by UIMatrix, can be ToggleButton, Checkbox et al
void UIElement::OnToggled(UIElement * box)
{
	if (parent)
		parent->OnToggled(box);
}

void UIElement::InheritNeighbours(UIElement * fromElement)
{
	interaction.InheritNeighbours(fromElement->interaction);
}


/// Suggests a neighbour which could be to the top of this element. Meant to be used for UI-navigation support. The reference element indicates the element from which we are seeking a compatible or optimum neighbour.
UIElement * UIElement::GetUpNeighbour(GraphicsState* graphicsState, UIElement * referenceElement, bool & searchChildrenOnly)
{
	UIElement * element = NULL;
	// Ok, we are the one,
	if (referenceElement == NULL){
		referenceElement = this;
	}
	/// If we have a pointer, just use it!
	if (interaction.upNeighbour){
		element = interaction.upNeighbour;
	}
	/// If not, fetch it if possible.
	else {
		/// Check if we got a preferred neighbour.
		if (interaction.upNeighbourName.Length())
			element = this->GetRoot()->GetElementByName(interaction.upNeighbourName);
		/// If still haven't found a decent one, consult our parent. Unless we're in the stack of course, upon which it means we have found no decent new neighbour.
		if (!element && !searchChildrenOnly && !interaction.inStack)
		{
			if (parent)
				element = parent->GetUpNeighbour(graphicsState, referenceElement, searchChildrenOnly);
			/// No parent? Then we probably don't have any good one, search lower down, yo.
			else 
				return NULL;
		}
	}
	/// All-right, so we found a suitable neighbour hopefully. Make sure that it is hoverable?
	if (element && !element->interaction.navigatable)
	{
		/// First query if the element has any valid hoverable children, if so select them.
		UIElement * childElement = element->GetElementByFlag(UIFlag::ACTIVATABLE);
		if (childElement)
			element = childElement;
		/// If no valid children could be found, continue searching.
		else 
		{
			// Except if we have started recursing, then just return NULL or ourself..
			if (element == this)
				return NULL;
		}
	}
	// Nothing found? Then set ourselves to be this sought after element!
	if (!element)
	{
		element = GetElementClosestToSelf(true);
	}
	
	// For some UI elements, such as lists, the list itself is not navigatable, so query it to get the first element in its list if so here.
	if (element)
		element = element->GetNavigationElement(NavigateDirection::Up);

	return element;
}

/// Suggests a neighbour which could be to the right of this element. Meant to be used for UI-navigation support. The reference element indicates the element to which we are seeking a compatible or optimum neighbour.
UIElement * UIElement::GetRightNeighbour(UIElement * referenceElement, bool & searchChildrenOnly)
{
	UIElement * element = NULL;
	// Ok, we are the one,
	if (referenceElement == NULL){
		referenceElement = this;
	}
	/// If we have a pointer, just use it!
	if (interaction.rightNeighbour && interaction.rightNeighbour->interaction.activateable)
	{
		element = interaction.rightNeighbour;
	}
	/// If not, fetch it if possible.
	else {
		/// Check if we got a preferred neighbour.
		if (interaction.rightNeighbourName.Length())
		{
			element = this->GetRoot()->GetElementByName(interaction.rightNeighbourName);
			if (element && !element->interaction.activateable)
				element = NULL;
		}
		/// If still haven't found a decent one, consult our parent. Unless we're in the stack of course, upon which it means we have found no decent new neighbour.
		if (!element && !searchChildrenOnly && !interaction.inStack)
		{
			if (parent)
				element = parent->GetRightNeighbour(referenceElement, searchChildrenOnly);
			/// No parent? Then we probably don't have any good one, search lower down, yo.
			else 
				return NULL;
		}
	}
	/// All-right, so we found a suitable neighbour hopefully. Make sure that it is hoverable?
	if (element && !element->interaction.activateable){
		/// First query if the element has any valid hoverable children, if so select them.
		UIElement * childElement = element->GetElementByFlag(UIFlag::ACTIVATABLE);
		if (childElement)
			element = childElement;
		/// If no valid children could be found, continue searching.
		else {
			// Except if we have started recursing, then just return NULL or ourself..
			if (element == this)
				return NULL;
//			element = element->GetRightNeighbour(NULL, searchChildrenOnly);
		}
	}
	// Nothing found? Then set ourselves to be this sought after element!
	if (!element)
	{
		element = GetElementClosestToSelf(true);
	}
	// For some UI elements, such as lists, the list itself is not navigatable, so query it to get the first element in its list if so here.
	if (element)
		element = element->GetNavigationElement(NavigateDirection::Right);
	return element;
}

/// Suggests a neighbour which could be to the right of this element. Meant to be used for UI-navigation support. The reference element indicates the element to which we are seeking a compatible or optimum neighbour, and should be NULL for the initial call.
UIElement * UIElement::GetDownNeighbour(GraphicsState* graphicsState, UIElement * referenceElement, bool & searchChildrenOnly)
{
	UIElement * element = NULL;
	// Ok, we are the one,
	if (referenceElement == NULL){
		referenceElement = this;
	}
	/// If we have a pointer, just use it!
	if (interaction.downNeighbour){
		element = interaction.downNeighbour;
	}
	/// If not, fetch it if possible.
	else {
		/// Check if we got a preferred neighbour.
		if (interaction.downNeighbourName.Length())
			element = this->GetRoot()->GetElementByName(interaction.downNeighbourName);
		/// If still haven't found a decent one, consult our parent. Unless we're in the stack of course, upon which it means we have found no decent new neighbour.
		if (!element && !searchChildrenOnly && !interaction.inStack)
		{
			if (parent)
				element = parent->GetDownNeighbour(graphicsState, referenceElement, searchChildrenOnly);
			/// No parent? Then we probably don't have any good one, search lower down, yo.
			else 
				return NULL;
		}
	}
	/// All-right, so we found a suitable neighbour hopefully. Make sure that it is hoverable?
	if (element && !element->interaction.hoverable){
		/// First query if the element has any valid hoverable children, if so select them.
		UIElement * childElement = element->GetElementByFlag(UIFlag::ACTIVATABLE);
		if (childElement)
			element = childElement;
		/// If no valid children could be found, continue searching.
		else 
		{
			// Except if we have started recursing, then just return NULL or ourself..
			if (element == this)
				return NULL;
//			element = element->GetDownNeighbour(NULL, searchChildrenOnly);
		}
	}
	// Nothing found? Then set ourselves to be this sought after element!
	if (!element)
	{
		element = GetElementClosestToSelf(true);
	}
	// For some UI elements, such as lists, the list itself is not navigatable, so query it to get the first element in its list if so here.
	if (element)
		element = element->GetNavigationElement(NavigateDirection::Down);
	return element;

}
/// Suggests a neighbour which could be to the right of this element. Meant to be used for UI-navigation support. The reference element indicates the element to which we are seeking a compatible or optimum neighbour, and should be NULL for the initial call.
UIElement * UIElement::GetLeftNeighbour(UIElement * referenceElement, bool & searchChildrenOnly)
{
	UIElement * element = NULL;
	// Ok, we are the one,
	if (referenceElement == NULL){
		referenceElement = this;
	}
	/// If we have a pointer, just use it!
	if (interaction.leftNeighbour && interaction.leftNeighbour->interaction.activateable)
	{
		element = interaction.leftNeighbour;
	}
	/// If not, fetch it if possible.
	else {
		/// Check if we got a preferred neighbour.
		if (interaction.leftNeighbourName.Length())
		{
			element = this->GetRoot()->GetElementByName(interaction.leftNeighbourName);
			if (element && !element->interaction.activateable)
				element = NULL;
		}
		/// If still haven't found a decent one, consult our parent. Unless we're in the stack of course, upon which it means we have found no decent new neighbour.
		if (!element && !searchChildrenOnly && !interaction.inStack)
		{
			if (parent)
				element = parent->GetLeftNeighbour(referenceElement, searchChildrenOnly);
			/// No parent? Then we probably don't have any good one, search lower down, yo.
			else 
				return NULL;
		}
	}
	/// All-right, so we found a suitable neighbour hopefully. Make sure that it is hoverable?
	if (element && !element->interaction.activateable){
		/// First query if the element has any valid hoverable children, if so select them.
		UIElement * childElement = element->GetElementByFlag(UIFlag::ACTIVATABLE);
		if (childElement)
			element = childElement;
		/// If no valid children could be found, continue searching.
		else
		{
			// Except if we have started recursing, then just return NULL or ourself..
			if (element == this)
				return NULL;
//			element = element->GetLeftNeighbour(referenceElement, searchChildrenOnly);
		}
	}
	// Nothing found? Then set ourselves to be this sought after element!
	if (!element)
	{
		element = GetElementClosestToSelf(true);
	}
	// For some UI elements, such as lists, the list itself is not navigatable, so query it to get the first element in its list if so here.
	if (element)
		element = element->GetNavigationElement(NavigateDirection::Left);

	return element;
}

/** For some UI elements, such as lists, the list itself is not navigatable, so query it to get the first element in its list if so here.
		By default returns itself. */
UIElement * UIElement::GetNavigationElement(NavigateDirection direction) {
	return this;
}

// Is it navigatable?
bool UIElement::IsNavigatable() {
	if (!this->IsVisible())
		return false;
	if (!interaction.navigatable)
		return false;
	return true;

}

/// Works recursively.
UIElement * UIElement::GetElementClosestToSelf(bool mustBeActivatable) {
	return GetElementClosestTo(layout.position, mustBeActivatable);
}


UIElement * UIElement::GetElementClosestTo(Vector3f & position, bool mustBeActivatable /*= false*/)
{
	List<UIElement*> allChildren = this->AllChildrenR();
	UIElement * closest = NULL;
	float closestDistance = 1000000.f;
	for (int i = 0; i < allChildren.Size(); ++i)
	{
		UIElement * child = allChildren[i];
		if (mustBeActivatable && !child->interaction.activateable)
			continue;
		float dist = (child->layout.position - position).Length();
		if (dist < closestDistance)
		{
			closest = child;
			closestDistance = dist;
		}
	}
	// Fetch self if no neighbour close enough.
	return closest;
}

UIElement * UIElement::GetElementClosestToInX(Vector3f & position, bool mustBeActivatable)
{
	List<UIElement*> allChildren = this->AllChildrenR();
	UIElement * closest = NULL;
	float closestDistance = 1000000.f;
	for (int i = 0; i < allChildren.Size(); ++i)
	{
		UIElement * child = allChildren[i];
		if (mustBeActivatable && !child->interaction.activateable)
			continue;
		float dist = AbsoluteValue(child->layout.position.x - position.x);
		if (dist < closestDistance)
		{
			closest = child;
			closestDistance = dist;
		}
		if (dist == closestDistance)
		{
			// Same x? Check closer y.
			if (AbsoluteValue(child->layout.position.y - position.y) < AbsoluteValue(closest->layout.position.y - position.y))
				closest = child; // Same distance in X, so don't adjust the closestDistance.
		}
	}
	// Fetch self if no neighbour close enough.
	return closest;
}
UIElement * UIElement::GetElementClosestToInY(Vector3f & position, bool mustBeActivatable)
{
	List<UIElement*> allChildren = this->AllChildrenR();
	UIElement * closest = NULL;
	float closestDistance = 1000000.f;
	for (int i = 0; i < allChildren.Size(); ++i)
	{
		UIElement * child = allChildren[i];
		if (mustBeActivatable && !child->interaction.activateable)
			continue;
		float dist = AbsoluteValue(child->layout.position.y - position.y);
		if (dist < closestDistance)
		{
			closest = child;
			closestDistance = dist;
		}
		if (dist == closestDistance)
		{
			// Same y? Check closer x.
			if (AbsoluteValue(child->layout.position.x - position.x) < AbsoluteValue(closest->layout.position.x - position.x))
				closest = child; // Same distance in y, so don't adjust the closestDistance.
		}
	}
	// Fetch self if no neighbour close enough.
	return closest;
}

// Sets the selected flag to false for all children.
void UIElement::DeselectAll(){
	for (int i = 0; i < children.Size(); ++i){
		children[i]->DeselectAll();
	}
	interaction.selected = false;
}

UIElement* UIElement::GetElementByPos(int i_posX, int i_posY){

	UIElement* result = 0;
	// Don't process invisible UIElements, please.
	if (interaction.visible == false)
		return 0;
	// Check if the mouse is outside the element's boundaries.
	if (layout.IsOutside(i_posX, i_posY)){
		// Return false if we are outside of the boundaries,
		// since we haven't found the selected element.
		return 0;
	}

	// Alright, the mouse is inside this element!
	// Do we have children?
	for (int i = 0; i < children.Size(); ++i){
		result = children[i]->GetElementByPos((int)(i_posX - layout.posX), (int)(i_posY - layout.posY));
		// If so, check if they are the active element.
		if (result != 0){
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
	UIElement * result = 0;
	for (int i = 0; i < children.Size(); ++i){
		result = children[i]->GetActiveElement();
		if (result)
			return result;
	}
	return 0;
}

/// Getter for element by state, where the stateFlag will be bitwise anded (&) to fetch the correct element.
UIElement* UIElement::GetElementByState(int stateFlag)
{
	if (state & stateFlag)
		return this;
	UIElement * result = NULL;
	for (int i = 0; i < children.Size(); ++i)
	{
		UIElement * child = children[i];
		if (!child->interaction.visible)
			continue;
		result = child->GetElementByState(stateFlag);
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
	for (int i = 0; i < children.Size(); ++i){
		/// Skip invisible children by default.
		if (!children[i]->interaction.visible)
			continue;
		children[i]->GetElementsByState(stateFlags, listToFill);
	}
	return true;
}

/// Checks for visibility, activateability, etc. Also works bit-wise!
UIElement * UIElement::GetElementByFlag(int uiFlags)
{
	// Check stuff
	if (ConformsToFlags(uiFlags))
		return this;
	UIElement * result = NULL;
	for (int i = 0; i < children.Size(); ++i){
		/// Skip invisible children by default.
		if (!children[i]->interaction.visible)
			continue;
		result = children[i]->GetElementByFlag(uiFlags);
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
	for (int i = 0; i < children.Size(); ++i){
		/// Skip invisible children by default.
		if (!children[i]->interaction.visible)
			continue;
		children[i]->GetElementsByFlags(uiFlags, listToFill);
	}
	return true;
}

/// Checks if all flags are true. See UIFlag namespace. Flags can be binary &-ed.
bool UIElement::ConformsToFlags(int uiFlags)
{
	bool isThisOne = true;
	if (uiFlags & UIFlag::VISIBLE)
		isThisOne = isThisOne & interaction.visible;
	if (uiFlags & UIFlag::HOVERABLE)
		isThisOne = isThisOne & interaction.hoverable;
	if (uiFlags & UIFlag::ACTIVATABLE)
		isThisOne = isThisOne & interaction.activateable;
	if (state & UIState::DISABLED)
		return false;
	if (isThisOne)
		return true;
	return false;
}

/// Called upon hovering on an element. By default queues the string set in onHover to be processed by the message manager and game state.
void UIElement::OnHover()
{
	// Send it's onHover message!
	if (onHover.Length())
	{
		Message * message = new Message(MessageType::ON_UI_ELEMENT_HOVER);
		message->msg = onHover;
		message->element = this;
		MesMan.QueueMessage(message);
	}
}


UIElement* UIElement::GetElementWithID(int elementID){
	if (id == elementID)
		return this;
	UIElement * result = NULL;
	for (int i = 0; i < children.Size(); ++i){
		result = children[i]->GetElementWithID(elementID);
		if (result)
			return result;
	}
	return NULL;
}

/// Checks if this element is visible, as it will depend on the parent UIs too.
bool UIElement::IsVisible()
{
	if (!this->interaction.visible)
	{
		return false;
	}
	if (parent && !parent->IsVisible())
		return false;
	return true;
}

/// Gets absolute position and stores them in the pointers' variables, in pixels, relative to upper left corner
Vector2i UIElement::GetAbsolutePos()
{
	Vector2i absPos ((int) floor(layout.posX+0.5f), (int) floor(layout.posY+0.5f));
	UIElement* pp = parent;
	// While parent pointer is non-NULL
	while(pp){
		// Increment result
		absPos += pp->pageBegin;
		pp = pp->parent;
	}
	// Increment positions
	return absPos;
}
	
/// Absolute positions, in pixels, relative to upper left corner
int UIElement::GetAbsolutePosX(){
	int result = (int)layout.posX;
	if (parent)
		result += parent->GetAbsolutePosX();
	return result;
}
int UIElement::GetAbsolutePosY(){
	int result = (int)layout.posY;
	if (parent)
		result += parent->GetAbsolutePosY();
	return result;
}

#include <iomanip>

/// For debugging, prints info and tree-structure.
void UIElement::Print(int level){
    std::cout<<"\n";
    if (!interaction.visible)
        std::cout<<"Not visible ";
    std::cout<<std::setfill(' ')<<std::setw(level)<<level<<" - "<<name<<", "<<visuals.textureSource;
    if (!interaction.visible)
        return;
    for (int i = 0; i < children.Size(); ++i)
        children[i]->Print(level+1);
}

// Movement functions. These take into consideration the parent element and UIType
void UIElement::MoveX(int distance){
	layout.MoveX(type, distance, parent->layout.sizeX);
}

void UIElement::MoveY(int distance){
	layout.MoveY(type, distance, parent->layout.sizeY);
}

// Moves to specified position. Takes addresses as parameters for given values in order to confirm validity.
// Ignores value if pointer is NULL
void UIElement::MoveTo(int * x, int * y){
	layout.MoveTo(type, x, y);
}

/// Sets name of the element
void UIElement::SetName(const String i_name){
	name = i_name;
}

/// Returns a pointer to specified UIElement
UIElement * UIElement::GetElementByName(String i_name, UIFilter filter){
	if (i_name == name) {
		if (filter == UIFilter::Visible && !interaction.visible)
			return nullptr;
		return this;
	}
	UIElement * result;
	for (int i = 0; i < children.Size(); ++i){
        UIElement * child = children[i];
        assert(child && "NULL-child? wtf?");
   //     std::cout<<"\nChild name: "<<child->name;
		String childName = children[i]->name;
		result = children[i]->GetElementByName(i_name, filter);
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
	for (int i = 0; i < children.Size() && result == NULL; ++i)
	{
		result = children[i]->GetElementBySource(i_source);
	}
	return result;
}

/// Getterrr
const UIElement * UIElement::GetChild(int index){
	assert(index >= 0 && index < children.Size());
	return children[index];
}

/// Adds x children. Subclassed in e.g. Matrix-class in order to setup contents properly.
bool UIElement::AddChildren(GraphicsState * graphicsState, List<UIElement*> children)
{
	// No options? Just add children.. even tho it will look like crap.
	for (int i = 0; i < children.Size(); ++i)
	{
		AddChild(graphicsState, children[i]);
	}
	return true;
}

// Structurization
bool UIElement::AddChild(GraphicsState* graphicsState, UIElement *in_child)
{
    assert(in_child);
    if (in_child == NULL)
        return false;
	if (in_child->name.Length() == 0)
	{
		in_child->name = name + "Child";
	}
	children.Add(in_child);
	// Set it's parent to this.
	in_child->parent = this;
	in_child->ui = ui;
	return true;
}

/// Attempts to remove said child from this element. Returns false if it was not a valid child. (thus action unnecessary).
bool UIElement::RemoveChild(GraphicsState* graphicsState, UIElement * element)
{
	if (children.RemoveItemUnsorted(element))
	{
		element->parent = NULL;
		return true;
	}
	return false;
}


bool UIElement::AddToParent(GraphicsState* graphicsState, String parentName, UIElement * child)
{
    UIElement * padre = GetElementByName(parentName);
    if (padre){
        padre->AddChild(graphicsState, child);
        return true;
    }
	std::cout<<"\nUnable to add element "<<child->name<<" to parent \'"<<parentName<<"\'. Could not find parent within UI.";
    return false;
}

void UIElement::SetRootDefaults(UserInterface * ui) {
	name = "root";
	interaction.exitable = false;
	interaction.selectable = false;
	interaction.activateable = false;
	visuals.textureSource = "0x0000"; // Alpha texture.
	// Link it.
	ui = ui;
}

void UIElement::SetParent(UIElement *in_parent){
	parent = in_parent;
}

/// Checks if the target element is somehow a part of this list. If it is, the function will return the index of the child it is or belongs to. Otherwise -1 will be returned.
int UIElement::BelongsToChildIndex(UIElement * search)
{
	if (!search)
		return -1;
	int index = children.GetIndexOf(search);
	while(search->parent && index == -1)
	{
		index = children.GetIndexOf(search);
		search = search->parent;
	}
	return index;
}


/// Queues the UIElement to be buffered via the GraphicsManager
void UIElement::QueueBuffering()
{
	std::cout<<"Deprecated";
	; Graphics.QueueMessage(new GMBufferUI(this));
}

/// Bufferizes the UIElement mesh into the vbo buffer
void UIElement::Bufferize()
{
	visuals.Bufferize();
	// Bufferize children too!
	for (int i = 0; i < children.Size(); ++i) {
		children[i]->Bufferize();
	}
}

// Does it again!
void UIElement::Rebufferize(GraphicsState& graphicsState) {
	// Check if already buffered. If so re-create the mesh in it's entirety.
	if (IsBuffered())
	{
		// Recalculate bounds depending on parent.
		AdjustToParent(&graphicsState);
		if (IsGeometryCreated())
			ResizeGeometry(&graphicsState);
		else
			CreateGeometry(&graphicsState);
	}
	Bufferize();
}


/// Releases resources used by the UIElement. Should only be called by a thread with valid GL context!
void UIElement::FreeBuffers(GraphicsState& graphicsState)
{
	visuals.FreeBuffers(graphicsState);

	for (int i = 0; i < children.Size(); ++i){
		children[i]->FreeBuffers(graphicsState);
	}
	for (int i = 0; i < borderElements.Size(); ++i)
		borderElements[i]->FreeBuffers(graphicsState);
}


/// Render, public function that calls internal render functions.
void UIElement::Render(GraphicsState & graphicsState)
{
    // Push matrices
	Matrix4d tmp = graphicsState.modelMatrixD;

    // Render ourself and maybe children.
    RenderSelf(graphicsState);
	PrintGLError("GLError in UIElement::Render after RenderSelf");
	
    if (children.Size())
        RenderChildren(graphicsState);
	PrintGLError("GLError in UIElement::Render after RenderChildren");

	RenderBorders(graphicsState);
	PrintGLError("GLError in UIElement::Render after RenderBorders");

	// Pop matrices
	graphicsState.modelMatrixF = graphicsState.modelMatrixD = tmp;
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
void UIElement::RenderSelf(GraphicsState & graphicsState)
{
	if (!IsBuffered()) { // Shouldn't get here, but just in case..
		AdjustToParentCreateGeometryAndBufferize(&graphicsState);
	}

	visuals.UpdateHighlightColor(IsDisabled(), state & UIState::ACTIVE, state & UIState::HOVER);
	visuals.Render(graphicsState);
	RenderText(graphicsState);
}

void UIElement::RenderText(GraphicsState & graphicsState)
{
	text.RenderText(
		graphicsState,
		layout,
		IsDisabled(),
		HasState(UIState::HOVER),
		HasState(UIState::TOGGLED),
		HasState(UIState::ACTIVE),
		visuals.highlightOnHover);
}

void UIElement::FormatText(GraphicsState& graphicsState)
{
	text.FormatText(graphicsState, layout);
}

#include "UIBorder.h"

UIElement * UIElement::CreateBorderElement(GraphicsState* graphicsState, String textureSource, char alignment) {
	UIBorder * borderElement = new UIBorder();
	borderElement->name = parent->name + " border "+ String(alignment);
	borderElement->visuals.textureSource = textureSource;
	borderElement->parent = this;
	Texture * texture = TexMan.GetTexture(textureSource);
	if (texture == nullptr) {
		LogGraphics("Unable to fetch texture for border: " + textureSource + " for element " + name, INFO);
		delete borderElement;
		return nullptr;
	}
	borderElement->layout.alignment = alignment;
	borderElement->visuals.highlightOnHover = true;
	switch (alignment) {
	case TOP:
		borderElement->layout.sizeY = texture->size.y;
		borderElement->layout.lockSizeY = true;
		break;
	case BOTTOM:
		borderElement->layout.sizeY = texture->size.y;
		borderElement->layout.lockSizeY = true;
		break;
	case RIGHT:
		borderElement->layout.sizeX = texture->size.x;
		borderElement->layout.lockSizeX = true;
		break;
	case TOP_RIGHT:
		borderElement->layout.sizeY = texture->size.y;
		borderElement->layout.lockSizeY = true;
		borderElement->layout.sizeX = texture->size.x;
		borderElement->layout.lockSizeX = true;
		break;
	default:
		assert(false && "Implemenet");
	}
	borderElements.Add(borderElement);
	
	borderElement->AdjustToParentCreateGeometryAndBufferize(graphicsState);
	return borderElement;
}

void UIElement::RenderBorders(GraphicsState& graphicsState) {
	if (topBorderTextureSource.Length() > 0) {
		if (topBorder == nullptr) {
			topBorder = CreateBorderElement(&graphicsState, topBorderTextureSource, TOP);
		}
		topBorder->Render(graphicsState);
	}
	if (bottomBorderTextureSource.Length() > 0) {
		if (bottomBorder == nullptr) {
			bottomBorder = CreateBorderElement(&graphicsState, bottomBorderTextureSource, BOTTOM);
		}
		bottomBorder->Render(graphicsState);
	}

	if (rightBorderTextureSource.Length() > 0) {
		if (rightBorder == nullptr)
			rightBorder = CreateBorderElement(&graphicsState, rightBorderTextureSource, RIGHT);
		if (rightBorder)
			rightBorder->Render(graphicsState);
	}
	if (topRightCornerTextureSource.Length() > 0) {
		if (topRightCorner == nullptr)
			topRightCorner = CreateBorderElement(&graphicsState, topRightCornerTextureSource, TOP_RIGHT);
		topRightCorner->Render(graphicsState);
	}
}

void UIElement::RenderChildren(GraphicsState & graphicsState)
{
	Vector4d initialPositionTopRight(layout.right, layout.top, 0, 1),
			initialPositionBottomLeft(layout.left, layout.bottom, 0, 1);
	Vector3f currTopRight = graphicsState.modelMatrixF * initialPositionTopRight;
	Vector3f currBottomLeft = graphicsState.modelMatrixF * initialPositionBottomLeft;

	Rect previousScissor = graphicsState.scissor;
	Rect uiRect(currBottomLeft[0], currBottomLeft[1], currTopRight[0], currTopRight[1]);
	Rect uiScissor = previousScissor.Intersection(uiRect);

	// Set scissor! o.o
	graphicsState.SetGLScissor(uiScissor);

    // Render all children
	for (int i = 0; i < children.Size(); ++i)
	{
        UIElement * child = children[i];
        if (!child->interaction.visible)
            continue;
		// Render
		child->Render(graphicsState);
		// Reset scissor if needed.
		graphicsState.SetGLScissor(uiScissor);
	}
	graphicsState.SetGLScissor(previousScissor);
}


/// Adjusts the UI element size and position relative to new AppWindow size
void UIElement::AdjustToWindow(GraphicsState& graphicsState, const UILayout& windowLayout)
{
	int availableParentSizeX = parent ? parent->AvailableParentSizeX() : windowLayout.right - windowLayout.left;
	int centeredContentParentPosX = parent ? parent->CenteredContentParentPosX() : (windowLayout.right + windowLayout.left) / 2;
	layout.AdjustToWindow(windowLayout,	
		parent ? &parent->layout : &windowLayout,
		availableParentSizeX,
		centeredContentParentPosX, interaction.moveable);

	/// Set the new dimensions
	if (visuals.mesh)
		visuals.mesh->SetDimensions((float)layout.left, (float)layout.right, (float)layout.bottom, (float)layout.top, layout.zDepth);

    /// Reset text-variables so that they are re-calculated before rendering again.
    text.FormatText(graphicsState, layout);

	/// Adjust all children too
	for (int i = 0; i < children.Size(); ++i){
		assert(children[i] != this);
		children[i]->AdjustToWindow(graphicsState, layout);
	}
}

/// Calls AdjustToWindow for parent's bounds. Will assert if no parent is available.
void UIElement::AdjustToParent(GraphicsState* graphicsState)
{
	if (!parent)
		return;
	AdjustToWindow(*graphicsState, parent->layout);
	text.SetPreviousTextSizeRatio(1.f); // Reset to allow larger text if we are using the minimum criteria for rapdily changing text-slots ( such as distance).

	for (int i = 0; i < children.Size(); ++i) {
		children[i]->AdjustToParent(graphicsState);
	}
}

void UIElement::AdjustToParentCreateGeometryAndBufferize(GraphicsState* graphicsState) {
	AdjustToParent(graphicsState);
	CreateGeometry(graphicsState);
	Bufferize();
}

// Unbufferizes and deletes all border elements and their references.
void UIElement::DeleteBorders(GraphicsState* graphicsState) {
	for (int i = 0; i < borderElements.Size(); ++i)
		borderElements[i]->FreeBuffers(*graphicsState);
	borderElements.ClearAndDelete();

	rightBorder = topBorder = bottomBorder = nullptr;
	topRightCorner = nullptr;
}

// Sets it to override.
void UIElement::SetTextColors(const TextColors& overrideColors) {
	text.SetColors(overrideColors);
}

// Overrides, but only during onHover.
void UIElement::SetOnHoverTextColor(const Color& newOnHoverTextColor) {
	text.onHoverTextColor = new Color(newOnHoverTextColor);
}


/** Sets slider level by adjusting it's child's position.
	Values should be within the range [0.0,1.0]. **/
void UISlider::SetLevel(float level){
	if(!children.Size()){
		std::cout<<"\nSlider "<<this->name<<" lacking child (sliderHandle)!";
		return;
	}
	children[0]->layout.alignmentX = level;
	return;
}
/// Returns the slider's level
float UISlider::GetLevel(){
	if (!children.Size()){
		std::cout<<"\nERROR: Unable to get slider level as it has no child (handle)!";
		return 0;
	}
	return children[0]->layout.alignmentX;
}

// Creates the Square mesh used for rendering the UIElement and calls SetDimensions with it's given values.
void UIElement::CreateGeometry(GraphicsState* graphicsState)
{
	visuals.CreateGeometry(graphicsState, layout);
	for (int i = 0; i < children.Size(); ++i) {
		children[i]->CreateGeometry(graphicsState);
	}
}

void UIElement::ResizeGeometry(GraphicsState* graphicsState)
{
	assert(visuals.IsGeometryCreated());
	assert(visuals.mesh);
	visuals.ResizeGeometry(graphicsState, layout);

	DeleteBorders(graphicsState);

	for (int i = 0; i < children.Size(); ++i){
		children[i]->ResizeGeometry(graphicsState);
	}

	text.SetPreviousTextSizeRatio(1.f); // Reset to allow larger text if we are using the minimum criteria for rapdily changing text-slots ( such as distance).

	// Always re-bufferize after resizing geos.
	Bufferize();
}

void UIElement::DeleteGeometry()
{
//	assert(mesh);
	visuals.DeleteGeometry();

	for (int i = 0; i < children.Size(); ++i){
		children[i]->DeleteGeometry();
	}
	for (int i = 0; i < borderElements.Size(); ++i)
		borderElements[i]->DeleteGeometry();
}

void UIElement::InheritDefaults(UIElement * child) {
	// Inherit some defaults?
	child->text = text;
	child->layout.padding = layout.padding;
	if (text.GetColors() != nullptr)
		child->SetTextColors(*text.GetColors());
};

void UIElement::SetState(int newState) {
	state = newState;
}

/// For example UIState::HOVER, not to be confused with flags! State = current, Flags = possibilities
bool UIElement::AddState(GraphicsState* graphicsState, int i_state, bool force)
{
	if (!AddStateSilently(i_state, force))
		return false;
	OnStateAdded(graphicsState, i_state);
	return true;
}

bool UIElement::AddState(GraphicsState* graphicsState, int state, UIFilter filter) {
	if (!AddStateSilently(state, false))
		return false;
	OnStateAdded(graphicsState, state);
	if (filter == UIFilter::Recursive)
		for (int i = 0; i < children.Size(); ++i) {
			children[i]->AddState(graphicsState, state, filter);
		}
	return true;
}

bool UIElement::AddStateSilently(int i_state, bool force) {
	if (force) {
	}
	else { // Check early out if not forcing.
		// Return if trying to add invalid state.
		if (!interaction.hoverable && i_state & UIState::HOVER)
			return false;
		// Don't allow activating an non-activatable element.
		if (!interaction.activateable && i_state & UIState::ACTIVE)
			return false;
		// Ignore activating if it's disabled.
		if (HasState(UIState::DISABLED) && i_state & UIState::ACTIVE)
			return false;
	}

	if (i_state == UIState::HOVER)
	{
		if (inputState->demandActivatableForHoverElements && !interaction.activateable)
			return false;
		if (inputState->demandHighlightOnHoverForHoverElements && !visuals.highlightOnHover)
			return false;
		for (int i = 0; i < borderElements.Size(); ++i) {
			borderElements[i]->state |= i_state;
		}
	}
	state |= i_state;
	return true;
}

// For sub-classes to adjust children as needed (mainly for input elements).
void UIElement::OnStateAdded(GraphicsState* graphicsState, int state) {
	if (state == UIState::ACTIVE) {
		bool didSomething = false;
		for (int i = 0; i < activationActions.Size(); ++i) {
			activationActions[i].Process(graphicsState, this);
			didSomething = true;
		}
		if (didSomething)
			return;
		/// It it us! Activate power!
		if (activationMessage.Length() == 0)
		{
			LogGraphics("Activatable UI element has no valid activation message string!", INFO);
		}
		else if (activationMessage.Length() != 0)
		{
			List<String> msgs = activationMessage.Tokenize("&");
			for (int i = 0; i < msgs.Size(); ++i)
			{
				Message * message = new Message(msgs[i]);
				message->element = this;
				MesMan.QueueMessage(message);
			}
		}
	}
}


// See UIState::
bool UIElement::HasState(int queryState) {
	return state & queryState;
}

bool UIElement::HasStateRecursive(int queryState) {
	if (state & queryState)
		return true;
	for (int i = 0; i < children.Size(); ++i) {
		UIElement * child = children[i];
		if (child->HasStateRecursive(queryState))
			return true;
	}
	return false;
}



/// For example UIState::HOVER, if recursive will apply to all children.
void UIElement::RemoveState(int statesToRemove, bool recursive /* = false*/){
	state &= ~statesToRemove;
	if (recursive){
		for (int i = 0; i < children.Size(); ++i){
			children[i]->RemoveState(statesToRemove, recursive);
		}
	}

	for (int i = 0; i < borderElements.Size(); ++i) {
		borderElements[i]->state &= ~statesToRemove;
	}
}

void UIElement::RemoveState(int statesToRemove, UIFilter filter) {
	state &= ~statesToRemove;
	for (int i = 0; i < borderElements.Size(); ++i) {
		borderElements[i]->state &= ~statesToRemove;
	}
	if (filter == UIFilter::Recursive)
		for (int i = 0; i < children.Size(); ++i) {
			children[i]->RemoveState(statesToRemove, filter);
		}
}

// When navigating, either via control, or arrow keys or whatever.
void UIElement::Navigate(NavigateDirection direction) {
}


/// Take care when using!
void UIElement::SetFlags(UIFlag flags){
	assert(false);
	/// RETHINK MAN!
	// state |= flags;
}

// Works recursively.
void UIElement::RemoveFlags(UIFlag flags){
	int antiFlag = ~flags;
	if (state & flags){
//		std::cout<<"\nFound UIElement "<<name<<" with specified flags to remove: "<<flags;
	}
	state &= antiFlag;
	if (state == 0)
		state = UIState::IDLE;
	for (int i = 0; i < children.Size(); ++i){
		children[i]->RemoveFlags(flags);
	}
}

#include "UI/UILabel.h"

UILabel::UILabel(String name /*= ""*/)
: UIElement()
{
	this->name = name;
	SetText(name);
	type = UIType::LABEL;
	interaction.hoverable = false;
	interaction.navigatable = false;
	visuals.highlightOnHover = false;
	interaction.selectable = interaction.activateable = false;
};

UILabel::~UILabel()
{
//	std::cout<<"\nUILabel destructor";
}

UISliderHandle::UISliderHandle()
: UIElement()
{
	type = UIType::SLIDER_HANDLE;
	interaction.moveable = true;
};

UISliderHandle::~UISliderHandle()
{
//	std::cout<<"\nUISliderHandle destructor";
}

UISlider::UISlider()
: UIElement()
{
	type = UIType::SLIDER_BAR;
	progress = 0;
};

UISlider::~UISlider()
{
//	std::cout<<"\nUISlider destructor";
}


/** Used by input-captuing elements. Calls recursively upward until an element wants to respond to the input.
	Returns 1 if it processed anything, 0 if not.
*/
UIInputResult UIElement::OnKeyDown(GraphicsState* graphicsState, int keyCode, bool downBefore)
{
	if (parent)
		parent->OnKeyDown(graphicsState, keyCode, downBefore);
	return UIInputResult::NoUpdate;
}
/// Used for getting text. This will be local translated language key codes?
UIInputResult UIElement::OnChar(int asciiCode){
	assert(false);
	return UIInputResult::NoUpdate;
}


/// Called to ensure visibility of target element. First call should be made to the target element with a NULL-argument!
void UIElement::EnsureVisibility(GraphicsState* graphicsState, UIElement * element)
{
	// If this is a "system" element, i.e. scroll-bars that are automatically added, skip this step, as they should always be visible.
	if (isSysElement)
		return;
	if (!element)
		element = this;
	if (parent)
		parent->EnsureVisibility(graphicsState, this);
}

/// Sets color, but may also update corner/edge elements to use the same color multiplier as well.
void UIElement::SetColor(Vector4f color) {
	this->visuals.color = color;
	for (int i = 0; i < borderElements.Size(); ++i) {
		borderElements[i]->visuals.color = color;
	}
}

