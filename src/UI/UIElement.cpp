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

extern InputManager input;
extern UserInterface ui[GameStateID::MAX_GAME_STATES];

int UIElement::idEnumerator = 0;
String UIElement::defaultTextureSource; //  = "80Gray50Alpha.png";
Vector4f UIElement::defaultTextColor = Vector4f(1,1,1,1);

/// Called when this UI is made active (again).
void UIElement::OnEnterScope(){
	// Do nothing in general.
	for (int i = 0; i < children.Size(); ++i)
		children[i]->OnEnterScope();
}
/// Called once this element is no longer visible for any reason. E.g. switching game states to display another UI.
void UIElement::OnExitScope()
{
	/// Skip those which have already exited scope.
	if (!this->visible)
		return;
	// Do nothing in general.
	for (int i = 0; i < children.Size(); ++i)
		children[i]->OnExitScope();
	if (onExit.Length())
		MesMan.QueueMessages(onExit);
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
	lineSizeRatio = -1.f;
	childrenCreated = false;
	/// ID
	id = idEnumerator++;
	/// Hierarchy
	parent = NULL;
	ui = NULL;
	divider = Vector2f(0.5f, 0.5f);
	// Graphical properties
	mesh = NULL;
	texture = NULL;
	vboBuffer = -1;
	vboVertexCount = 0;
	zDepth = 0;
	noLabel = false;

	previousTextSizeRatio = 1.f;
	alignment = NULL_ALIGNMENT;	// Alignment relative to parent
	scalable = DEFAULT_SCALABILITY;		// Allow scaling depending on AppWindow size?
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

	inStack = false;

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

	// Text.
	label = NULL;
	textSizeRatio = 1.0f;
	currentTextSizeRatio = -1.0f;

	this->textureSource = defaultTextureSource;
	fontSource = TextFont::defaultFontSource;
	font = NULL;
	text.color = defaultTextColor;

	/** Will enable/disable cyclicity of input navigation when this element is pushed. When popped, the next element in the stack will determine cyclicity. */
	cyclicY = true;

	/// When true, re-directs all (or most) keyboard input to the target element for internal processing when active. Must be subclass of UIInput as extra functions there are used for this.
	demandInputFocus = false;

	/// Neighbour pointers
	upNeighbour = rightNeighbour = leftNeighbour = downNeighbour = NULL;

	/// GL related.
	vertexArray = -1;

	isBuffered = isGeometryCreated = false;
	color = Vector4f(1,1,1,1);
}

UIElement::UIElement(){
	Nullify();
}

void UIElement::CreateChildren(){}

/// Copy-cosntructor.
UIElement::UIElement(const UIElement & ref){
	Nullify();
	zDepth = ref.zDepth;
	alignment = ref.alignment;	// Alignment relative to parent
	scalable = ref.scalable;		// Allow scaling depending on AppWindow size?
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
}

UIElement::~UIElement()
{
//	std::cout<<"\nUIElement destructor";
	/// Hierarchy
	parent = NULL;
	/// Use for-loop instead of crazy recursion for deallocating the children
	children.ClearAndDelete();

	/// Deallocate texture and mesh if needed, as well as vbo, we should never do that here though!
	assert(vboBuffer == -1 && "vboBuffer not released in UIElement::~UIElement()!");
	/// Textures will be deallocated by the texture manager...
	/// But take care of the mesh!
	SAFE_DELETE(mesh);
	/// Inform the input manager that we delete the element.
	// InputMan.OnElementDeleted(this);
}

/// Sets the bufferized flag. Should only be called before program shutdown. Ensures less assertions will fail.
void UIElement::SetBufferized(bool bufferizedFlag)
{
	if (bufferizedFlag == false)
		vboBuffer = -1;
	for (int i = 0; i < children.Size(); ++i)
	{
		children[i]->SetBufferized(bufferizedFlag);
	}
}	

/// Callback-function for sub-classes to implement own behaviour to run within the UI-class' code.
void UIElement::Proceed()
{
}

// Used for handling things like drag-n-drop and copy-paste operations, etc. as willed.
void UIElement::ProcessMessage(Message * message)
{
}

/// Sets text, queueing recalculation of the rendered variant.
void UIElement::SetText(CTextr newText, bool force)
{
	/// Check that it's not currently being edited. If so cancel this setting.
	if (this->demandInputFocus && state & UIState::ACTIVE && !force){
		return;
	}
	this->text = newText;
	/// Reset text-variables so that they are re-calculated before rendering again.
	currentTextSizeRatio = -1.0f;
}

/** Fetches texture, assuming the textureSource has been set already. Binds and bufferizes, so call only from graphics thread. 
	Returns false if no texture could be find, bind or bufferized. */
