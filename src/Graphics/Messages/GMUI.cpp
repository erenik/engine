// Emil Hedemalm
// 2013-07-14

#include "InputState.h"
#include "UI/UITypes.h"
#include "UI/UI.h"
#include "GMUI.h"
#include "../../UI/UserInterface.h"
#include "Graphics/GraphicsManager.h"
#include "Viewport.h"
#include "OS/Sleep.h"
#include "Message/Message.h"

#include "Window/AppWindow.h"
#include "UI/UITypes.h"
#include "UI/UILists.h"
#include "UI/UIInputs.h"
#include "UI/UIBar.h"
#include "UI/DataUI/UIMatrix.h"
#include "UI/Buttons/UIRadioButtons.h"
#include "UI/UILabel.h"

#include "Input/InputManager.h"
#include "StateManager.h"
#include "Message/MessageManager.h"
#include "Window/AppWindowManager.h"

#include "File/LogFile.h"
#include "UI/UIFileBrowser.h"

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
	retryOnFailure = false;
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
		window = WindowMan.GetCurrentlyActiveWindow();
		if (!window)
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
		if (retryOnFailure)
			retry = true;
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

GMSetHoverUI::GMSetHoverUI(void* nullPointer, UserInterface* inUI)
	: GMUI(GM_SET_HOVER_UI, inUI), nullify(true)
{
}


GMSetHoverUI::GMSetHoverUI(String uiName, UserInterface * inUI)
: GMUI(GM_SET_HOVER_UI, inUI), name(uiName)
{
}
void GMSetHoverUI::Process(GraphicsState* graphicsState)
{
	if (!GetUI())
        return;
	if (!name.Length())
		return;
	if (nullify) {
		ui->SetHoverElement(graphicsState, nullptr);
		return;
	}

	UIElement * e = ui->GetElementByName(name);
	if (!e){
		std::cout<<"\nINFO: No element found with specified name \""<<name<<"\"";
		return;
	}
	ui->SetHoverElement(graphicsState, e);
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
void GMSetUIp::Process(GraphicsState * graphicsState)
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
			e->visuals.texture = texture;
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
void GMSetUIvb::Process(GraphicsState * graphicsState) 
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
#define GetElement(t) UIElement * e = ui->GetElementByName(uiName); \
	if (!e){\
		LogGraphics("INFO: No element found with specified name \""+uiName+"\" in "+String(t), INFO);\
		retry = true;\
		return;\
	}

void GMSetUIi::Process(GraphicsState * graphicsState)
{
	if (!GetUI())
        return;
	if (!uiName.Length())
		return;
	GetElement("GMSetUIi");
/*	UIElement * e = ui->GetElementByName(uiName);
	if (!e){
		LogGraphics("INFO: No element found with specified name \""+uiName+"\" in GMSetUIi", INFO);
		return;
	}
	*/
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
					rb->SetValue(graphicsState, value);
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


void GMSetUIv2i::Process(GraphicsState * graphicsState)
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
            matrix->SetSize(*graphicsState, value);
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

void GMSetUIv3f::Process(GraphicsState * graphicsState)
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
			e->SetTextColors(TextColors(value));
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
		case GMUI::COLOR:
		case GMUI::VECTOR_INPUT:
	    case GMUI::TEXT_COLOR:
			break;
		default:
			assert(false && "Invalid target in GMSetUIv4f");
	}
}


void GMSetUIv4f::Process(GraphicsState * graphicsState)
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
		case GMUI::COLOR:
			e->SetColor(value);
			break;
        case GMUI::TEXT_COLOR:
			e->SetTextColors(TextColors(value));
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
		case GMUI::CHILD_SIZE_RATIO_Y:
		case GMUI::BAR_FILL_RATIO:
			break;
		default:
			assert(false && "Invalid target in GMSetUIf");
	}
}


