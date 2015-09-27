// Emil Hedemalm
// 2013-07-14

#include "InputState.h"
#include "UI/UI.h"
#include "GMUI.h"
#include "../../UI/UserInterface.h"
#include "Graphics/GraphicsManager.h"
#include "Viewport.h"
#include "OS/Sleep.h"

#include "UI/UITypes.h"
#include "UI/UIList.h"
#include "UI/UIInputs.h"
#include "UI/DataUI/UIMatrix.h"
#include "UI/Buttons/UIRadioButtons.h"

#include "Input/InputManager.h"
#include "StateManager.h"
#include "Message/MessageManager.h"
#include "Window/AppWindowManager.h"

#include "File/LogFile.h"

/// Default constructor, will target active global UI.
GMUI::GMUI(int messageType)
: GraphicsMessage(messageType)
{
	Nullify();
}

/// Default constructor, specifies the viewport to be the default global one.
GMUI::GMUI(int messageType, Viewport * viewport /* = NULL*/)
: GraphicsMessage(messageType)
{
	Nullify();
	this->viewport = viewport;
}

/// Default constructor, if viewPortTarget is NULL it will target the active global UI.
GMUI::GMUI(int messageType, UserInterface * targetUI)
: GraphicsMessage(messageType)
{
	Nullify();
	this->ui = targetUI;
}

void GMUI::Nullify()
{
	ui = NULL;
	viewport = NULL;
	window = NULL;
	global = false;
}


bool GMUI::GetUI()
{
	if (ui)
	{
		// Ensure the UI is good.
		return UserInterface::IsGood(ui);
	}
	if (!window)
	{
		window = WindowMan.MainWindow();
	}
	if (window)
	{
		if (global)
			ui = window->GetGlobalUI();
		else
			ui = window->GetUI();
	}
    if (ui == NULL){
        std::cout<<"\nWARNING: No valid UI available for use.";
        return false;
    }
	return UserInterface::IsGood(ui);
}

/// Fetches global UI. 
bool GMUI::GetGlobalUI()
{
	if (!window)
		window = ActiveWindow();
	ui = window->GetGlobalUI();
	return ui != NULL;
	assert(false);
	/*
	if (viewportID == NULL)
		ui = Graphics.GetGlobalUI();
	// If it does not exist. Create it, since it is the system-global UI.
	if (!ui)
	{
		Graphics.SetGlobalUI(new UserInterface());
		ui = Graphics.GetGlobalUI();
		// Root not created automatically..? Create it now then.
		ui->CreateRoot();
		// Adjust to AppWindow straight away! o.o
		ui->AdjustToWindow(Graphics.Width(), Graphics.Height());
		return true;
	}
	*/
}

GMSetHoverUI::GMSetHoverUI(String uiName, UserInterface * inUI)
: GMUI(GM_SET_HOVER_UI, inUI), name(uiName)
{
}
void GMSetHoverUI::Process()
{
	if (!GetUI())
        return;
	if (!name.Length())
		return;
	UIElement * e = ui->GetElementByName(name);
	if (!e){
		std::cout<<"\nINFO: No element found with specified name \""<<name<<"\"";
		return;
	}
	e->Hover();
}

GMSetUIp::GMSetUIp(String uiName, int target, Texture * tex, UserInterface * ui)
	: GMUI(GM_SET_UI_POINTER, ui), name(uiName), target(target), texture(tex)
{
	switch(target)
	{
	case GMUI::TEXTURE:
		break;
	default:
		assert(false && "Bad target in GMSetUIp");
	}
}
void GMSetUIp::Process()
{
	if (!GetUI())
        return;
	if (!name.Length())
		return;
	UIElement * e = ui->GetElementByName(name);
	if (!e){
		std::cout<<"\nINFO: No element found with specified name \""<<name<<"\"";
		return;
	}
	switch(target)
	{
		case GMUI::TEXTURE: 		
		{
			e->texture = texture;
		}
	}	
}