bool UIElement::FetchBindAndBufferizeTexture()
{
	// When rendering an objectwith this program.
	glActiveTexture(GL_TEXTURE0 + 0);	
	/// Grab texture?
	bool validTexture = false;
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
            glBindTexture(GL_TEXTURE_2D, 0);
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
		return false;
	}
	if (texture != NULL && texture->glid == -1){
		TexMan.BufferizeTexture(texture);
	}
	if (!texture)
		return false;
	OnTextureUpdated();
	return true;
}

/// Called after FetchBindAndBufferizeTexture is called successfully. (may also be called other times).
void UIElement::OnTextureUpdated()
{
}

/// Recalculates and sets highlighting factors used when rendering the UI (dedicated shader settings)
void UIElement::UpdateHighlightColor()
{
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
	Shader * shader = ActiveShader();
	if (!shader)
		return;

//	assert(activeShader->uniformPrimaryColorVec4 != -1);
	if (shader->uniformPrimaryColorVec4 == -1)
	{
		std::cout<<"\nUI shader lacking primary color?";
	}
	glUniform4f(shader->uniformPrimaryColorVec4,
		baseColor[0], baseColor[1], baseColor[2], color[3]);
	if (shader->uniformHighlightColorVec4 == -1)
	{
		std::cout<<"\nUI shader lacking highlight color?";
	}
	//assert(activeShader->uniformHighlightColorVec4 != -1);
	glUniform4f(shader->uniformHighlightColorVec4,
		highlightColor[0], highlightColor[1], highlightColor[2], 0.0f);
}


void UIElement::DeleteElement(int targetID){
	UIElement* target = NULL;
	children.ClearAndDelete();
}