void GMSetUIf::Process(GraphicsState * graphicsState)
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
			list->SetScrollPosition(*graphicsState, value);
			break;
		}
		case GMUI::ALPHA:
			element->visuals.color[3] = value;
			break;
		case GMUI::TEXT_ALPHA:
			element->SetTextColors(element->GetText().colors->WithAlpha(value));
			break;
		case GMUI::TEXT_SIZE_RATIO:
			element->text.sizeRatio = value;
			break;
		case GMUI::CHILD_SIZE_RATIO_Y:
		{
			UIList * l = (UIList*)element;
			l->RescaleChildrenY(value);
			break;
		}
		case GMUI::FLOAT_INPUT: {
			if (element->type != UIType::FLOAT_INPUT)
				return;
			UIFloatInput * vi = (UIFloatInput*) element;
			vi->SetValue(value);
			break;
		}
		case GMUI::BAR_FILL_RATIO:
			if (element->type != UIType::BAR)
				return;
			UIBar * bar = (UIBar*)element;
			bar->SetFill(graphicsState, value);
			break;

	};
};

GMSetUIb::GMSetUIb(String name, int target, bool v, UserInterface * inUI)
: GMUI(GM_SET_UI_BOOLEAN, inUI), name(name), target(target), value(v), filter(UIFilter::None)
{
	AssertTarget();
}

GMSetUIb::GMSetUIb(String name, int target, bool v, Viewport * viewport)
: GMUI(GM_SET_UI_BOOLEAN, viewport), name(name), target(target), value(v), filter(UIFilter::None)
{
	AssertTarget();
};

GMSetUIb::GMSetUIb(String uiName, int target, bool v, UIFilter filter, UserInterface * inUI)
	: GMUI(GM_SET_UI_BOOLEAN, inUI), name(uiName), target(target), value(v), filter(filter)
{
	AssertTarget();
}

void GMSetUIb::AssertTarget()
{
	if (!name.Length())
		std::cout<<"ERROR: Invalid name-length of provided element, yo?";
    switch(target){
        case GMUI::VISIBILITY:
        case GMUI::TOGGLED:
        case GMUI::ACTIVATABLE:
		case GMUI::HOVERABLE:
		case GMUI::HOVER_STATE:
		case GMUI::ENABLED:
		case GMUI::CHILD_TOGGLED:
		case GMUI::CHILD_VISIBILITY:
		case GMUI::ACTIVE:
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


void GMSetUIb::Process(GraphicsState * graphicsState)
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
		case GMUI::TOGGLED: {
			UIToggleButton * toggleButton = (UIToggleButton*) e;
			toggleButton->SetToggledSilently(value);
			break;
		}
		case GMUI::CHILD_TOGGLED:
		{
			List<UIElement*> children = e->GetChildren();
			for (int i = 0; i < children.Size(); ++i){
				UIToggleButton * toggleButton = (UIToggleButton*)children[i];
				toggleButton->SetToggled(value);
			}
			break;
		}
		case GMUI::VISIBILITY:
			e->interaction.visible = value;
			break;
		case GMUI::CHILD_VISIBILITY:
		{
			List<UIElement*> children = e->GetChildren();
			for (int i = 0; i < children.Size(); ++i){
				children[i]->interaction.visible = value;
			}
			break;
		}
		case GMUI::ACTIVATABLE:
			e->interaction.activateable = value;
			break;
		case GMUI::HOVERABLE:
			e->interaction.hoverable = value;
			break;
		case GMUI::HOVER_STATE:
			if (value)
				e->AddState(graphicsState, UIState::HOVER);
			else
				e->RemoveState(UIState::HOVER);
			break;
		case GMUI::ENABLED:
			if (value)
				e->RemoveState(UIState::DISABLED, filter);
			else
				e->AddState(graphicsState, UIState::DISABLED, filter);
			break;
		case GMUI::ACTIVE:
			/// Ensure it is visible first..?
			if (!e->IsVisible())
			{
				return;
			}
			if (value)
				e->AddState(graphicsState, UIState::ACTIVE);
			else
				e->RemoveState(UIState::ACTIVE);
			//e->Activate(graphicsState);
			break;
		default:
			std::cout<<"\nERROR: Invalid target provided in GMSetUIb";
			assert(false && "ERROR: Invalid target provided in GMSetUIb");
			break;
	};
	Graphics.renderQueried = true;
};