/// Used to set arbitrary amounts of booleans. Mainly used for binary matrices (UIMatrix).
GMSetUIvb::GMSetUIvb(String uiName, int target, List<bool*> boolData, Viewport * viewport /*= NULL*/)
: GMUI(GM_SET_UI_VECB, viewport), name(uiName), target(target), data(boolData)
{
	switch(target){
		case GMUI::MATRIX_DATA:
			break;
		default:
			assert(false && "Invalid target in GMSetUIvb");
	}
}
void GMSetUIvb::Process() 
{
	if (!GetUI())
        return;
	if (!name.Length())
		return;
	UIElement * e = ui->GetElementByName(name);
	if (!e){
		std::cout<<"\nINFO: No element found with specified name \""<<name<<"\"";
		return;
	}
    switch(target){
        case GMUI::MATRIX_DATA:
        {
            if (e->type != UIType::MATRIX)
                return;
            UIMatrix * matrix = (UIMatrix*) e;
			matrix->SetData(data);
            break;
        }
	}
}

GMSetUIi::GMSetUIi(String uiName, int target, int value, UserInterface * ui)
: GMUI(GM_SET_UI_INTEGER, ui), uiName(uiName), target(target), value(value)
{
	switch(target)
	{
		case GMUI::INTEGER_INPUT:
			break;
		default:
			assert(false && "Bad target in GMSetUIi");
	}
}
void GMSetUIi::Process()
{
	if (!GetUI())
        return;
	if (!uiName.Length())
		return;
	UIElement * e = ui->GetElementByName(uiName);
	if (!e){
		LogGraphics("INFO: No element found with specified name \""+uiName+"\" in GMSetUIi", INFO);
		return;
	}
	switch(target)
	{
		case GMUI::INTEGER_INPUT:
		{
			switch(e->type)
			{
				case UIType::INTEGER_INPUT:
				{
					UIIntegerInput * intInput = (UIIntegerInput*) e;
					intInput->SetValue(value);
					break;			
				}
				case UIType::RADIO_BUTTONS:
				{
					UIRadioButtons * rb = (UIRadioButtons*)e;
					rb->SetValue(value);
					break;
				}
				default:
					if (e->type != UIType::INTEGER_INPUT)
					{
						std::cout<<"\nTrying to set integer input of element of other type.";
						assert(e->type == UIType::INTEGER_INPUT);
						return;
					}
				break;
			}
		}
	}
}


/// Used to set UI vector2i data. Primarily used to specify size of matrix or maybe later aboslute-size of an element.
GMSetUIv2i::GMSetUIv2i(String UIname, int target, Vector2i v, Viewport * viewport /* = NULL */)
: GMUI(GM_SET_UI_VEC2I, viewport), name(UIname), target(target), value(v)
{
	switch(target){
		case GMUI::MATRIX_SIZE:
		case GMUI::VECTOR_INPUT:
			break;
		default:
			assert(false && "Invalid target in GMSetUIv");
	}
}

GMSetUIv2i::GMSetUIv2i(String UIname, int target, Vector2i v, UserInterface * targetUI)
: GMUI(GM_SET_UI_VEC2I, targetUI), name(UIname), target(target), value(v)
{
	switch(target){
		case GMUI::MATRIX_SIZE:
		case GMUI::VECTOR_INPUT:
			break;
		default:
			assert(false && "Invalid target in GMSetUIv");
	}
}


void GMSetUIv2i::Process()
{
	if (!GetUI())
        return;
	if (!name.Length())
		return;
	UIElement * e = ui->GetElementByName(name);
	if (!e)
	{
		LogGraphics("INFO: No element found with specified name \""+name+"\" in GMSetUIv2i", INFO);
		return;
	}
    switch(target){
		case GMUI::VECTOR_INPUT:
		{
			if (e->type != UIType::VECTOR_INPUT)
                return;
            UIVectorInput * vecIn = (UIVectorInput*) e;
			vecIn->SetValue2i(value);
            break;
     		
		}
        case GMUI::MATRIX_SIZE:
        {
            if (e->type != UIType::MATRIX)
                return;
            UIMatrix * matrix = (UIMatrix*) e;
            matrix->SetSize(value);
            break;
        }
    };
};


