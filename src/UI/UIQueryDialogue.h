/// Emil Hedemalm
/// 2014-01-02
/// UI subclass that can create a (modal) dialogue to demand user interaction before continuing.

#ifndef UI_QUERY_DIALOG
#define UI_QUERY_DIALOG

#include "UIElement.h"

class UIQueryDialogue : public UIElement {
public:
	/** Constructor for a simple dialogue with one confirmation button and one cancel button. 
		The default dialogue will close no matter what alternative is picked.
	*/
	UIQueryDialogue(String queryHeader, String actionToBeTakenIfProceeding, String actionToBeTakenIfDeclining = "");
	virtual ~UIQueryDialogue();
	/// Creates the relevant children. Separate function in order to not have everything allocated in the constructor.
	void CreateChildren();
	UIElement * OKButton() { return okButton; };
	UIElement * CancelButton() { return cancelButton; };

	/// Header/title-bar text. Usually set as an argument in the constructor.
	String headerText;
	/// Body of the dialogue. Default: "Are you sure you want to continue? Any unsaved data may be lost."
	String textToPresent;
	/// Specifies if the dialogue should demand input before the user may interact with other UI or not. Default is true.
	bool modal;
	/// Boolean on whether this dialogue should be popped from the UI  after it's usage. Default is true.
	bool popUponContinuing;
	/// Boolean on whether this dialogue should be deleted after it's usage. Default is true.
	bool deleteUponContinuing;
private:
	String actionToBeTakenIfProceeding;
	String actionToBeTakenIfDeclining;
	/// Used to give unique IDs to each dialogue
	static int dialoguesCreated;

	UIElement * okButton;
	UIElement * cancelButton;
};

/// Similar to the Query-dialogue, the String-dialogue opens up a single input in order to prompt the player for a text, e.g. name.
class UIStringDialogue : public UIElement {
public:
	/** Constructor for a simple dialogue with one confirmation button and one cancel button. 
		The default dialogue will close no matter what alternative is picked.
	*/
	UIStringDialogue(String queryHeader, String actionToBeTakenIfProceeding, String textToPresent, String initialText);
	virtual ~UIStringDialogue();
	/// Creates the relevant children. Separate function in order to not have everything allocated in the constructor.
	void CreateChildren();

	/// Callback from the input-system. Will submit a SetString message and remove this Dialogue.
	virtual void Proceed();

	/// Stuff.
	String GetString() { return input->text; };
	UIElement * StringInput() { return input; };

	/// Header/title-bar text. Usually set as an argument in the constructor.
	String headerText;
	/// Body of the dialogue. Default: "Are you sure you want to continue? Any unsaved data may be lost."
	String textToPresent;
	/// Initial contents of the input.
	String initialText;
	/// Specifies if the dialogue should demand input before the user may interact with other UI or not. Default is true.
	bool modal;
	/// Boolean on whether this dialogue should be popped from the UI  after it's usage. Default is true.
	bool popUponContinuing;
	/// Boolean on whether this dialogue should be deleted after it's usage. Default is true.
	bool deleteUponContinuing;
private:
	String actionToBeTakenIfProceeding;
	String actionToBeTakenIfDeclining;
	/// Used to give unique IDs to each dialogue
	static int dialoguesCreated;

	UIElement * input;
};


#endif