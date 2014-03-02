// Emil Hedemalm
// 2013-07-14

#include "GMUI.h"
#include "../../UI/UserInterface.h"
#include "Graphics/GraphicsManager.h"
#include "../Render/RenderViewport.h"
#include "OS/Sleep.h"
#include "UI/UITypes.h"
#include "UI/UIList.h"
#include "UI/UIInput.h"
#include "UI/DataUI/UIMatrix.h"
#include "Input/InputManager.h"
#include "StateManager.h"
#include "Message/MessageManager.h"

/// Default constructor, specifies the viewport to be the default global one.
GMUI::GMUI(int messageType, int viewportID /* = NULL*/)
: GraphicsMessage(messageType), viewportID(viewportID)
{
	ui = NULL;
}

bool  GMUI::GetUI()
{
	if (viewportID == NULL)
		ui = Graphics.GetGlobalUI();
	else {
		RenderViewport * viewport = NULL;
		viewport = (RenderViewport*)Graphics.GetViewport(viewportID);
		if (!viewport)
			return false;
		ui = viewport->GetUI();
	}
    if (ui == NULL){
        std::cout<<"\nWARNING: No valid UI available for use.";
        return false;
    }
    return true;
}

/// Used to set arbitrary amounts of booleans. Mainly used for binary matrices (UIMatrix).
GMSetUIvb::GMSetUIvb(String uiName, int target, List<bool*> boolData, int viewport /*= NULL*/)
: GMUI(GM_SET_UI_VECB), name(uiName), target(target), data(boolData)
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


/// Used to set UI vector2i data. Primarily used to specify size of matrix or maybe later aboslute-size of an element.
GMSetUIv2i::GMSetUIv2i(String UIname, int target, Vector2i v, int viewport /* = NULL */)
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

void GMSetUIv2i::Process()
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