// Targets the main windows ui.
GMSetUIs::GMSetUIs(String uiName, int target, String text)
: GMUI (GM_SET_UI_STRING), uiName(uiName), target(target), text(text), force(false)
{
	AssertTarget();
}

GMSetUIs::GMSetUIs(String uiName, int target, String text, UserInterface * inUI)
: GMUI (GM_SET_UI_STRING, inUI), uiName(uiName), target(target), text(text), force(false)
{
	AssertTarget();
}

GMSetUIs::GMSetUIs(String uiName, int target, String text, bool force, UserInterface * inUI)
: GMUI (GM_SET_UI_STRING, inUI), uiName(uiName), target(target), text(text), force(force)
{
	AssertTarget();
}

GMSetUIs::GMSetUIs(String uiName, int target, String text, Viewport * viewport)
: GMUI (GM_SET_UI_STRING, viewport), uiName(uiName), target(target), text(text), force(false)
{
	AssertTarget();
};

GMSetUIs::GMSetUIs(String uiName, int target, String text, bool force, Viewport * viewport)
: GMUI (GM_SET_UI_STRING, viewport), uiName(uiName), target(target), text(text), force(force)
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
		case GMUI::STRING_INPUT:
		case GMUI::FILE_INPUT:
		case GMUI::INTEGER_INPUT_TEXT:
		case GMUI::ACTIVATION_MESSAGE:
		case GMUI::DROP_DOWN_INPUT_SELECT:
		case GMUI::FILE_BROWSER_PATH:
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

void GMSetUIs::Process(GraphicsState * graphicsState)
{
	if (!GetUI())
        return;
	GetElement("GMSetUIs");
	switch(target){
	case GMUI::FILE_BROWSER_PATH: {
		if (e->type != UIType::FILE_BROWSER)
			break;
		UIFileBrowser* fileBrowser = (UIFileBrowser*)e;
		fileBrowser->SetPath(text, true);
		break;
	}
		case GMUI::TEXTURE_INPUT_SOURCE:
		{
			if (e->type != UIType::TEXTURE_INPUT)
				break;
			UITextureInput * ti = (UITextureInput*) e;
			ti->SetTextureSource(text);
			break;
		}
		case GMUI::TEXTURE_SOURCE:
			if (e->visuals.textureSource == text)
				break;
			e->visuals.textureSource = text;
			e->visuals.texture = NULL; // Force it to be re-fetched!
			e->FetchBindAndBufferizeTexture();
			break;
		case GMUI::DROP_DOWN_INPUT_SELECT:
			if (e->type == UIType::DROP_DOWN_MENU) {
				UIDropDownMenu * dropDownMenu = (UIDropDownMenu*)e;
				dropDownMenu->Select(text, true);
				break;
			}
			break;
		case GMUI::STRING_INPUT_TEXT:
		{
			if (e->type != UIType::STRING_INPUT)
				break;
			UIStringInput * si = (UIStringInput*) e;
			si->input->SetText(text);
			break;
		}
		case GMUI::STRING_INPUT: 
		{ 
			if (e->type != UIType::STRING_INPUT)
				break;
			UIStringInput * si = (UIStringInput*)e;
			si->SetValue(text);
			break;
		}
		case GMUI::FILE_INPUT: {
			if (e->type != UIType::FILE_INPUT)
				break;
			UIFileInput * fi = (UIFileInput*)e;
			fi->SetValue(text);
			break;
			break;
		}
		case GMUI::INTEGER_INPUT_TEXT:
		{
			if (e->type != UIType::INTEGER_INPUT)
				break;
			UIIntegerInput * ii = (UIIntegerInput*)e;
			ii->label->SetText(text);
			break;
		}
		case GMUI::TEXT:
			e->SetText(text, force);
			break;
		case GMUI::ACTIVATION_MESSAGE:
			e->activationMessage = text;
			break;
		default:
			assert(false && "ERROR: Invalid target in GMSetUIs.");
			break;
	};
	Graphics.renderQueried = true;
}