/** Deletes target element if it is found.
	It will also unbufferize and free resources as necessary. Should ONLY be called from the render-thread!
*/
bool UIElement::Delete(UIElement * element){
	/// If found it, delete it.
	if (children.Exists(element))
	{
		children.Remove(element);
		/// Free GL buffers and deallocate it!
		element->FreeBuffers();
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
void UIElement::Clear() 
{
	// Unbufferize first?
	if (isBuffered)
		for (int i = 0; i < children.Size(); ++i)
		{
			UIElement * child = children[i];
			child->FreeBuffers();
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
UIElement * UIElement::Hover()
{
	AddState(UIState::HOVER);
	this->OnHover();
	return this;
}	

// Returns true once the highest-level appropriate element has been selected,
// or it has been determined that the selected child was not the active one.
// A value of true will thus make the algorithm return true and ending it's calculations.
UIElement* UIElement::Hover(int mouseX, int mouseY)
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
	/// Process stuff that should happen when hovering on this element!
	OnHover();

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
	for (int i = children.Size()-1; i >= 0; --i)
	{
	    UIElement * child = children[i];
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

	// If we weren't hovering over it before.
	if (! (state & UIState::HOVER))
	{
		this->OnHover();
	}
	// The mouse is inside this element,
	// and our children (if any) were not the active Entity.
	// This means that this element must be the active Entity!
	if (!AddState(UIState::HOVER))
		return NULL;
	
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
UIElement* UIElement::Click(int mouseX, int mouseY)
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
			return 0;
	}

	// Check axiomaticness (direct-activation without further processing)
	if (axiomatic){
		if (activateable){
			state |= UIState::ACTIVE;
			return this;
		}
		return 0;
	}

	// Alright, the mouse is inside this element!
	// Do we have children?
	for (int i = children.Size()-1; i >= 0; --i){
		UIElement * child = children[i];
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
	if (this->activateable){
		state |= UIState::ACTIVE;
		return this;
	}
	// If not, return false, since we haven't foun the right element.
	return NULL;
}

// Returns a non-0 message once the highest-level appropriate element has been found.
// No co-ordinates are required since we will instead require the element to already
// clicked.
UIElement* UIElement::Activate()
{
	UIElement* result = 0;
	// Don't process invisible UIElements, please.
	if (visible == false)
		return 0;
	// Alright, the mouse is inside this element!
	// Do we have children?
	for (int i = children.Size()-1; i >= 0; --i){
		UIElement * child = children[i];
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
		bool didNothing = true;
		/// It it us! Activate power!
		if (activationMessage.Length() == 0)
		{
			std::cout<<"Activatable UI element has no valid activation message string!";
		}
		else if (activationMessage.Length() != 0)
		{
			didNothing = false;
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
			didNothing = false;
			UIAction & uia = activationActions[i];
			uia.Process(this);
		}
		if (didNothing)
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
	if (visible == false)
		return 0;

	// Check if the mouse is outside the element's boundaries.
	if (mouseX > right || mouseX < left ||
		mouseY > top || mouseY < bottom)
    {
        return NULL;
	}
    // Do we have children?
	for (int i = 0; i < children.Size(); ++i){
		UIElement * child = children[i];
	    if (!child->visible)
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
		if (child->activateable)
			return true;
		if (child->HasActivatables())
			return true;
	}
	return false;
}

UIElement * UIElement::GetElement(String byName, int andType)
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
bool UIElement::OnScroll(float delta)
{
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

void UIElement::InheritNeighbours(UIElement * fromElement)
{
	if (!leftNeighbourName.Length())
		leftNeighbourName = fromElement->leftNeighbourName;
    if (!rightNeighbourName.Length())
		rightNeighbourName = fromElement->rightNeighbourName;
	if (!downNeighbourName.Length())
		downNeighbourName = fromElement->downNeighbourName;
	if (!upNeighbourName.Length())
		upNeighbourName = fromElement->upNeighbourName;
}


/// Suggests a neighbour which could be to the top of this element. Meant to be used for UI-navigation support. The reference element indicates the element from which we are seeking a compatible or optimum neighbour.
UIElement * UIElement::GetUpNeighbour(UIElement * referenceElement, bool & searchChildrenOnly)
{
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
		/// If still haven't found a decent one, consult our parent. Unless we're in the stack of course, upon which it means we have found no decent new neighbour.
		if (!element && !searchChildrenOnly && !inStack)
		{
			if (parent)
				element = parent->GetUpNeighbour(referenceElement, searchChildrenOnly);
			/// No parent? Then we probably don't have any good one, search lower down, yo.
			else 
				return NULL;
		}
	}
	/// All-right, so we found a suitable neighbour hopefully. Make sure that it is hoverable?
	if (element && !element->activateable)
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
//			element = element->GetUpNeighbour(NULL, searchChildrenOnly);
		}
	}
	// Nothing found? Then set ourselves to be this sought after element!
	if (!element)
	{
		element = GetElementClosestTo(referenceElement->position, true);
	}
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
	if (rightNeighbour && rightNeighbour->activateable)
	{
		element = rightNeighbour;
	}
	/// If not, fetch it if possible.
	else {
		/// Check if we got a preferred neighbour.
		if (rightNeighbourName.Length())
		{
			element = this->GetRoot()->GetElementByName(rightNeighbourName);
			if (element && !element->activateable)
				element = NULL;
		}
		/// If still haven't found a decent one, consult our parent. Unless we're in the stack of course, upon which it means we have found no decent new neighbour.
		if (!element && !searchChildrenOnly && !inStack)
		{
			if (parent)
				element = parent->GetRightNeighbour(referenceElement, searchChildrenOnly);
			/// No parent? Then we probably don't have any good one, search lower down, yo.
			else 
				return NULL;
		}
	}
	/// All-right, so we found a suitable neighbour hopefully. Make sure that it is hoverable?
	if (element && !element->activateable){
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
		element = GetElementClosestTo(referenceElement->position, true);
	}
	return element;
}

/// Suggests a neighbour which could be to the right of this element. Meant to be used for UI-navigation support. The reference element indicates the element to which we are seeking a compatible or optimum neighbour, and should be NULL for the initial call.
UIElement * UIElement::GetDownNeighbour(UIElement * referenceElement, bool & searchChildrenOnly)
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
		/// If still haven't found a decent one, consult our parent. Unless we're in the stack of course, upon which it means we have found no decent new neighbour.
		if (!element && !searchChildrenOnly && !inStack)
		{
			if (parent)
				element = parent->GetDownNeighbour(referenceElement, searchChildrenOnly);
			/// No parent? Then we probably don't have any good one, search lower down, yo.
			else 
				return NULL;
		}
	}
	/// All-right, so we found a suitable neighbour hopefully. Make sure that it is hoverable?
	if (element && !element->activateable){
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
		element = GetElementClosestTo(referenceElement->position, true);
	}
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
	if (leftNeighbour && leftNeighbour->activateable)
	{
		element = leftNeighbour;
	}
	/// If not, fetch it if possible.
	else {
		/// Check if we got a preferred neighbour.
		if (leftNeighbourName.Length())
		{
			element = this->GetRoot()->GetElementByName(leftNeighbourName);
			if (element && !element->activateable)
				element = NULL;
		}
		/// If still haven't found a decent one, consult our parent. Unless we're in the stack of course, upon which it means we have found no decent new neighbour.
		if (!element && !searchChildrenOnly && !inStack)
		{
			if (parent)
				element = parent->GetLeftNeighbour(referenceElement, searchChildrenOnly);
			/// No parent? Then we probably don't have any good one, search lower down, yo.
			else 
				return NULL;
		}
	}
	/// All-right, so we found a suitable neighbour hopefully. Make sure that it is hoverable?
	if (element && !element->activateable){
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
		element = GetElementClosestTo(referenceElement->position, true);
	}
	return element;
}

/// Works recursively.
UIElement * UIElement::GetElementClosestTo(Vector3f & position, bool mustBeActivatable /*= false*/)
{
	List<UIElement*> allChildren = this->AllChildrenR();
	UIElement * closest = NULL;
	float closestDistance = 1000000.f;
	for (int i = 0; i < allChildren.Size(); ++i)
	{
		UIElement * child = allChildren[i];
		if (mustBeActivatable && !child->activateable)
			continue;
		float dist = (child->position - position).Length();
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
		if (mustBeActivatable && !child->activateable)
			continue;
		float dist = AbsoluteValue(child->position.x - position.x);
		if (dist < closestDistance)
		{
			closest = child;
			closestDistance = dist;
		}
		if (dist == closestDistance)
		{
			// Same x? Check closer y.
			if (AbsoluteValue(child->position.y - position.y) < AbsoluteValue(closest->position.y - position.y))
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
		if (mustBeActivatable && !child->activateable)
			continue;
		float dist = AbsoluteValue(child->position.y - position.y);
		if (dist < closestDistance)
		{
			closest = child;
			closestDistance = dist;
		}
		if (dist == closestDistance)
		{
			// Same y? Check closer x.
			if (AbsoluteValue(child->position.x - position.x) < AbsoluteValue(closest->position.x - position.x))
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
	selected = false;
}

UIElement* UIElement::GetElementByPos(int i_posX, int i_posY){

	UIElement* result = 0;
	// Don't process invisible UIElements, please.
	if (visible == false)
		return 0;
	// Check if the mouse is outside the element's boundaries.
	if (i_posX < posX || i_posX > posX + sizeX ||
		i_posY < posY || i_posY > posY + sizeY){
		// Return false if we are outside of the boundaries,
		// since we haven't found the selected element.
		return 0;
	}

	// Alright, the mouse is inside this element!
	// Do we have children?
	for (int i = 0; i < children.Size(); ++i){
		result = children[i]->GetElementByPos((int)(i_posX - posX), (int)(i_posY - posY));
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
		if (!child->visible)
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
		if (!children[i]->visible)
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
		if (!children[i]->visible)
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
		if (!children[i]->visible)
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
	if (!this->visible)
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
	Vector2i absPos ((int) floor(posX+0.5f), (int) floor(posY+0.5f));
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
    for (int i = 0; i < children.Size(); ++i)
        children[i]->Print(level+1);
}

// Movement functions. These take into consideration the parent element and UIType
void UIElement::MoveX(int distance){
	switch(type){
		case UIType::SLIDER_HANDLE:
			posX += distance;
			if (posX > parent->sizeX - sizeX)
				posX = (int)parent->sizeX - sizeX;
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
				posY = (int)parent->sizeY - sizeY;
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
				posX = (int)parent->sizeX - sizeX;
			if (posX < 0)
				posX = 0;
			break;
		}
		case UIType::SCROLL_HANDLE: {
			posY -= sizeY/2;
			if (posY > parent->sizeY - sizeY)
				posY = (int)parent->sizeY - sizeY;
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
	for (int i = 0; i < children.Size(); ++i){
        UIElement * child = children[i];
        assert(child && "NULL-child? wtf?");
   //     std::cout<<"\nChild name: "<<child->name;
		String childName = children[i]->name;
		result = children[i]->GetElementWithName(i_name);
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
bool UIElement::AddChildren(List<UIElement*> children)
{
	// No options? Just add children.. even tho it will look like crap.
	for (int i = 0; i < children.Size(); ++i)
	{
		AddChild(children[i]);
	}
	return true;
}

// Structurization
bool UIElement::AddChild(UIElement *in_child)
{
    assert(in_child);
    if (in_child == NULL)
        return false;
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

	children.Add(in_child);
	// Set it's parent to this.
	in_child->parent = this;
	in_child->ui = ui;
	return true;
}

/// Attempts to remove said child from this element. Returns false if it was not a valid child. (thus action unnecessary).
bool UIElement::RemoveChild(UIElement * element)
{
	if (children.RemoveItemUnsorted(element))
	{
		element->parent = NULL;
		return true;
	}
	return false;
}


bool UIElement::AddToParent(String parentName, UIElement * child)
{
    UIElement * padre = GetElementWithName(parentName);
    if (padre){
        padre->AddChild(child);
        return true;
    }
	std::cout<<"\nUnable to add element "<<child->name<<" to parent \'"<<parentName<<"\'. Could not find parent within UI.";
    return false;
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
	/// Check for mesh-Entity
	if (mesh){
		if (!name)
			;//std::cout<<"\nBuffering un-named UIElement.";
		else
			;//std::cout<<"\nBuffering UIElement \""<<name<<"\"";

		/// Create VAO

		// Check for errors before we proceed.
		CheckGLError("UIElement::Bufferize");

		if (GL_VERSION_MAJOR >= 3){
		// Generate VAO and bind it straight away if we're above GLEW 3.0
			if (vertexArray == -1)
				vertexArray = GLVertexArrays::New();
			// Binding caused error in wglDeleteContext later for some reason? Worth looking into later maybe.
		//	glBindVertexArray(vertexArray);
		}
		
		// Check for errors before we proceed.
		GLuint err = glGetError();
		if (err != GL_NO_ERROR)
			std::cout<<"\nGLError buffering UI in UIElement::Bufferize";

		// Count total vertices/texture point pairs without any further optimization.
		unsigned int vertexCount = mesh->faces.Size() * 3;
		unsigned int floatsPerVertex = 3 + 3 + 2;  // Pos + Normal + UV
		unsigned int vboVertexDataLength = vertexCount * floatsPerVertex;
		float * vboVertexData = new float[vboVertexDataLength];

		// Generate And bind The Vertex Buffer
		/// Check that the buffer isn't already generated
		if (vboBuffer == -1)
		{
			vboBuffer = GLBuffers::New();
		}
		glBindBuffer(GL_ARRAY_BUFFER, vboBuffer);

		// Load all data in one big fat array, yo Data
		unsigned int vertexDataCounted = 0;
		// Reset vertices count
		vboVertexCount = 0;
		for (int i = 0; i < mesh->faces.Size(); ++i)
		{
			MeshFace * face = &mesh->faces[i];
			// Count vertices in all triangles
			for (int j = 0; j < 3; ++j)
			{
				int currentVertex = face->vertices[j];
				// Position
				vboVertexData[vertexDataCounted + 0] = mesh->vertices[currentVertex][0];
				vboVertexData[vertexDataCounted + 1] = mesh->vertices[currentVertex][1];
				vboVertexData[vertexDataCounted + 2] = mesh->vertices[currentVertex][2];
				// Normal
				int currentNormal = face->normals[j];
				vboVertexData[vertexDataCounted + 3] = mesh->normals[currentNormal][0];
				vboVertexData[vertexDataCounted + 4] = mesh->normals[currentNormal][1];
				vboVertexData[vertexDataCounted + 5] = mesh->normals[currentNormal][2];
				// UV
				int currentUV = face->uvs[j];
				vboVertexData[vertexDataCounted + 6] = mesh->uvs[currentUV][0];
				vboVertexData[vertexDataCounted + 7] = mesh->uvs[currentUV][1];
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
	for (int i = 0; i < children.Size(); ++i){
		children[i]->Bufferize();
	}

	// Bufferize textures straight away if needed too
	if (this->texture && texture->glid == -1)
		TexMan.BufferizeTexture(texture);

	// Mark it as buffered.
	isBuffered = true;
}
/// Releases resources used by the UIElement. Should only be called by a thread with valid GL context!
void UIElement::FreeBuffers()
{
	if (vboBuffer){
		glDeleteBuffers(1, &vboBuffer);
		vboBuffer = -1;
	}
	if (vertexArray){
		glDeleteVertexArrays(1, &vertexArray);
		vertexArray = -1;
	}
	for (int i = 0; i < children.Size(); ++i){
		children[i]->FreeBuffers();
	}
	isBuffered = false;
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
	PrintGLError("GLError before UIElement::Render?!");
	if (!isGeometryCreated)
	{
		AdjustToParent();
		CreateGeometry();
	}
	if (!isBuffered)
	{
		// Re-adjust to parent.
		AdjustToParent();
		ResizeGeometry();
		Bufferize();
	}

    PrintGLError("GLError in UIElement::Render 2");
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
	

	FetchBindAndBufferizeTexture();



	/// Set mip-map filtering to closest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    PrintGLError("GLError Binding texture");

	// Check for valid buffer before rendering,
	// Also check for valid texture...!
	if (vboBuffer && validTexture)
	{
		Shader * shader = ActiveShader();
		if (!shader)
			return;
		UpdateHighlightColor();

		// Set material?	-	Not needed for UI!?
		// Just set a light-parameter to be multiplied to the texture?
		// Nothing for now... TODO

		// Set VBO and render
		// Bind vertices
		graphicsState.BindVertexArrayBuffer(vboBuffer);
		glVertexAttribPointer(shader->attributePosition, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, 0);		// Position
		
		// Bind UVs
		static const GLint offsetU = 6 * sizeof(GLfloat);		// Buffer already bound once at start!
		glVertexAttribPointer(shader->attributeUV, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void *)offsetU);		// UVs
		
        PrintGLError("GLError Binding Buffers");
		int vertices = vboVertexCount;



		// If moveable, translate it to it's proper position!
		if (moveable)
		{
			///
			if (shader->uniformModelMatrix != -1){
				/// TRanslatem power !
				Matrix4f * model = &graphicsState.modelMatrixD;
				float transX = alignmentX * parent->sizeX;
				float transY = alignmentY * parent->sizeY;
				model->Translate(transX,transY,0);
				graphicsState.modelMatrixF = graphicsState.modelMatrixD;
			}
		}

		/// Load in ze model matrix
		glUniformMatrix4fv(shader->uniformModelMatrix, 1, false, graphicsState.modelMatrixF.getPointer());

        PrintGLError("GLError glUniformMatrix in UIElement");
		// Render normally
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES, 0, vboVertexCount);        // Draw All Of The Triangles At Once
		PrintGLError("GLError glDrawArrays in UIElement");

		// Unbind buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		checkGLError();

	}
	RenderText(graphicsState);
}

void UIElement::RenderText(GraphicsState & graphicsState)
{
	/// Bind correct font if applicable.
	if (this->text.Length()){
		if (this->font){
			graphicsState.currentFont = this->font;
		}
		else if (this->fontSource && !this->font){
			this->font = Graphics.GetFont(this->fontSource);
			if (this->font)
				graphicsState.currentFont = this->font;
		}
		// If still no font, use default font.
		if (!font)
		{
			graphicsState.currentFont = Graphics.GetFont(TextFont::defaultFontSource);
		}
	}
	// Render text if applicable!
	if ((this->text.Length() || text.caretPosition > -1) 
		&& graphicsState.currentFont)
	{
	}
	else
		return;

	TextFont * currentFont = graphicsState.currentFont;
	Matrix4d tmp = graphicsState.modelMatrixD;
	graphicsState.modelMatrixD.Translate(this->left + textToRender.offsetX, this->top,(this->zDepth+0.05));
	float pixels = sizeY * textSizeRatio; // Graphics.Height();

    if (currentTextSizeRatio <= 0)
	{
		FormatText();
    }

	pixels *= currentTextSizeRatio; //this->textSizeRatio;
//		std::cout<<"\nTextToRender size in pixels: "<<pixels;
	graphicsState.modelMatrixD.Scale(pixels);	//Graphics.Height()
	graphicsState.modelMatrixF = graphicsState.modelMatrixD;
	// If disabled, dull the color! o.o
	if (this->IsDisabled())
		currentFont->disabled = true;
	else
		currentFont->disabled = false;
	if (this->state & UIState::HOVER && highlightOnHover)
		currentFont->hoveredOver = true;
	else
		currentFont->hoveredOver = false;
//	color[3] *= 0.5f;
//		std::cout<<"\nTextToRender: "<<textToRender;
	graphicsState.currentFont->RenderText(this->textToRender, graphicsState);
	graphicsState.modelMatrixF = graphicsState.modelMatrixD = tmp;
}

void UIElement::FormatText()
{
	/// Resize to fit.
	TextFont * currentFont = GraphicsThreadGraphicsState->currentFont;
	textToRender = text;
	/// Rows available
	int rowsAvailable = 1;
	currentTextSizeRatio = 1.0f;
	/// Returns the size required by a call to RenderText if it were to be done now. In... pixels? or units
	Vector2f size = currentFont->CalculateRenderSizeUnits(text);
	/// Pixels per unit, defined by the relative Y size the text should have.
	float pixels = sizeY * textSizeRatio; // Graphics.Height();
	float pixelsPerUnit = sizeY * textSizeRatio;
	/// Total pixels required if rendering right now.
	Vector2f pixelsRequired = size * pixelsPerUnit;
	/// Check how much X is available, if we need to center it, etc.
	int xAvailable = sizeX - pixelsRequired.x;
	float xRatio = 1.f, yRatio = 1.f;
	if (pixelsRequired.x > rowsAvailable * sizeX){
		// assert(false && "Too much text!");
//				std::cout<<"\nNOTE: Too much text for given space and size, scaling down text to fit!";
		xRatio = sizeX / pixelsRequired.x;
		// Scale it down, yes.
	}
	if (pixelsRequired.y > sizeY)
	{
		// D: Divide moar?
		yRatio = sizeY / pixelsRequired.y;
	}
	if (xRatio < yRatio)
		currentTextSizeRatio = xRatio;
	else
		currentTextSizeRatio = yRatio;

	/// Check previous ratio. Use lower one.
	if (previousTextSizeRatio < currentTextSizeRatio)
		currentTextSizeRatio = previousTextSizeRatio;

	previousTextSizeRatio = currentTextSizeRatio;
	
	/// Re-align.
	if (textAlignment == UIElement::CENTER)
	{
		pixelsPerUnit = currentTextSizeRatio * sizeY;
		pixelsRequired = size * pixelsPerUnit;
		int offsetX = sizeX * 0.5f - pixelsRequired.x * 0.5f;
		if (offsetX < 0)
			offsetX = 0;
		textToRender.offsetX = offsetX;
	}

}

void UIElement::RenderChildren(GraphicsState & graphicsState)
{
	Vector4d initialPositionTopRight(right, top, 0, 1), 
			initialPositionBottomLeft(left, bottom, 0, 1);
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
        if (!child->visible)
            continue;
		// Render
		child->Render(graphicsState);
		// Reset scissor if needed.
		graphicsState.SetGLScissor(uiScissor);
	}
	graphicsState.SetGLScissor(previousScissor);
}


/// Adjusts the UI element size and position relative to new AppWindow size
void UIElement::AdjustToWindow(int w_left, int w_right, int w_bottom, int w_top)
{
    /// Reset text-variables so that they are re-calculated before rendering again.
    currentTextSizeRatio = -1.0f;

	// Extract some attributes before we begin
    left = -1, right = 1, bottom = -1, top = 1;
	sizeX = 1, sizeY = 1;
	float z = 0;
	if (parent){
		left = parent->posX - parent->sizeX/2;
		right = parent->posX + parent->sizeX/2;
		bottom = parent->posY - parent->sizeY/2;
		top = parent->posY + parent->sizeY/2;
	}
	else {
		left = w_left;
		right = w_right;
		bottom = w_bottom;
		top = w_top;
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
		centerX = (float)left;
		centerY = (float)bottom;
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
			left = RoundInt(centerX - sizeX * sizeRatioX / 2);
			right = RoundInt(centerX + sizeX * sizeRatioX / 2);
			bottom = RoundInt(centerY - sizeY * sizeRatioY / 2);
			top = RoundInt(centerY + sizeY * sizeRatioY / 2);
			break;
		}
		case MAXIMIZE:
			if (this->keepRatio){
				//float screenRatio = (w_right - w_left) / (float)(w_top - w_bottom);
				float newRatio = 1;//screenRatio / ratio;
				left = RoundInt(centerX - sizeX * newRatio / 2);
				right = RoundInt(centerX + sizeX * newRatio / 2);
				bottom = RoundInt(centerY - sizeY * newRatio / 2);
				top = RoundInt(centerY + sizeY * newRatio / 2);
			}
			else
				mesh->SetDimensions((float)w_left, (float)w_right, (float)w_bottom, (float)w_top, (float)zDepth);
			break;
		case CENTER:
			/// Do nothing, we start off using regular centering
			left = RoundInt(centerX - sizeX * sizeRatioX / 2);
			right = RoundInt(centerX + sizeX * sizeRatioX / 2);
			bottom = RoundInt(centerY - sizeY * sizeRatioY / 2);
			top = RoundInt(centerY + sizeY * sizeRatioY / 2);
			break;
			/*
		case TOP:
			/// Do nothing, we start off using regular centering
			left = centerX - sizeX * sizeRatioX / 2;
			right = centerX + sizeX * sizeRatioX / 2;
			bottom = centerY - sizeY * sizeRatioY;
			top = centerY;
			break;
			*/
		default:
			// Make half-size
			std::cout<<"UIElement "<<this->name<<" gets default half-size.";
			mesh->SetDimensions((left*3.f + right)/4.f, (left + right*3.f/4.f), (bottom * 3.f+ top)/4.f, (bottom + top*3.f)/4.f, zDepth+1.f);
			break;
	}

	/// Set the new dimensions
	if (mesh)
		mesh->SetDimensions((float)left, (float)right, (float)bottom, (float)top, z);

	// Save away the new sizes
	sizeX = (int) (right - left);
	sizeY = (int) (top - bottom);
	posX = sizeX/2 + left;
	posY = sizeY/2 + bottom;
	position = Vector3f((float)posX, (float)posY, zDepth);
	// Correct the position for the movable objects! ^^
	if (moveable)
	{
		posX += RoundInt(parent->sizeX * alignmentX);
		posY += RoundInt(parent->sizeY * alignmentY);
		left = RoundInt(posX - sizeX / 2.0f);
		right = RoundInt(posX + sizeX / 2.0f);
		top = RoundInt(posY + sizeY / 2.0f);
		bottom = RoundInt(posY - sizeY / 2.0f);
	}

	/// Adjust all children too
	for (int i = 0; i < children.Size(); ++i){
		assert(children[i] != this);
		children[i]->AdjustToWindow((int)left, (int)right, (int)bottom,(int)top);
	}
}

/// Calls AdjustToWindow for parent's bounds. Will assert if no parent is available.
void UIElement::AdjustToParent()
{
	if (!parent)
		return;
	AdjustToWindow(parent->left, parent->right, parent->bottom, parent->top);
	previousTextSizeRatio = 1.f; // Reset to allow larger text if we are using the minimum criteria for rapdily changing text-slots ( such as distance).
}

int UIElement::GetAlignment(String byName)
{
	byName.SetComparisonMode(String::NOT_CASE_SENSITIVE);
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
	if(!children.Size()){
		std::cout<<"\nSlider "<<this->name<<" lacking child (sliderHandle)!";
		return;
	}
	children[0]->alignmentX = level;
	return;
}
/// Returns the slider's level
float UISlider::GetLevel(){
	if (!children.Size()){
		std::cout<<"\nERROR: Unable to get slider level as it has no child (handle)!";
		return 0;
	}
	return children[0]->alignmentX;
}

// Creates the Square mesh used for rendering the UIElement and calls SetDimensions with it's given values.
void UIElement::CreateGeometry()
{
	Square * sq = new Square();
//	std::cout<<"\nResizing geometry "<<name<<": L"<<left<<" R"<<right<<" B"<<bottom<<" T"<<top<<" Z"<<this->zDepth;
	sq->SetDimensions((float)left, (float)right, (float)bottom, (float)top, this->zDepth);
	this->mesh = sq;
	for (int i = 0; i < children.Size(); ++i){
		children[i]->CreateGeometry();
	}
	isGeometryCreated = true;
}
void UIElement::ResizeGeometry()
{
	if (!isGeometryCreated)
		CreateGeometry();
	assert(mesh);
//    std::cout<<"\nResizing geometry: L"<<left<<" R"<<right<<" B"<<bottom<<" T"<<top<<" Z"<<this->zDepth;
	this->mesh->SetDimensions((float)left, (float)right, (float)bottom, (float)top, this->zDepth);
	for (int i = 0; i < children.Size(); ++i){
		children[i]->ResizeGeometry();
	}
	// Mark as not buffered to refresh it properly
	isBuffered = false;
	previousTextSizeRatio = 1.f; // Reset to allow larger text if we are using the minimum criteria for rapdily changing text-slots ( such as distance).
}
void UIElement::DeleteGeometry()
{
//	assert(mesh);
	if (mesh == NULL)
		return;
	delete mesh;
	mesh = NULL;
	for (int i = 0; i < children.Size(); ++i){
		children[i]->DeleteGeometry();
	}
}

/// For example UIState::HOVER, not to be confused with flags! State = current, Flags = possibilities
bool UIElement::AddState(int i_state)
{
	// Return if trying to add invalid state.
	if (!hoverable && i_state & UIState::HOVER)
		return false;
	if (i_state == UIState::HOVER)
	{
		if (inputState->demandActivatableForHoverElements && !activateable)
			return false;
		if (inputState->demandHighlightOnHoverForHoverElements && !highlightOnHover)
			return false;
	}
	state |= i_state;
	return true;
}

/// For example UIState::HOVER, if recursive will apply to all children.
void UIElement::RemoveState(int statesToRemove, bool recursive /* = false*/){
	state &= ~statesToRemove;
	if (recursive){
		for (int i = 0; i < children.Size(); ++i){
			children[i]->RemoveState(statesToRemove, recursive);
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
	for (int i = 0; i < children.Size(); ++i){
		children[i]->RemoveFlags(flags);
	}
}

UILabel::UILabel(String name /*= ""*/)
: UIElement()
{
	this->name = name;
	text = name;
	type = UIType::LABEL;
	hoverable = true;
	highlightOnHover = false;
	selectable = activateable = false;
	/// Set text-color at least for labels!
	text.color = defaultTextColor;
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
int UIElement::OnKeyDown(int keyCode, bool downBefore)
{
	if (parent)
		parent->OnKeyDown(keyCode, downBefore);
	return 0;
}
/// Used for getting text. This will be local translated language key codes?
int UIElement::OnChar(int asciiCode){
	assert(false);
	return 0;
}


/// Called to ensure visibility of target element. First call should be made to the target element with a NULL-argument!
void UIElement::EnsureVisibility(UIElement * element)
{
	// If this is a "system" element, i.e. scroll-bars that are automatically added, skip this step, as they should always be visible.
	if (isSysElement)
		return;
	if (!element)
		element = this;
	if (parent)
		parent->EnsureVisibility(this);
}
