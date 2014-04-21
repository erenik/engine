#include "UserInterface.h"
#include "../Globals.h"

#include "../StateManager.h"
#include "../TextureManager.h"
#include "Message/MessageManager.h"
#include "Message/Message.h"
#include "Mesh/Square.h"
#include "../GraphicsState.h"
#include <iomanip>
#include "UIList.h"
#include "UIInput.h"
#include "UIButtons.h"
#include "UIVideo.h"
#include "Graphics/Fonts/Font.h"
#include "UI/UITypes.h"
#include "UI/DataUI/UIMatrix.h"

bool UserInterface::printDebug = true;

UserInterface::UserInterface(){
	this->root = NULL;
	isGeometryCreated = false;
	isBuffered = false;
	hoverElement = NULL;
	demandHovered = true;
	defaultTextColor = Vector4f(1.f,1.f,1.f,1.f);
	defaultTextureSource = "80Gray50Alpha.png";
}

/** Mouse interactions with the UI, in x and y from 0.0 to 1.0, with 0.0 being in the Upper Left corner. Returns the element currently hovered over.
	If allUi is specified the current stack order will be ignored to a certain extent, meaning that ui below the current stack top will be made available too.
	Search is conducted starting at the highest stack element and going down until an element is found that we can hover over.
*/
UIElement * UserInterface::Hover(float x, float y, bool allUi){
	UIElement * stackTop = GetStackTop();
	if (!stackTop)
		return NULL;
	UIElement * result = NULL;
	/// Search taking into consideration the stack but searching until an element is found.
	if(allUi)
	{
		for (int i = stack.Size()-1; i >= 0; --i)
		{
			UIElement * stackElement = stack[i];
			/// Remove the hover flag before re-hovering.
			stackElement->RemoveFlags(UIState::HOVER);
			result = stackElement->Hover(x,y);
			if (result)
				break;
		}
		/// If still no result, try the root.
		if (!result)
		{
			root->RemoveFlags(UIState::HOVER);
			result = root->Hover(x,y);
		}
		/// Demand hover will have to be investigated how it could work in this mode, if at all.
		hoverElement = result;	
	}
	/// Old search using only the stack-top.
	else {
		UIElement * previous = stackTop->GetElementByState(UIState::HOVER);
		/// Remove the hover flag before re-hovering.
		stackTop->RemoveFlags(UIState::HOVER);
		result = stackTop->Hover(x,y);
		hoverElement = result;
		/// If we always want a hovered element (for whatever reason).
		if (!hoverElement && demandHovered && previous){
			hoverElement = stackTop->Hover(previous->posX, previous->posY);
		}
	}
	return result;
}
/// If allUi is specified, the action will try, similar to hover, and go through the entire stack from the top down until it processes an element.
UIElement * UserInterface::Click(float x, float y, bool allUi){
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
	result = stackTop->Activate();
	if (result){
		if (result->onActivate != NULL){
			result->onActivate(result);
			return result;
		}
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

/** Function called when an element is activated,
	by mouse-clicking, pressing enter on selction or otherwise. */
void UserInterface::Activate(UIElement* activeElement){
	std::cout<<"\nUserInterface::Activate called";
	if (activeElement->onActivate != NULL){
		std::cout<<"\nActive element: "<<activeElement;
		std::cout<<" "<<activeElement->name;
		return;
		activeElement->onActivate(activeElement);
	}
	if (activeElement->activationMessage.Length() == 0){
		std::cout<<"Activatable UI element has no valid activation message string!";
		return;
	}
	if (activeElement->activationMessage.Length() != 0){
		List<String> msgs = activeElement->activationMessage.Tokenize("&");
		for (int i = 0; i < msgs.Size(); ++i){
			Message * message = new Message(msgs[i]);
			message->element = activeElement;
			MesMan.QueueMessage(message);
		}
	}
	else {
		std::cout<<"\nonActivate and activationMessage both NULL in element: "<<activeElement->name;
	}
}

/// Get element by position. Will skip invisible elements.
UIElement * UserInterface::GetElement(float x, float y){
    if (!root)
		return NULL;
    return root->GetElement(x,y);
}

/// Woo!
UIElement * UserInterface::GetElement(String byName, int andType)
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
void UserInterface::ResizeGeometry(){
	assert(isGeometryCreated);
	root->ResizeGeometry();
}
void UserInterface::DeleteGeometry(){
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
bool UserInterface::AdjustToWindow(int i_width, int i_height){
	// Check if we need to do anything at all first!
	if (width == i_width && height == i_height)
		return false;
	width = i_width; height = i_height;
	root->AdjustToWindow(0, width, 0, height);
	return true;
}

/// Creates/updates VBOs for all UI elements.
void UserInterface::Bufferize(){

	assert(!isBuffered);
	assert(this->isGeometryCreated);
	root->Bufferize();
	isBuffered = true;
	std::cout<<"\nUI "<<this->source<<" bufferized.";
}

/// Releases GL resources
bool UserInterface::Unbufferize(){
	assert(isBuffered && "Unbuffering something that isn't buffered, are we?");
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
void UserInterface::Render(GraphicsState& graphics){
	/// Disable depth-test.
	glDisable(GL_DEPTH_TEST);
	List<UIElement*> list;
	root->GetElementsByState(UIState::HOVER, list);
	//std::cout<<"\nHoverElements: "<<list.Size();
	/// Render the tree.
	root->Render(graphics);
}

/// Prints the UI's tree recursively. The level parameter is in order to display the tree properly.
void UserInterface::Print(int level/* = -1*/){
    root->Print(level);
}

/// lol?
UIElement * UserInterface::GetElementById(int ID){
	if (root)
		return root->GetElementWithID(ID);
	return NULL;
}

/// Returns a  pointer to element with specified name. Returns NULL if it cannot be found.
UIElement * UserInterface::GetElementByName(const char * name){
	if (name == NULL)
		return NULL;
	if (root)
		return root->GetElementWithName(name);
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
UIElement * UserInterface::GetActiveElement(){
	if (root)
		return root->GetActiveElement();
	return NULL;
}

/// Returns active input focus element, if any. Based on the GetActiveElement but performs additional checks.
UIElement * UserInterface::ActiveInputFocusElement(){
	UIElement * e = GetActiveElement();
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

UIElement * UserInterface::GetHoverElement(){
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

UserInterface::~UserInterface(){
	assert(!isBuffered);
	assert(!isGeometryCreated);
	if (root != NULL)
		delete root;
	root = NULL;
}

#include <fstream>

/// Directory for the UI relative to root (bin/)
String UserInterface::rootUIDir; //  = "gui/";

/** Attempts to load the UI from file. Returns false if it failed.
	If any elements exist before loading they will be deleted first.
*/
bool UserInterface::Load(String fromFile){
    if (root)
		delete root;
	/// Create the root node.
	this->root = new UIElement();
	root->exitable = false;
	root->name = "root";
	root->selectable = false;
	root->activateable = false;

	uiName = fromFile;
	/// Load into root.
	bool success = LoadFromFile(fromFile, root);
	if (success){
		source = fromFile;
	}
	return success;
}


/** Loads target UI from file, storing it as a single UIElement so that it may be inserted into any other UI of choice.
	Caller is responsible for inserting it into a proper UI or deallocation if it fails.
*/
UIElement * UserInterface::LoadUIAsElement(String uiSrcLocation){
	UIElement * newElement = new UIElement();
	bool success = LoadFromFile(uiSrcLocation, newElement);
	/// Check that it worked as intended.
	if (!success)
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
int UserInterface::PushToStack(UIElement * element){
	if (!element)
			return NULL_ELEMENT;
	if (InStack(element))
		return ALREADY_IN_STACK;
	/// This adds at the end, yes?
	stack.Add(element);
	element->visible = true;
	return PUSHED_TO_STACK;
}
void UserInterface::PopFromStack(String elementName){
	PopFromStack(GetElementByName(elementName));
}
bool UserInterface::PopFromStack(UIElement * element, bool force){
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
	bool result = stack.Remove(element, ListOption::RETAIN_ORDER) != NULL;
	assert(!stack.Exists(element));
	/// Make it invisible in any case?
	element->visible = false;
	/// Call on exit scope for it!
	element->OnExitScope();
	return result;
}

/// Pops the top element, returning a pointer to it.
UIElement * UserInterface::PopFromStack(){
	if (!stack.Size())
		return NULL;
	UIElement * last = stack.Last();
	stack.Remove(last, ListOption::RETAIN_ORDER);
	last->visible = false;
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
UIElement * UserInterface::GetStackTop(){
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
		root->OnExitScope();
}

/// Loads from target file, using given root as root-element in the UI-hierarchy.
bool UserInterface::LoadFromFile(String filePath, UIElement * root)
{
	String fromFile = filePath;
	if (!fromFile.Contains(rootUIDir)){
		fromFile = rootUIDir + fromFile;
	}

	char * data;
	int fileSize;
	std::fstream file;
	file.open(fromFile.c_str(), std::ios_base::in);

	//	assert(file.is_open() && "Unable to open file in GameState::LoadUI");
	if (!file.is_open()){
		std::cout<<"\nUserInterface::LoadFromFile: Unable to open file: "<<fromFile;
		return false;
	}
	root->source = filePath;

	// Get size by seeking to end of file
	int start  = (int) file.tellg();
	file.seekg( 0, std::ios::end );
	fileSize = (int) file.tellg();

	// Allocate data array to required length
	data = new char [fileSize+5];
	memset(data, 0, fileSize+5);

	// Go to beginning of file and read the data
	file.seekg( 0, std::ios::beg);
	file.read((char*) data, fileSize);
	// Close file stream
	file.close();

	assert(!file.bad());

    std::cout<<"\n=====================================";
    std::cout<<"\nBeginning parsing file "<<fromFile;

    // Dump data into handable format.
	String contents;
    contents = data;

	// Delete data
	delete[] data;
	data = NULL;

	List<String> lines = contents.GetLines();

    for (int i = 0; i < lines.Size(); ++i) {
      ;//  std::cout<<"\nLine "<<i<<lines[i];
    }

	String str;
	UIElement * element = NULL;
	enum parsingState {
		NULL_STATE,
		MID_COMMENT,	 // For /* */
	};
	int parsingState = NULL_STATE;

	/// Default values that can be set when parsing
	int defaultAlignment = UIElement::NULL_ALIGNMENT;
	String defaultTexture = "default.png";
	String defaultParent = "root";
	String defaultRootFolder = "";
	bool defaultScalability = true;
	bool defaultVisibility = true;
	Vector4f defaultTextColor = Vector4f(0,0,0,1);
	float defaultSizeRatioY	= 1.0f;
	float defaultSizeRatioX	= 1.0f;
	float defaultPadding = 0.0f;
	float defaultTextSize = 1.0f;
	String defaultOnTrigger = "";

#define ENSURE_NEXT_TOKEN if(tokens.Size() < 2){ assert(false && "argument token missing"); continue; };
#define NEXT_TOKEN	(tokens[1])
#define SET_DEFAULTS {element->alignment = defaultAlignment;\
	element->textureSource = defaultTexture;\
	element->scalable = defaultScalability;\
	element->textColor = defaultTextColor;\
	element->sizeRatioY	= defaultSizeRatioY;\
	element->sizeRatioX	= defaultSizeRatioX;\
	element->padding = defaultPadding;\
	element->textSizeRatio = defaultTextSize;\
	element->onTrigger = defaultOnTrigger;\
	element->fontSource = TextFont::defaultFontSource;\
	element->visible = defaultVisibility;\
	}
#define ADD_PREVIOUS_TO_UI_IF_NEEDED {\
	if (element && element != root)\
    root->AddToParent(defaultParent, element);\
	element = NULL;\
	}

    /// Read until done or too many errors!
	bool wasLastLine = false;
	std::cout<<"\nLines to parse: "<<lines.Size();
	for (int i = 0; i < lines.Size(); ++i){

		String line = lines[i];
     //   return true;

        if (i == 59){
            std::cout<<"Shouga die gooha.";
        }
	//	str = line;

        if (line.Length() < 1)
            continue;

		/// Manually parse the line using a few identifiers that can be relevant.
		List<String> tokens;
		int lastEvaluatedIndex = 0;
		List<char> stack;
		char last;
		char cChar;
		for (int l = 0; l < line.Length(); ++l){
			cChar = line.At(l);
	//		std::cout<<"\nChar at "<<l<<": int("<<(int)cChar<<") char: "<<cChar;
			switch(cChar)
			{
				// If not in a current stack, save as a separate word.
				case ' ':
				case '\t':
				case '\n':
				case '\r':
				case '\f':
					if (!stack.Size()){
						// Add it.
						String t;
						for (int j = lastEvaluatedIndex; j < l; j++){
							t += line.At(j);
						}
						t.RemoveInitialWhitespaces();
						if (t.Length())
							tokens.Add(t);
						lastEvaluatedIndex = l;
					}
					break;
				case '(':
					stack.Add(cChar);
					break;
				case ')':
					last = stack.Last();
					assert(last == '(');
					stack.RemoveIndex(stack.Size()-1);
					break;
				default:
					;
			}
		}
		// Add final word as needed.
		String tok;
		for (int j = lastEvaluatedIndex; j < line.Length(); j++){
			tok += line.At(j);
		}
		tok.RemoveInitialWhitespaces();
		if (tok.Length())
			tokens.Add(tok);

		List<String> strings = line.Tokenize("\"");
		String firstQuote, secondQuote, thirdQuote;
		if (strings.Size() >= 2)
			firstQuote = strings[1];
		else if (tokens.Size() >= 2)
			firstQuote = tokens[1];

		if (strings.Size() >= 3)
			secondQuote = strings[2];
		else if (tokens.Size() >= 3)
			secondQuote = tokens[2];

		if (strings.Size() >= 4)
			thirdQuote = strings[3];
		if (tokens.Size() >= 4)
			thirdQuote = tokens[3];

		// Print em for debug
/*		std::cout<<"\n";
		for (int t = 0; t < tokens.Size(); ++t)
			std::cout<<"\nToken "<<t<<": "<<tokens[t] <<" ";
*/

		// Old one using a regular tokenizer.
	//	List<String> tokens = line.Tokenize(" \n\r\t\v\f");
		if (tokens.Size() < 1)
			continue;
/*
		/// If we've got quotation marks on the line, try and parse them straight away into the second token.
		if (line.Contains("\"")){
			List<String> tokens2 = line.Tokenize("\"");
			tokens[1] = tokens2[1];
		}
*/
        if (printDebug){
//            std::cout<<"\nLine "<<std::setw(3)<<i<<": "<<line;;
  //          std::cout<<"\n\tTokens: "<<tokens.Size();
        }

		for (int t = 0; t < tokens.Size(); ++t){
			String token = tokens[t];
			token.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	//		for (int i = 0; i < token.Length(); ++i)
      //          std::cout<<"\n"<<i<<": (int)"<<(int)token.c_str()[i]<<" (char)"<<token.c_str()[i];
            if (token.Contains("\r")){
          //      std::cout<<"Token '\\r'! Skipping o-o";
                continue;
            }
			// Evaluate some things first depending on the current parsing state
			else if (parsingState == MID_COMMENT){
				if (token.Contains("*/")){
					parsingState = NULL_STATE;
					continue;
				}
				continue;
			}
			// Regular parses
			else if (token.Contains("//")){
				// Skip the rest of the line
				// Done by default at the end of these if-elseif-clauses
				break;
			}
			else if (token.Contains("/*")){
				parsingState = MID_COMMENT;
				continue;
			}
			else if (token == "defaultAlignment"){
				ENSURE_NEXT_TOKEN
				defaultAlignment = UIElement::GetAlignment(NEXT_TOKEN);
			}
			else if (token == "defaultTexture"){
				ENSURE_NEXT_TOKEN
				String param = tokens[1];
				param.SetComparisonMode(String::NOT_CASE_SENSITIVE);
                if (param == "NULL")
                    defaultTexture = String();
                else
                    defaultTexture = param;
			}
			else if (token == "defaultOnTrigger"){
				ENSURE_NEXT_TOKEN
				defaultOnTrigger = NEXT_TOKEN;
			}
			else if (token == "defaultParent" ||
				token == "parent")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED
				ENSURE_NEXT_TOKEN
				defaultParent = NEXT_TOKEN;
			}
			else if (token == "defaultScalability"){
				ENSURE_NEXT_TOKEN
				defaultScalability = NEXT_TOKEN.ParseBool();
			}
			else if (token == "defaultVisibility"){
				ENSURE_NEXT_TOKEN
				defaultVisibility = NEXT_TOKEN.ParseBool();
			}
			else if (token == "defaultSizeRatioXY" ||
				token == "defaultSizeRatio" ||
				token == "defaultSizeXY" ||
				token == "defaultSize")
			{
				if (tokens.Size() == 2){
					defaultSizeRatioX = defaultSizeRatioY = tokens[1].ParseFloat();
				}
				else if (tokens.Size() >= 3){
					defaultSizeRatioX = tokens[1].ParseFloat();
					defaultSizeRatioY = tokens[2].ParseFloat();
				}
			}
			else if (token == "defaultSizeRatioY"){
				ENSURE_NEXT_TOKEN
				defaultSizeRatioY = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "defaultSizeRatioX"){
				ENSURE_NEXT_TOKEN
				defaultSizeRatioX = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "defaultPadding"){
				ENSURE_NEXT_TOKEN
				defaultPadding = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "defaultTextSize"){
				ENSURE_NEXT_TOKEN
				defaultTextSize = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "defaultTextColor"){
				switch(tokens.Size()-1){
					case 1: // Assume it's alpha and keep the other colors as usual
						defaultTextColor.w = NEXT_TOKEN.ParseFloat();
						break;
					case 4:
						defaultTextColor.w = tokens[4].ParseFloat();
					case 3: // Assume it's RGB
						defaultTextColor.x = tokens[1].ParseFloat();
						defaultTextColor.y = tokens[2].ParseFloat();
						defaultTextColor.z = tokens[3].ParseFloat();
						break;
					case 2: case 0:
						assert(false && "Irregular amount of tokens following \"defaultTextColor\"; 1 for alpha, 3 for RGB and 4 for RGBA.");
						break;
				}
			}
			else if (token == "defaultRootFolder"){
				ENSURE_NEXT_TOKEN
				defaultRootFolder = NEXT_TOKEN + "/";
				if (NEXT_TOKEN == "NULL")
                    defaultRootFolder = "";
			}
			else if (token == "root"){
				element = root;
			}
			else if (token == "element" || token == "div"){
				ADD_PREVIOUS_TO_UI_IF_NEEDED
				element = new UIElement();
				if (tokens.Size() > 1)
					element->name = firstQuote;
				SET_DEFAULTS
			}
			else if (token == "Button"){
				ADD_PREVIOUS_TO_UI_IF_NEEDED
				element = new UIButton();
				if (tokens.Size() > 1){
					element->name = firstQuote;
					/// Set the elements text and message default to it's name too, yo.
					element->activationMessage = element->text = element->name;
				}
				SET_DEFAULTS
			}
			else if (token == "Label"){
				ADD_PREVIOUS_TO_UI_IF_NEEDED
				element = new UILabel();
				if (tokens.Size() > 1)
					element->text = element->name = firstQuote;
				SET_DEFAULTS
			}
			else if (token == "TextField" || token == "Input"){
				ADD_PREVIOUS_TO_UI_IF_NEEDED
				element = new UITextField();
				SET_DEFAULTS
				if (tokens.Size() > 1)
					element->name = element->onTrigger = firstQuote;
			}
			else if (token == "List"){
				ADD_PREVIOUS_TO_UI_IF_NEEDED
				element = new UIList();
				if (tokens.Size() > 1)
					element->name = firstQuote;
				SET_DEFAULTS
			}
			else if (token == "ColumnList"){
				ADD_PREVIOUS_TO_UI_IF_NEEDED
					element = new UIColumnList();
				if (tokens.Size() > 1)
					element->name = firstQuote;
				SET_DEFAULTS
			}
			else if (token == "checkbox"){
				ADD_PREVIOUS_TO_UI_IF_NEEDED
				element = new UICheckBox();
				if (tokens.Size() > 1){
					element->name = firstQuote;
					element->activationMessage = element->text = element->name;
				}
				SET_DEFAULTS
			}
			else if (token == "Matrix")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				UIMatrix * matrix = new UIMatrix(firstQuote);
				element = matrix;
				SET_DEFAULTS;
				matrix->CreateChildren();
			}
			else if (token == "TextureInput")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				UITextureInput * ti = new UITextureInput(firstQuote, "Set"+firstQuote);
				element = ti;
				SET_DEFAULTS;
				ti->CreateChildren();
			}
			else if (token == "StringInput")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED;
				UIStringInput * si = new UIStringInput(firstQuote, "Set"+firstQuote);
				element = si;
				SET_DEFAULTS;
				si->CreateChildren();
			}
			else if (token == "IntegerInput" ||
				token == "IntInput")
			{
				ADD_PREVIOUS_TO_UI_IF_NEEDED
				UIIntegerInput * ii = new UIIntegerInput(firstQuote, "Set"+firstQuote);
				element = ii;
				SET_DEFAULTS
				ii->CreateChildren();
			}
			else if (token == "FloatInput"){
				ADD_PREVIOUS_TO_UI_IF_NEEDED
				UIFloatInput * fi = new UIFloatInput(firstQuote, "Set"+firstQuote);
				element = fi;
				SET_DEFAULTS
				fi->CreateChildren();
			}
			else if (token == "VectorInput"){
				ADD_PREVIOUS_TO_UI_IF_NEEDED
				String action = "Set"+secondQuote;
				UIVectorInput * vi = new UIVectorInput(firstQuote.ParseInt(), secondQuote, action);
				element = vi;
				SET_DEFAULTS
				vi->CreateChildren();
			}
			else if (token == "Video"){
				ADD_PREVIOUS_TO_UI_IF_NEEDED
				UIVideo * video = new UIVideo(firstQuote, secondQuote);
				element = video;
				SET_DEFAULTS
				video->CreateStream();
			}
			/// Single expressions that apply effects to an element
			else if (token == "navigateUIOnPush"){
				element->navigateUIOnPush = true;
			}
			else if (token.Length() <= 2)
                break;
			else if (tokens.Size() < 2){
				std::cout<<"\nINFO: Lacking argument on line "<<i<<": "<<line;
				std::cout<<"\nSkipping row and going to next one!";
				break;
			}
			else if (element == NULL){
			    String previousLine = lines[i-1];
                std::cout<<"\nTrying to act upon a null-element! Continuing until a new element is created!";
                break;
			}
			else if (token == "Name"){
				ENSURE_NEXT_TOKEN
				element->name = firstQuote;
			}
			else if (token == "Padding"){
				ENSURE_NEXT_TOKEN
				element->padding = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "OnActivate"){
				ENSURE_NEXT_TOKEN
				element->activationMessage = NEXT_TOKEN;
			}
			else if (token == "onTrigger"){
				ENSURE_NEXT_TOKEN
				element->onTrigger = NEXT_TOKEN;
			}
			else if (token == "onExit"){
				ENSURE_NEXT_TOKEN
				element->onExit = NEXT_TOKEN;
			}
			else if (token == "exitable"){
				ENSURE_NEXT_TOKEN
				element->exitable = (NEXT_TOKEN).ParseBool();
			}
			else if (token == "visible"){
				ENSURE_NEXT_TOKEN
				element->visible = (NEXT_TOKEN).ParseBool();
			}
			else if (token == "maxDecimals"){
				ENSURE_NEXT_TOKEN;
				switch(element->type)
				{
					case UIType::FLOAT_INPUT:
					{
						UIFloatInput * fi = (UIFloatInput *)element;
						fi->maxDecimals = NEXT_TOKEN.ParseFloat();
						break;
					}
				}
			}
			else if (token == "texture"){
				ENSURE_NEXT_TOKEN
				String param = tokens[1];
				param.SetComparisonMode(String::NOT_CASE_SENSITIVE);
                if (param == "NULL")
                    element->textureSource = String();
                else
                    element->textureSource = defaultRootFolder + NEXT_TOKEN;
			}
			else if (token == "text"){
				ENSURE_NEXT_TOKEN
				String text = tokens[1];
		/*		std::cout<<"\nTex: "<<line;
				for (int i = 0; i < line.Length(); ++i){
                    std::cout<<"\nc: (int)"<<(int)line.c_str()[i]<<" char: "<<(char)line.c_str()[i];
				} */
        //        std::cout<<"\nLine: "<<line;

                List<String> tmp = line.Tokenize("\"");
				text = tmp[1];
		        /// Check special cases like dedicated label child elements.
				if (element->label){
					element->label->text = text;
					break;
				}
				element->text = text;
			}
			else if (token == "textColor"){
				switch(tokens.Size()-1){
					case 1: // Assume it's alpha and keep the other colors as usual
						element->textColor.w = NEXT_TOKEN.ParseFloat();
						break;
					case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
						element->textColor.w = tokens[4].ParseFloat();
					case 3: // Assume it's RGB
						element->textColor.x = tokens[1].ParseFloat();
						element->textColor.y = tokens[2].ParseFloat();
						element->textColor.z = tokens[3].ParseFloat();
						break;
					case 2: case 0:
						assert(false && "Irregular amount of tokens following \"textColor\"; 1 for alpha, 3 for RGB and 4 for RGBA.");
						break;
				}
			}
			else if (token == "textSizeRatio" || token == "textSize"){
				ENSURE_NEXT_TOKEN
				element->textSizeRatio = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "origin"){
				ENSURE_NEXT_TOKEN
				element->origin = NEXT_TOKEN.ParseInt();
			}
			else if (token == "scalable"){
				ENSURE_NEXT_TOKEN
				element->scalable = NEXT_TOKEN.ParseBool();
			}
			else if (token == "formatX"){
                ENSURE_NEXT_TOKEN
                element->formatX = NEXT_TOKEN.ParseBool();
			}
			else if (token == "sizeRatioX"){
				ENSURE_NEXT_TOKEN
				element->sizeRatioX = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "sizeRatioY"){
				ENSURE_NEXT_TOKEN
				element->sizeRatioY = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "sizeRatio"){
				ENSURE_NEXT_TOKEN
				element->sizeRatioX = element->sizeRatioY = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "sizeRatioXY"){
				if (tokens.Size() < 3)
					continue;
				element->sizeRatioX = tokens[1].ParseFloat();
				element->sizeRatioY = tokens[2].ParseFloat();
			}
			else if (token == "alignmentX"){
				ENSURE_NEXT_TOKEN
				element->alignmentX = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "alignmentY"){
				ENSURE_NEXT_TOKEN
				element->alignmentY = NEXT_TOKEN.ParseFloat();
			}
			else if (token == "alignmentXY"){
				if (tokens.Size() < 3)
					continue;
				element->alignmentX = tokens[1].ParseFloat();
				element->alignmentY = tokens[2].ParseFloat();
			}
			else if (token == "rightNeighbour"){
                element->rightNeighbourName = NEXT_TOKEN;
			}

			else if (token == "leftNeighbour"){
                element->leftNeighbourName = NEXT_TOKEN;
			}
			else if (token == "topNeighbour" || token == "upNeighbour"){
                element->upNeighbourName = NEXT_TOKEN;
			}
			else if (token == "bottomNeighbour"  || token == "downNeighbour"){
                element->downNeighbourName = NEXT_TOKEN;
			}
			else if (token == "AddTo"){
				if (tokens.Size() > 1){
				    String parentName = NEXT_TOKEN;
					UIElement * e = NULL;
					if (parentName == "root")
						e = root;
					else
						e = root->GetElementWithName(parentName);
					if (e == NULL){
                        std::cout<<"\nUndefined parent element "<<tokens[1]<<" for element "<<element->name<<"! Make sure you define it before you add children to it! o-o";
                        for (int c = 0; c < parentName.Length(); ++c)
                        std::cout<<"\nc "<<std::setw(2)<<" (int)"<<(int)parentName.c_str()[c]<<" (char)"<<parentName.c_str()[c];
						break;
					}
                    std::cout<<"\nAdding element "<<element->name<<" as child to "<<e->name;
                    int childrenPre = e->childList.Size();
					e->AddChild(element);
                    int children = e->childList.Size();
                    assert(children > childrenPre);
				}
				else {
					assert(element && "Element NULL! No element has been defined or it was already added! o-o");
					if (element == NULL)
						break;
					root->AddChild(element);
				}
				element = NULL;
			}
			else {
			//	assert(false && "Unknown token in UserInterface::Load(fromFile)");
				std::cout<<"\nUnknown token in UserInterface::Load(fromFile): "<<token;

			}
			// By default proceed with next row straight away
			t = tokens.Size();
		}
	}
	ADD_PREVIOUS_TO_UI_IF_NEEDED
	return true;
}


/// Removes references to target element
void UserInterface::OnElementDeleted(UIElement * element)
{
	stack.Remove(element);
}