GMSetUIt::GMSetUIt(String uiName, int target, CTextr tex)
: GMUI (GM_SET_UI_TEXT), uiName(uiName), target(target), text(tex)
{
	AssertTarget();
}
GMSetUIt::GMSetUIt(String uiName, int target, CTextr tex, UserInterface* ui)
: GMUI(GM_SET_UI_TEXT, ui), uiName(uiName), target(target), text(tex)
{
	AssertTarget();
}
GMSetUIt::GMSetUIt(String uiName, int target, List<Text> texts)
: GMUI(GM_SET_UI_TEXT), uiName(uiName), target(target), texts(texts)
{
	AssertTarget();
}

/** Explicitly declared constructor to avoid memory leaks.
	No explicit constructor may skip subclassed variable deallocation!
*/
GMSetUIt::~GMSetUIt()
{

}
void GMSetUIt::AssertTarget()
{
	switch(target){
		case GMUI::LOG_APPEND:
		case GMUI::LOG_FILL:
		case GMUI::TEXT:
		case GMUI::STRING_INPUT_TEXT:
			break;
		default:
		{
			assert(false && "Invalid target provided in GMSetUIs");
			break;
		}
	}
}

void GMSetUIt::Process(GraphicsState * graphicsState)
{
	if (!GetUI())
        return;
	GetElement("GMSetUIt");
	switch(target){
	case GMUI::TEXT:
		e->SetText(text);
		break;

	case GMUI::STRING_INPUT_TEXT:
	{
		if (e->type != UIType::STRING_INPUT)
			break;
		UIStringInput * si = (UIStringInput*)e;
		si->input->SetText(text);
		break;
	}
	case GMUI::LOG_APPEND:
	{
		if (e->type != UIType::LOG)
			break;
		UILog * l = (UILog*) e;
		l->Append(text);
		break;
	}
	case GMUI::LOG_FILL:
	{
		if (e->type != UIType::LOG)
			break;
		UILog * l = (UILog*) e;
		l->Fill(texts);
		break;
	}
	default:
		assert(false && "ERROR: Invalid target in GMSetUIt.");
		break;
	};
	Graphics.renderQueried = true;
}


GMSetGlobalUIs::GMSetGlobalUIs(String uiName, int target, String text, bool force, AppWindow * window)
: GMSetUIs(uiName, target, text)
{
	if (!window)
		this->window = WindowMan.MainWindow();
	else
		this->window = window;
	global = true;
}

GMClearUI::GMClearUI(List<String> uiNames)
: GMUI(GM_CLEAR_UI), uiNames(uiNames) {}

GMClearUI::GMClearUI(String uiName, UserInterface * inUI)
: GMUI(GM_CLEAR_UI, inUI)
{
	uiNames.AddItem(uiName);
}

GMClearUI::GMClearUI(String uiName, Viewport * viewport)
: GMUI(GM_CLEAR_UI, viewport)
{
	uiNames.AddItem(uiName);
}

void GMClearUI::Process(GraphicsState * graphicsState)
{
	if (!GetUI())
        return;
	for (int i = 0; i < uiNames.Size(); ++i)
	{
		String uiName = uiNames[i];
		UIElement * e = ui->GetElementByName(uiName);
		if (!e){
			LogGraphics("GMClearUI: No element found with specified name \""+uiName+"\"", DEBUG);
			return;
		}
		e->Clear(*graphicsState);
	}
	Graphics.renderQueried = true;
}