GMSetUIv3f::GMSetUIv3f(String UIname, int target, const Vector3f & v, Viewport * viewport/* = NULL*/)
: GMUI(GM_SET_UI_VEC3F, viewport), name(UIname), target(target), value(v)
{
	AssertTarget();
}

GMSetUIv3f::GMSetUIv3f(String uiName, int target, ConstVec3fr v, UserInterface * ui)
: GMUI(GM_SET_UI_VEC3F, ui), name(uiName), target(target), value(v)
{
	AssertTarget();
}
void GMSetUIv3f::AssertTarget()
{
	switch(target){
		case GMUI::TEXT_COLOR:
		case GMUI::VECTOR_INPUT:
			break;
		default:
			assert(false && "Invalid target in GMSetUIv");
	}
}

void GMSetUIv3f::Process()
{
	if (!GetUI())
        return;
	if (!name.Length())
		return;
	UIElement * e = ui->GetElementByName(name);
	if (!e)
	{
		LogGraphics("INFO: No element found with specified name \""+name+"\"", DEBUG);
		return;
	}
    switch(target){
        case GMUI::TEXT_COLOR:
            e->textColor = value;
            break;
        case GMUI::VECTOR_INPUT:
            if (e->type != UIType::VECTOR_INPUT)
                return;
            UIVectorInput * vi = (UIVectorInput*) e;
            vi->SetValue4f(value);
            break;

    };
};

GMSetUIv4f::GMSetUIv4f(String UIname, int target, const Vector4f & v, UserInterface * ui)
: GMUI(GM_SET_UI_VEC4F, ui), name(UIname), target(target), value(v)
{
	AssertTarget();
}

GMSetUIv4f::GMSetUIv4f(String UIname, int target, const Vector4f & v, Viewport * viewport /*= NULL*/)
: GMUI(GM_SET_UI_VEC4F, viewport), name(UIname), target(target), value(v)
{
	AssertTarget();
}

void GMSetUIv4f::AssertTarget()
{
	switch(target){
		case GMUI::VECTOR_INPUT:
	    case GMUI::TEXT_COLOR:
			break;
		default:
			assert(false && "Invalid target in GMSetUIv4f");
	}
}


void GMSetUIv4f::Process()
{
	if (!GetUI())
        return;
	if (!name.Length())
		return;
	UIElement * e = ui->GetElementByName(name);
	if (!e)
	{
		LogGraphics("INFO: No element found with specified name \""+name+"\"", DEBUG);
		return;
	}
    switch(target){
        case GMUI::TEXT_COLOR:
            e->textColor = value;
            break;
        case GMUI::VECTOR_INPUT:
            if (e->type != UIType::VECTOR_INPUT)
                return;
            UIVectorInput * vi = (UIVectorInput*) e;
            vi->SetValue4f(value);
            break;
    };
};


/// Targets global UI
GMSetUIf::GMSetUIf(String UIname, int target, float value)
: GMUI(GM_SET_UI_FLOAT), name(UIname), target(target), value(value)
{
	AssertTarget();
}


GMSetUIf::GMSetUIf(String UIname, int target, float value, UserInterface * inUI)
: GMUI(GM_SET_UI_FLOAT, inUI), name(UIname), target(target), value(value)
{
	AssertTarget();
}

/// For setting floating point values, like relative sizes/positions, scales etc.
GMSetUIf::GMSetUIf(String UIname, int target, float value, Viewport * viewport /* = NULL*/)
: GMUI(GM_SET_UI_FLOAT, viewport), name(UIname), target(target), value(value)
{
	AssertTarget();
}

