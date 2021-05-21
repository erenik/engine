/// Emil Hedemalm
/// 2014-01-02
/// UI subclass that can create a (modal) dialogue to demand user interaction before continuing.

#include "UITypes.h"
#include "UIQueryDialogue.h"
#include "UILists.h"
#include "UIButtons.h"
#include "UIInputs.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"

/// Used to give unique IDs to each dialogue
int UIQueryDialogue::dialoguesCreated = 0;

UIQueryDialogue::UIQueryDialogue(String queryHeader, String actionToBeTakenIfProceeding, String actionToBeTakenIfDeclining)
: UIElement()
{
	type = UIType::QUERY_DIALOGUE;
	this->headerText = queryHeader;
	this->actionToBeTakenIfProceeding = actionToBeTakenIfProceeding;
	this->actionToBeTakenIfDeclining = actionToBeTakenIfDeclining;
	modal = true;
	/// Set modalitity using hoverable property, and disable the highlight so the user doesn't get confused (or the bg will flash when moving the cursor)
	this->interaction.hoverable = true;
	interaction.navigatable = true;
	visuals.highlightOnHover = false;
	/// Disable any automatic popping, so recommend adding it as an actionToBeTakenIfProceeding?
	this->interaction.exitable = false;
	/// If modal, set a texture for it too?
	visuals.textureSource = "black50Alpha";
	/// Set ID and name
	this->name = "UIQueryDialogue_"+String::ToString(this->id);


	/// Default "Are you sure? Any unsaved data may be lost."
	textToPresent = "Are you sure you wish to continue? Any unsaved data may be lost.";

	/// Boolean on whether this dialogue should be popped from the UI  after it's usage. Default is true.
	popUponContinuing = true;
	/// Boolean on whether this dialogue should be deleted after it's usage. Default is true.
	deleteUponContinuing = false;
	removeOnPop = true;
	okButton = cancelButton = NULL;


	/// Wether NavigateUI should be enabled when this element is pushed.
	interaction.navigateUIOnPush = true;
	/// If force navigate UI should be applied for this element.
	interaction.forceNavigateUI = true;
}

UIQueryDialogue::~UIQueryDialogue()
{
	std::cout<<"\nUIQueryDialogue destructor";
}


#include "UI/UILabel.h"

/// Creates the relevant children. Separate function in order to not have everything allocated in the constructor.
void UIQueryDialogue::CreateChildren(GraphicsState* graphicsState)
{
	if (childrenCreated)
		return;

	/// Clear any children if we had any first.
	if (children.Size())
		children.ClearAndDelete();
	/// Create the "box"
	UIList * box = new UIList();
	box->layout.sizeRatioX = box->layout.sizeRatioY = 0.5f;
	box->layout.alignmentX = box->layout.alignmentY = 0.5f;
	box->visuals.textureSource = "80Gray50Alpha";
	AddChild(nullptr, box);

	/// Title
	UILabel * label = new UILabel();
	label->SetText(headerText);
	label->visuals.textureSource = "80Gray50Alpha";
	label->layout.sizeRatioY = 0.15f;
	box->AddChild(nullptr, label);

	/// Body
	label = new UILabel();
	label->SetText(textToPresent);
	label->layout.sizeRatioX = 0.3f;
	label->layout.sizeRatioY = 0.4f;
	box->AddChild(nullptr, label);

	UIColumnList * cList = new UIColumnList();
	cList->layout.sizeRatioY = 0.2f;
	box->AddChild(nullptr, cList);

	/// Cancel/Decline-button
	UIButton * button;
	button = new UIButton("CancelButton");
	button->SetText("Cancel");
	button->layout.sizeRatioX = 0.5f;
	if (popUponContinuing)
		button->activationMessage += "&PopUI("+this->name+")&";
	button->activationMessage += actionToBeTakenIfDeclining;
	cList->AddChild(nullptr, button);
	cancelButton = button;

	/// OK/Continue-button
	button = new UIButton("OKButton");
	button->SetText("OK");
	button->layout.sizeRatioX = 0.5f;
	if (popUponContinuing)
		button->activationMessage += "&PopUI("+this->name+")&";
	button->activationMessage += actionToBeTakenIfProceeding;
	cList->AddChild(nullptr, button);
	okButton = button;

	childrenCreated = true;
}