GMScrollUI::GMScrollUI(String uiName, float scrollDiff, Viewport * viewport)
: GMUI(GM_SCROLL_UI, viewport), uiName(uiName), scrollDistance(scrollDiff)
{
}

GMScrollUI::GMScrollUI(String uiName, float scrollDistance, UserInterface * ui)
: GMUI(GM_SCROLL_UI, ui), uiName(uiName), scrollDistance(scrollDistance)
{
}

void GMScrollUI::Process(GraphicsState * graphicsState){
    if (!GetUI())
        return;
    UIElement * e = ui->GetElementByName(uiName);
    if (e == NULL){
		LogGraphics("GMScrollUI: No element found with specified name \""+uiName+"\"", INFO);
        return;
    }
	LogGraphics("Scrolling on element: " + uiName, INFO);
	bool scrolled = false;
	switch(e->type){
        case UIType::LIST:{
            UIList * list = (UIList*) e;
			scrolled = list->Scroll(graphicsState, scrollDistance);
            break;
        }
        default:
			scrolled = e->OnScroll(graphicsState, scrollDistance);
    }
	if (scrolled) {
		// Hover with last known cursor co-ordinates.
		ui->Hover(graphicsState, InputMan.GetMousePosition());
	}
	else {
		MouseMessage * mm = new MouseMessage(MouseMessage::SCROLL, nullptr, Vector2i());
		mm->scrollDistance = scrollDistance;
		if (e->interaction.hoverable)
			mm->element = e;
		MesMan.QueueMessage(mm);
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
void GMAddGlobalUI::Process(GraphicsState* graphicsState)
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
	e->AddChild(graphicsState, element);
	Graphics.renderQueried = true;
}

/// Message to add a newly created UI to the active game state's UI.
GMAddUI::GMAddUI(List<UIElement*> elements, String toParent, UserInterface * inUI)
: GMUI(GM_ADD_UI, inUI), elements(elements), parentName(toParent)
{
}

GMAddUI::GMAddUI(List<UIElement*> elements, String toParent, Viewport * viewport, UIFilter withFilter)
: GMUI(GM_ADD_UI, viewport), elements(elements), parentName(toParent), filter(withFilter)
{
}
void GMAddUI::Process(GraphicsState * graphicsState)
{
	if (!GetUI())
        return;
	UIElement * e = NULL;
	if (parentName == "root")
		e = ui->GetRoot();
	else
		e = ui->GetElementByName(parentName, filter);
	if (!e){
		LogGraphics("GMAddUI: No UIElement with given name could be found: "+parentName, INFO);
		return;
	}
	for (int i = 0; i < elements.Size(); ++i)
	{
		auto element = elements[i];
		e->AddChild(graphicsState, elements[i]);
		element->AdjustToParentCreateGeometryAndBufferize(graphicsState);
	}
	Graphics.renderQueried = true;
}

// If 0, grabs main window.
GMPushUI::GMPushUI(String sourceName) // Default window and UI.
	: GMUI(GM_PUSH_UI), element(0), uiName(sourceName)
{
	this->window = 0;
	this->ui = 0;
}
GMPushUI::GMPushUI(UIElement * in_element)
	: GMUI(GM_PUSH_UI), element(in_element)
{
	this->window = 0;
	this->ui = 0;
}


GMPushUI * GMPushUI::ByNameOrSource(String elementName) {
	GMPushUI * p = new GMPushUI(elementName);
	p->searchAllUI = true;
	return p;
}

/// Creates and returns a new message, aimed at a specific window (or the main one, if 0).
GMPushUI * GMPushUI::ToWindow(String elementName, AppWindow * window)
{
	GMPushUI * p = new GMPushUI(elementName);
	p->window = window;
	return p;
}
GMPushUI * GMPushUI::ToUI(String elementName, UserInterface * toUI)
{
	GMPushUI * p = new GMPushUI(elementName);
	p->ui = toUI;
	return p;
}
GMPushUI * GMPushUI::ToUI(UIElement * element, UserInterface * ontoUI)
{
	GMPushUI * p = new GMPushUI(element);
	p->ui = ontoUI;
	return p;	
}


void GMPushUI::Process(GraphicsState * graphicsState)
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

	if (ui->Root() == nullptr) {
		ui->CreateRoot();
		LogGraphics("Emergency creating root!", WARNING);
	}

	/// Check if it's a full resource string, if so try and create it now.
	if (!e && uiName.Contains(".gui"))
	{
		/// Try see if we can get it by source name.
		e = ui->GetElementBySource(uiName);
		if (!e)
		{
			e = UserInterface::LoadUIAsElement(graphicsState, uiName);
			/// If not added, add it too.
			if (e)
				ui->Root()->AddChild(graphicsState, e);
		}
	}
	if (!e){
		std::cout<<"\nGMPushUI: Invalid UIElement.";
		return;
	}

	PushUI(e, ui, graphicsState);
}