void GMSetUIf::AssertTarget()
{
	switch(target)
	{
		case GMUI::SCROLL_POSITION_Y:
		case GMUI::FLOAT_INPUT:
		case GMUI::TEXT_SIZE_RATIO:
		case GMUI::ALPHA:
		case GMUI::TEXT_ALPHA:
			break;
		default:
			assert(false && "Invalid target in GMSetUIf");
	}
}


void GMSetUIf::Process()
{
	if (!GetUI())
        return;
	if (!name.Length())
		return;
	element = ui->GetElementByName(name);
	if (!element){
		LogGraphics("INFO: No element found with specified name \""+name+"\"", DEBUG);
		return;
	}
	switch(target)
	{
		case GMUI::SCROLL_POSITION_Y:
		{
			assert(element->type == UIType::LIST);
			UIList * list = (UIList*) element;
			list->SetScrollPosition(value);
			break;
		}
		case GMUI::ALPHA:
			element->color[3] = value;
			break;
		case GMUI::TEXT_ALPHA:
			element->textColor[3] = value;
			break;
		case GMUI::TEXT_SIZE_RATIO:
			element->textSizeRatio = value;
			break;
		case GMUI::FLOAT_INPUT:
			if (element->type != UIType::FLOAT_INPUT)
				return;
			UIFloatInput * vi = (UIFloatInput*) element;
			vi->SetValue(value);
			break;

	};
};

GMSetUIb::GMSetUIb(String name, int target, bool v, UserInterface * inUI)
: GMUI(GM_SET_UI_BOOLEAN, inUI), name(name), target(target), value(v)
{
	AssertTarget();
}

GMSetUIb::GMSetUIb(String name, int target, bool v, Viewport * viewport)
: GMUI(GM_SET_UI_BOOLEAN, viewport), name(name), target(target), value(v)
{
	AssertTarget();
};

void GMSetUIb::AssertTarget()
{
	if (!name.Length())
		std::cout<<"ERROR: Invalid name-length of provided element, yo?";
    switch(target){
        case GMUI::VISIBILITY:
        case GMUI::TOGGLED:
        case GMUI::ACTIVATABLE:
		case GMUI::HOVERABLE:
		case GMUI::ENABLED:
		case GMUI::CHILD_TOGGLED:
		case GMUI::CHILD_VISIBILITY:
            break;
        default:
            assert(false && "bad target");
    };
}


/// For setting floating point values, like relative sizes/positions, scales etc. of elements in the system-global UI.
GMSetGlobalUIf::GMSetGlobalUIf(String uiName, int target, float value, AppWindow * window)
: GMSetUIf(uiName, target, value)
{
	if (!window)
		this->window = WindowMan.MainWindow();
	else
		this->window = window;
	global = true;
};


void GMSetUIb::Process()
{
	if (!GetUI())
        return;
	if (!name.Length())
		return;
	UIElement * e = ui->GetElementByName(name);
	if (!e){
		LogGraphics("INFO: No element found with specified name \""+name+"\"", DEBUG);
		return;
	}
	switch(target){
	    case GMUI::TOGGLED:
            e->toggled = value;
            break;
		case GMUI::CHILD_TOGGLED:
		{
			List<UIElement*> children = e->GetChildren();
			for (int i = 0; i < children.Size(); ++i){
				children[i]->toggled = value;
			}
			break;
		}
		case GMUI::VISIBILITY:
			e->visible = value;
			break;
		case GMUI::CHILD_VISIBILITY:
		{
			List<UIElement*> children = e->GetChildren();
			for (int i = 0; i < children.Size(); ++i){
				children[i]->visible = value;
			}
			break;
		}
		case GMUI::ACTIVATABLE:
			e->activateable = value;
			break;
		case GMUI::HOVERABLE:
			e->hoverable = value;
			break;
		case GMUI::ENABLED:
			if (value)
				e->RemoveState(UIState::DISABLED);
			else
				e->AddState(UIState::DISABLED);
			break;
		default:
			std::cout<<"\nERROR: Invalid target provided in GMSetUIb";
			assert(false && "ERROR: Invalid target provided in GMSetUIb");
			break;
	};
	Graphics.renderQueried = true;
};