/// Used to give unique IDs to each dialogue
int UIStringDialogue::dialoguesCreated = 0;

UIStringDialogue::	UIStringDialogue(String queryHeader, String actionToBeTakenIfProceeding, String textToPresent, String initialText)
: UIElement(), textToPresent(textToPresent), initialText(initialText)
{
	type = UIType::QUERY_DIALOGUE;
	this->headerText = queryHeader;
	this->actionToBeTakenIfProceeding = actionToBeTakenIfProceeding;
	this->actionToBeTakenIfDeclining = actionToBeTakenIfDeclining;
	modal = true;
	/// Set modalitity using hoverable property, and disable the highlight so the user doesn't get confused (or the bg will flash when moving the cursor)
	this->interaction.hoverable = true;
	interaction.navigatable = true;
	visuals.highlightOnHover = false;
	/// Disable any automatic popping, so recommend adding it as an actionToBeTakenIfProceeding?
	this->interaction.exitable = false;
	/// If modal, set a texture for it too?
	visuals.textureSource = "black50Alpha";
	/// Set ID and name
	this->name = "UIQueryDialogue_"+String::ToString(this->id);


	/// Default "Are you sure? Any unsaved data may be lost."
	this->textToPresent = textToPresent;

	/// Boolean on whether this dialogue should be popped from the UI  after it's usage. Default is true.
	popUponContinuing = true;
	/// Boolean on whether this dialogue should be deleted after it's usage. Default is true.
	deleteUponContinuing = false;
	removeOnPop = true;
	input = NULL;


	/// Wether NavigateUI should be enabled when this element is pushed.
	interaction.navigateUIOnPush = true;
	interaction.disableNavigateUIOnPop = false;
	/// If force navigate UI should be applied for this element.
	interaction.forceNavigateUI = true;
}

UIStringDialogue::~UIStringDialogue()
{
	std::cout<<"\nUIStringDialogue destructor";
}

/// Creates the relevant children. Separate function in order to not have everything allocated in the constructor.
void UIStringDialogue::CreateChildren(GraphicsState* graphicsState)
{
	if (childrenCreated)
		return;

	/// Clear any children if we had any first.
	if (children.Size())
		children.ClearAndDelete();
	/// Create the "box"
	UIList * box = new UIList();
	box->layout.sizeRatioX = box->layout.sizeRatioY = 0.5f;
	box->layout.alignmentX = box->layout.alignmentY = 0.5f;
	box->visuals.textureSource = "80Gray50Alpha";
	box->layout.padding = 0.01f;
	AddChild(nullptr, box);

	/// Title
	UILabel * label = new UILabel();
	label->SetText(headerText);
	label->visuals.textureSource = "80Gray50Alpha";
	label->layout.sizeRatioY = 0.15f;
	box->AddChild(nullptr, label);

	/// Body
	label = new UILabel();
	label->SetText(textToPresent);
	label->layout.sizeRatioX = 0.3f;
	label->layout.sizeRatioY = 0.4f;
	box->AddChild(nullptr, label);

	/// Add the input.
	input = new UIInput("StringInput");
	input->SetText(initialText);
	input->layout.sizeRatioY = 0.2f;
	box->AddChild(nullptr, input);

	/// And the ok-button.
	UIElement * okButton = new UIButton("OK");
	okButton->activationMessage = "UIProceed("+this->name+")";
	okButton->layout.sizeRatioY = 0.2f;
	okButton->layout.sizeRatioX = 0.4f;
	okButton->layout.alignmentX = 0.2f;
	box->AddChild(nullptr, okButton);
	childrenCreated = true;
}

/// Callback from the input-system. Will submit a SetString message and remove this Dialogue.
void UIStringDialogue::Proceed()
{
	// Post message with the string.
	SetStringMessage * msg = new SetStringMessage(actionToBeTakenIfProceeding, input->GetText());
	MesMan.QueueMessage(msg);

	// Queue removal of the dialogue.
	if (popUponContinuing)
		MesMan.QueueMessages("PopUI("+this->name+")");
}