void GMPushUI::PushUI(UIElement * e, UserInterface* ui, GraphicsState * graphicsState) {
	/// Add to root.
	if (e->Parent() == 0)
		ui->GetRoot()->AddChild(graphicsState, e);
	/// Push to stack, the InputManager will also try and hover on the first primary element.
	PushToStack(graphicsState, ui, e);

	/// Enable navigate ui if element wants it.
	if (e->interaction.navigateUIOnPush) {
		e->interaction.previousNavigateUIState = InputMan.NavigateUIState();
		if (e->interaction.forceNavigateUI)
			InputMan.SetForceNavigateUI(true);
		else
			InputMan.SetNavigateUI(true);
	}

	MesMan.QueueMessage(new OnUIPushed(e->name));
}

void GMPushUI::PushToStack(GraphicsState* graphicsState, UserInterface* ui, UIElement* element) {
	if (!ui || !element)
		return;
	/// Push it.
	int result = ui->PushToStack(element);
	if (result == UserInterface::NULL_ELEMENT)
		return;
	UIElement * firstActivatable = element->GetElementByFlag(UIFlag::HOVERABLE | UIFlag::ACTIVATABLE);
	//	std::cout<<"\nHovering to element \""<<firstActivatable->name<<"\" with text \""<<firstActivatable->text<<"\"";
	ui->SetHoverElement(graphicsState, firstActivatable);

	// Set navigation cyclicity.
	static bool cyclicY = false;

	cyclicY = element->interaction.cyclicY;
	//hoverElement = ui->Hover(firstActivatable->layout.posX, firstActivatable->layout.posY);

	ui->PrintStack();

	// Bufferize upon pushing to stack!
	element->AdjustToWindow(*graphicsState, ui->GetLayout());
	element->CreateGeometry(graphicsState);
	element->ResizeGeometry(graphicsState);
	element->Bufferize();
}