// Targets the main windows ui.
GMSetUIs::GMSetUIs(String uiName, int target, Text text)
: GMUI (GM_SET_UI_TEXT), uiName(uiName), target(target), text(text), force(false)
{
	AssertTarget();
}

GMSetUIs::GMSetUIs(String uiName, int target, Text text, UserInterface * inUI)
: GMUI (GM_SET_UI_TEXT, inUI), uiName(uiName), target(target), text(text), force(false)
{
	AssertTarget();
}

GMSetUIs::GMSetUIs(String uiName, int target, Text text, bool force, UserInterface * inUI)
: GMUI (GM_SET_UI_TEXT, inUI), uiName(uiName), target(target), text(text), force(force)
{
	AssertTarget();
}

GMSetUIs::GMSetUIs(String uiName, int target, Text text, Viewport * viewport)
: GMUI (GM_SET_UI_TEXT, viewport), uiName(uiName), target(target), text(text), force(false)
{
	AssertTarget();
};

GMSetUIs::GMSetUIs(String uiName, int target, Text text, bool force, Viewport * viewport)
: GMUI (GM_SET_UI_TEXT, viewport), uiName(uiName), target(target), text(text), force(force)
{
	AssertTarget();
};

void GMSetUIs::AssertTarget()
{
	switch(target){
		case GMUI::TEXT:
		case GMUI::TEXTURE_SOURCE:
		case GMUI::TEXTURE_INPUT_SOURCE:
		case GMUI::STRING_INPUT_TEXT:
		case GMUI::INTEGER_INPUT_TEXT:
			break;
		default:
		{
			assert(false && "Invalid target provided in GMSetUIs");
			break;
		}
	}
}

GMSetUIs::~GMSetUIs(){
}

void GMSetUIs::Process()
{
	if (!GetUI())
        return;
	UIElement * e = ui->GetElementByName(uiName);
	if (!e){
		LogGraphics("INFO: No element found with specified name \""+uiName+"\"", DEBUG);
		return;
	}
	switch(target){
		case GMUI::TEXTURE_INPUT_SOURCE:
		{
			if (e->type != UIType::TEXTURE_INPUT)
				break;
			UITextureInput * ti = (UITextureInput*) e;
			ti->SetTextureSource(text);
			break;
		}
		case GMUI::TEXTURE_SOURCE:
			if (e->textureSource == text)
				break;
			e->textureSource = text;
			e->texture = NULL; // Force it to be re-fetched!
			e->FetchBindAndBufferizeTexture();
			break;
		case GMUI::STRING_INPUT_TEXT:
		{
			if (e->type != UIType::STRING_INPUT)
				break;
			UIStringInput * si = (UIStringInput*) e;
			si->input->SetText(text);
			break;
		}
		case GMUI::INTEGER_INPUT_TEXT:
		{
			if (e->type != UIType::INTEGER_INPUT)
				break;
			UIIntegerInput * ii = (UIIntegerInput*)e;
			ii->input->SetText(text);
			break;
		}
		case GMUI::TEXT:
			e->SetText(text, force);
			break;
		default:
			assert(false && "ERROR: Invalid target in GMSetUIs.");
			break;
	};
	Graphics.renderQueried = true;
}

GMSetGlobalUIs::GMSetGlobalUIs(String uiName, int target, Text text, bool force, AppWindow * window)
: GMSetUIs(uiName, target, text)
{
	if (!window)
		this->window = WindowMan.MainWindow();
	else
		this->window = window;
	global = true;
}

GMClearUI::GMClearUI(String uiName, UserInterface * inUI)
: GMUI(GM_CLEAR_UI, inUI), uiName(uiName){}

GMClearUI::GMClearUI(String uiName, Viewport * viewport)
: GMUI(GM_CLEAR_UI, viewport), uiName(uiName){}