GMSetUIv3f::GMSetUIv3f(String UIname, int target, Vector3f v, int viewport/* = NULL*/)
: GMUI(GM_SET_UI_VEC3F, viewport), name(UIname), target(target), value(v)
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
	if (!e){
		std::cout<<"\nINFO: No element found with specified name \""<<name<<"\"";
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


GMSetUIv4f::GMSetUIv4f(String UIname, int target, Vector4f v, int viewport /*= NULL*/)
: GMUI(GM_SET_UI_VEC4F, viewport), name(UIname), target(target), value(v)
{
	switch(target){
	    case GMUI::TEXT_COLOR:
			break;
		default:
			assert(false && "Invalid target in GMSetUIv");
	}
}

void GMSetUIv4f::Process()
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

GMSetUIf::GMSetUIf(String UIname, int target, float value, int viewport /* = NULL*/)
: GMUI(GM_SET_UI_FLOAT, viewport), name(UIname), target(target), value(value)
{
	switch(target){
		case GMUI::FLOAT_INPUT:
		case GMUI::TEXT_SIZE_RATIO:
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
		std::cout<<"\nINFO: No element found with specified name \""<<name<<"\"";
		return;
	}
	switch(target){
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
}


GMSetUIb::GMSetUIb(String name, int target, bool v, int viewport)
: GMUI(GM_SET_UI_BOOLEAN, viewport), name(name), target(target), value(v)
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

};

void GMSetUIb::Process(){
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

GMSetUIs::GMSetUIs(String uiName, int target, Text text, int viewport)
: GMUI (GM_SET_UI_TEXT, viewport), uiName(uiName), target(target), text(text), force(false)
{
	switch(target){
		case GMUI::TEXT:
		case GMUI::TEXTURE_SOURCE:
		case GMUI::TEXTURE_INPUT_SOURCE:
		case GMUI::STRING_INPUT_TEXT:
			break;
		default:
		{
			assert(false && "Invalid target provided in GMSetUIs");
			break;
		}
	}
};

GMSetUIs::GMSetUIs(String uiName, int target, Text text, bool force, int viewport)
: GMUI (GM_SET_UI_TEXT, viewport), uiName(uiName), target(target), text(text), force(force)
{
};

GMSetUIs::~GMSetUIs(){
}

void GMSetUIs::Process(){
	if (!GetUI())
        return;
	UIElement * e = ui->GetElementByName(uiName);
	if (!e){
		// std::cout<<"\nNo UIElement with given name could be found: "<<uiName;
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
			e->textureSource = text;
			e->texture = 0;
			break;
		case GMUI::STRING_INPUT_TEXT:
		{
			if (e->type != UIType::STRING_INPUT)
				break;
			UIStringInput * si = (UIStringInput*) e;
			si->input->SetText(text);
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

GMClearUI::GMClearUI(String uiName, int viewport)
: GMUI(GM_CLEAR_UI, viewport), uiName(uiName){}

void GMClearUI::Process(){
	if (!GetUI())
        return;
	UIElement * e = ui->GetElementByName(uiName);
	if (!e){
		std::cout<<"\nNo UIElement with given name could be found: "<<uiName;
		return;
	}
	e->Clear();
	Graphics.renderQueried = true;
}

GMScrollUI::GMScrollUI(String uiName, float scrollDiff, int viewport)
: GMUI(GM_SCROLL_UI, viewport), uiName(uiName), scrollDistance(scrollDiff)
{
}

void GMScrollUI::Process(){
    if (!GetUI())
        return;
    UIElement * e = ui->GetElementByName(uiName);
    if (e == NULL){
        std::cout<<"\nERROR: Unable to find element by given name \""<<uiName<<"\" in GMScrollUI";
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

GMAddUI::GMAddUI(UIElement * element, String toParent, int viewport)
: GMUI(GM_ADD_UI, viewport), element(element), parentName(toParent){
	assert(element);
}
void GMAddUI::Process(){
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
	e->AddChild(element);
	Graphics.renderQueried = true;
}

GMPushUI::GMPushUI(String uiName, int viewport)
: GMUI(GM_PUSH_UI, viewport), uiName(uiName), element(NULL){};

GMPushUI::GMPushUI(UIElement * ui, int viewport)
: GMUI(GM_PUSH_UI, viewport), element(ui){};

void GMPushUI::Process(){
	if (!GetUI())
        return;
	if (!ui){
		std::cout<<"\nGMPushUI: Invalid UI.";
		return;
	}
	UIElement * e = NULL;
	if (element)
		e = element;
	else
		e = ui->GetElementByName(uiName);
	/// Check if it's a full resource string, if so try and create it now.
	if (uiName.Contains(".gui"))
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
	Input.PushToStack(e, ui);

	/// Enable navigate ui if element wants it.
	if (e->navigateUIOnPush){
		e->previousNavigateUIState = Input.NavigateUIState();
		if (e->forceNavigateUI)
			Input.ForceNavigateUI(true);
		else
			Input.NavigateUI(true);
	}
}


/// Function that deletes a UI, notifying relevant parties beforehand.
void DeleteUI(UIElement * element, UserInterface * inUI){
	Input.acceptInput = false;
	Input.OnElementDeleted(element);
#define SLEEP_TIME	10
	Sleep(SLEEP_TIME);
	bool result = inUI->Delete(element);
	if (!result){
		std::cout<<"\nUnable to delete element: "<<element;
		std::cout<<"\nUnable to delete element: "<<element->name;
	}
	Sleep(SLEEP_TIME);
	Input.acceptInput = true;
}

GMPopUI::GMPopUI(String uiName, bool force, int viewport)
: GMUI(GM_POP_UI, viewport), uiName(uiName), element(NULL), force(force){}

void GMPopUI::Process(){
	if (!GetUI())
        return;
	if (!ui){
		std::cout<<"\nGMPopUI: Invalid UI.";
		return;
	}
	UIElement * e = NULL;
	if (element)
		e = element;
	else
		e = ui->GetElementByName(uiName);
	if (!e){
		std::cout<<"\nGMPopUI: Invalid UIElement: "<<uiName;
		return;
	}
	/// Push to stack, the InputManager will also try and hover on the first primary element.
	bool success = Input.PopFromStack(e, ui, force);

    /// Post onExit message if it was popped.
    if (success){
        MesMan.QueueMessages(e->onExit);

		/// When popping, either check the stack or just revert to the previous state.
		Input.LoadNavigateUIState(e->previousNavigateUIState);

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