/// Function that deletes a UI, notifying relevant parties beforehand.
void DeleteUI(GraphicsState* graphicsState, UIElement * element, UserInterface * inUI)
{
	// Pause main thread.
	PrepareForUIRemoval();
	InputMan.OnElementDeleted(element);
	bool result = inUI->Delete(graphicsState, element);
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

GMPopUI::GMPopUI(String uiName, bool force) 
	: GMUI(GM_POP_UI), uiName(uiName), element(nullptr), force(force)
{
	ui = nullptr;
}



void GMPopUI::Process(GraphicsState * graphicsState)
{
	GetUI();
	if (!ui) {
		std::cout << "\nGMPopUI: Invalid UI.";
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
		LogGraphics("GMPopUI: Invalid UIElement: " + uiName, DEBUG);
		return;
	}

	/// Push to stack, the InputManager will also try and hover on the first primary element.
	String elementName = e->name;
	bool success = PopFromStack(graphicsState, e, ui, force);
	if (!success)
		return;

	LogGraphics("Popped element/menu: " + e->name, INFO);
	/// If the element wants to keep track of the navigate UI state, then reload it. If not, don't as it will set it to false by default if so.
	if (e->interaction.disableNavigateUIOnPop)
		InputMan.LoadNavigateUIState(e->interaction.previousNavigateUIState);

	// Clean it up, yo.
	if (e->source.Length() > 0) {
		ui->GetRoot()->RemoveChild(graphicsState, e);
		e->FreeBuffers(*graphicsState);
		delete e;
	}

	/// By default, set navigate UI to true too!
//	InputMan.SetNavigateUI(true);
}

UIElement * GMPopUI::PopFromStack(GraphicsState* graphicsState, UIElement * element, UserInterface * ui, bool force) {
	if (!ui || !element) {
		std::cout << "\nNull UI or element";
		return NULL;
	}
	// If trying to pop root, assume user is trying to cancel the UI-navigation mode?
	if (element->name == "root") {
		InputMan.SetNavigateUI(false);
	}
	/// Pop it.
	bool success = ui->PopFromStack(graphicsState, element, force);
	if (!success) {
		LogGraphics("Unable to pop UI from stack, might require force=true", FATAL);
		return NULL;
	}

	ui->PrintStack();

	// Set new navigation cyclicity.
	UIElement * stackTop = ui->GetStackTop();
	static bool cyclicY = false;
	cyclicY = stackTop->interaction.cyclicY;

	// If element was previously active, remove it, keep only hover.
	stackTop->RemoveState(UIState::ACTIVE, true);

	UIElement * currentHover = stackTop->GetElementByState(UIState::HOVER);
	if (currentHover) {
		ui->SetHoverElement(graphicsState, currentHover);
		return element;
	}
	UIElement * firstActivatable = stackTop->GetElementByFlag(UIFlag::HOVERABLE | UIFlag::ACTIVATABLE);
	//	std::cout<<"\nHovering to element \""<<firstActivatable->name<<"\" with text \""<<firstActivatable->text<<"\"";
	ui->SetHoverElement(graphicsState, firstActivatable);

	/// If no activatable menu item is out, set navigate UI to false?
	/// No. Better embed this into the appropriate UI's onExit message!
	// InputMan.SetNavigateUI(true);
	return element;
}

/*
GMDeleteUI::GMDeleteUI(UIElement * element)
: GMUI(GM_DELETE_UI), element(element)
{
    std::cout<<"\nDeleting element??: "<<element->name;
}

void GMDeleteUI::Process(GraphicsState * graphicsState){
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

void GMRemoveUI::Process(GraphicsState * graphicsState){
	if (!GetUI())
		return;
	DeleteUI(graphicsState, element, ui);
}
/// Used for setting String-list in Drop-down menus
GMSetUIContents::GMSetUIContents(String uiName, List<String> contents)
: GMUI(GM_SET_UI_CONTENTS), contents(contents), uiName(uiName)
{}
GMSetUIContents::GMSetUIContents(UserInterface * ui, String uiName, List<String> contents)
: GMUI(GM_SET_UI_CONTENTS, ui), contents(contents), uiName(uiName)
{}
GMSetUIContents::GMSetUIContents(List<UIToggleButton*> elements, String uiName)
: GMUI(GM_SET_UI_CONTENTS), toggleButtons(elements), uiName(uiName)
{
}
void GMSetUIContents::Process(GraphicsState * graphicsState)
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
	{
		e = ui->GetElementBySource(uiName);
	}
	if (!e){
		std::cout<<"\nGMPopUI: Invalid UIElement: "<<uiName;
		return;
	}
	switch(e->type)
	{
		case UIType::MATRIX:
			((UIMatrix*)e)->SetContents(graphicsState, toggleButtons);
			break;
		case UIType::DROP_DOWN_LIST:
			((UIDropDownList*)e)->SetContents(graphicsState, contents);
	}
}