void GMClearUI::Process(){
	if (!GetUI())
        return;
	UIElement * e = ui->GetElementByName(uiName);
	if (!e){
		LogGraphics("INFO: No element found with specified name \""+uiName+"\"", DEBUG);
		return;
	}
	e->Clear();
	Graphics.renderQueried = true;
}

GMScrollUI::GMScrollUI(String uiName, float scrollDiff, Viewport * viewport)
: GMUI(GM_SCROLL_UI, viewport), uiName(uiName), scrollDistance(scrollDiff)
{
}

void GMScrollUI::Process(){
    if (!GetUI())
        return;
    UIElement * e = ui->GetElementByName(uiName);
    if (e == NULL){
		LogGraphics("INFO: No element found with specified name \""+uiName+"\"", DEBUG);
        return;
    }
    switch(e->type){
        case UIType::LIST:{
            UIList * list = (UIList*) e;
            list->Scroll(scrollDistance);
            break;
        }
        default:
            assert(false && "Bad type in GMScrollUI");
    }
	Graphics.renderQueried = true;
}

/// Message to add a newly created UI to the global state's UI, mostly used for overlay-effects and handling error-messages.
GMAddGlobalUI::GMAddGlobalUI(UIElement *element, String toParent /* = "root" */)
: GMUI(GM_ADD_UI), element(element), parentName(toParent)
{
	assert(element);
	global = true;
}
void GMAddGlobalUI::Process()
{
	if (!GetGlobalUI())
        return;
	UIElement * e = NULL;
	if (parentName == "root")
		e = ui->GetRoot();
	else
		e = ui->GetElementByName(parentName);
	if (!e){
		std::cout<<"\nNo UIElement with given name could be found: "<<parentName;
		return;
	}
	e->AddChild(element);
	Graphics.renderQueried = true;
}

/// Message to add a newly created UI to the active game state's UI.
GMAddUI::GMAddUI(List<UIElement*> elements, String toParent, UserInterface * inUI)
: GMUI(GM_ADD_UI, inUI), elements(elements), parentName(toParent)
{
}

GMAddUI::GMAddUI(List<UIElement*> elements, String toParent, Viewport * viewport)
: GMUI(GM_ADD_UI, viewport), elements(elements), parentName(toParent)
{
}
void GMAddUI::Process()
{
	if (!GetUI())
        return;
	UIElement * e = NULL;
	if (parentName == "root")
		e = ui->GetRoot();
	else
		e = ui->GetElementByName(parentName);
	if (!e){
		std::cout<<"\nNo UIElement with given name could be found: "<<parentName;
		return;
	}
	for (int i = 0; i < elements.Size(); ++i)
	{
		e->AddChild(elements[i]);
	}
	Graphics.renderQueried = true;
}

GMPushUI::GMPushUI(String uiName, UserInterface * ontoUI)
: GMUI(GM_PUSH_UI, viewport), uiName(uiName), element(NULL)
{
	ui = ontoUI;
};

GMPushUI::GMPushUI(UIElement * element, UserInterface * ontoUI)
: GMUI(GM_PUSH_UI, viewport), element(element)
{
	assert(element);
	ui = ontoUI;
};

void GMPushUI::Process()
{
	if (!ui){
		if (!GetUI())
			return;
	}
	if (!ui)
	{
		std::cout<<"\nGMPushUI: Invalid UI.";
		return;
	}
	UIElement * e = NULL;
	if (element)
		e = element;
	else
		e = ui->GetElementByName(uiName);
	/// Check if it's a full resource string, if so try and create it now.
	if (!e && uiName.Contains(".gui"))
	{
		/// Try see if we can get it by source name.
		e = ui->GetElementBySource(uiName);
		if (!e)
		{
			e = UserInterface::LoadUIAsElement(uiName);
			/// If not added, add it too.
			if (e)
				ui->Root()->AddChild(e);
		}
	}
	if (!e){
		std::cout<<"\nGMPushUI: Invalid UIElement.";
		return;
	}
	/// Push to stack, the InputManager will also try and hover on the first primary element.
	InputMan.PushToStack(e, ui);

	/// Enable navigate ui if element wants it.
	if (e->navigateUIOnPush){
		e->previousNavigateUIState = InputMan.NavigateUIState();
		if (e->forceNavigateUI)
			InputMan.ForceNavigateUI(true);
		else
			InputMan.NavigateUI(true);
	}
}


