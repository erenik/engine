/// Emil Hedemalm
/// 2014-01-02
/// UI subclass that can create a (modal) dialogue to demand user interaction before continuing.

#include "UITypes.h"
#include "UIQueryDialogue.h"
#include "UIList.h"
#include "UIButtons.h"

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
	this->hoverable = true;
	this->highlightOnHover = false;
	/// Disable any automatic popping, so recommend adding it as an actionToBeTakenIfProceeding?
	this->exitable = false;
	/// If modal, set a texture for it too?
	this->textureSource = "black50Alpha";
	/// Set ID and name
	this->id = dialoguesCreated++;
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
	navigateUIOnPush = true;
	/// If force navigate UI should be applied for this element.
	forceNavigateUI = true;
}

UIQueryDialogue::~UIQueryDialogue()
{
	std::cout<<"\nUIQueryDialogue destructor";
}

/// Creates the relevant children. Separate function in order to not have everything allocated in the constructor.
void UIQueryDialogue::CreateChildren(){
	/// Clear any children if we had any first.
	if (childList.Size())
		childList.ClearAndDelete();
	/// Create the "box"
	UIList * box = new UIList();
	box->sizeRatioX = box->sizeRatioY = 0.5f;
	box->alignmentX = box->alignmentY = 0.5f;
	box->textureSource = "80Gray50Alpha";
	AddChild(box);

	/// Title
	UILabel * label = new UILabel();
	label->text = headerText;
	label->textureSource = "80Gray50Alpha";
	label->sizeRatioY = 0.15f;
	box->AddChild(label);

	/// Body
	label = new UILabel();
	label->text = textToPresent;
	label->textSizeRatio = 0.3f;
	label->sizeRatioY = 0.4f;
	box->AddChild(label);

	UIColumnList * cList = new UIColumnList();
	cList->sizeRatioY = 0.2f;
	box->AddChild(cList);

	/// Cancel/Decline-button
	UIButton * button;
	button = new UIButton("CancelButton");
	button->text = "Cancel";
	button->sizeRatioX = 0.5f;
	if (popUponContinuing)
		button->activationMessage += "&PopUI("+this->name+")&";
	button->activationMessage += actionToBeTakenIfDeclining;
	cList->AddChild(button);
	cancelButton = button;

	/// OK/Continue-button
	button = new UIButton("OKButton");
	button->text = "OK";
	button->sizeRatioX = 0.5f;
	if (popUponContinuing)
		button->activationMessage += "&PopUI("+this->name+")&";
	button->activationMessage += actionToBeTakenIfProceeding;
	cList->AddChild(button);
	okButton = button;


}