/// Function that deletes a UI, notifying relevant parties beforehand.
void DeleteUI(UIElement * element, UserInterface * inUI)
{
	// Pause main thread.
	PrepareForUIRemoval();
	InputMan.OnElementDeleted(element);
	bool result = inUI->Delete(element);
	if (!result){
		std::cout<<"\nUnable to delete element: "<<element;
		std::cout<<"\nUnable to delete element: "<<element->name;
	}
	OnUIRemovalFinished();
}

GMPopUI::GMPopUI(String uiName, UserInterface * targetUI, bool force, Viewport * viewport)
: GMUI(GM_POP_UI, viewport), uiName(uiName), element(NULL), force(force)
{
	ui = targetUI;
}

void GMPopUI::Process()
{
	GetUI();
	if (!ui){
		std::cout<<"\nGMPopUI: Invalid UI.";
		return;
	}
	UIElement * e = NULL;
	if (element)
		e = element;
	else
		e = ui->GetElementByName(uiName);
	// Fetch by source if possible.
	if (!e)
		e = ui->GetElementBySource(uiName);
	if (!e)
	{
		LogGraphics("GMPopUI: Invalid UIElement: "+uiName, DEBUG);
		return;
	}
	/// Push to stack, the InputManager will also try and hover on the first primary element.
	bool success = InputMan.PopFromStack(e, ui, force);

    /// Post onExit message if it was popped.
    if (success)
	{
		/// Check if the element has any onPop messages.
		if (force)
		{
			if (e->onForcePop.Length())
				MesMan.QueueMessages(e->onForcePop);
		}
		else if (e->onPop.Length())
			MesMan.QueueMessages(e->onPop);
		/// If the element wants to keep track of the navigate UI state, then reload it. If not, don't as it will set it to false by default if so.
		if (e->navigateUIOnPush)
			InputMan.LoadNavigateUIState(e->previousNavigateUIState);

		/// If specified, remove the element too upon a successful pop-operation.
		if (e->removeOnPop){
			DeleteUI(e, ui);
		}
    }


	/// By default, set navigate UI to true too!
//	Input.NavigateUI(true);
}

/*
GMDeleteUI::GMDeleteUI(UIElement * element)
: GMUI(GM_DELETE_UI), element(element)
{
    std::cout<<"\nDeleting element??: "<<element->name;
}

void GMDeleteUI::Process(){
	assert(false && "GMDeleteUI DEPRECATED use GMRemoveUI! ");
    std::cout<<"\nDeleting element: "<<element->name;
    element->FreeBuffers();
    element->DeleteGeometry();
    delete element;
}
*/

GMRemoveUI::GMRemoveUI(UIElement * element)
: GMUI(GM_REMOVE_UI), element(element)
{
	assert(element != NULL);
}

void GMRemoveUI::Process(){
	if (!GetUI())
		return;
	DeleteUI(element, ui);
}

GMSetUIContents::GMSetUIContents(List<UIElement*> elements, String uiName)
: GMUI(GM_SET_UI_CONTENTS), elements(elements), uiName(uiName)
{
}
void GMSetUIContents::Process()
{
	GetUI();
	if (!ui){
		std::cout<<"\nGMPopUI: Invalid UI.";
		return;
	}
	UIElement * e = NULL;
	e = ui->GetElementByName(uiName);
	// Fetch by source if possible.
	if (!e)
		e = ui->GetElementBySource(uiName);
	if (!e){
		std::cout<<"\nGMPopUI: Invalid UIElement: "<<uiName;
		return;
	}
	switch(e->type)
	{
		case UIType::MATRIX:
			((UIMatrix*)e)->SetContents(elements);
			break;
	}
}

