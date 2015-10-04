/// Emil Hedemalm
/// 2015-10-04
/// UI and logic updates for script-editing/loading/saving/selection.

#include "SpaceShooter2D.h"
#include "Base/WeaponScript.h"
#include "UI/UIButtons.h" 
#include "UI/UILists.h"
#include "UI/UIInputs.h"
#include "Message/MathMessage.h"

List<UIElement*> tmp;
String existingScripts = "lScripts",
	editField = "lEdit",
	optionsToAdd = "lToAdd";
// QueueGraphics(new GMClearUI(List<String>("lScripts", "lEdit", "lToAdd")));

WeaponScript * editScript = NULL;
int editIndex = -1;

void UpdateScriptsUI()
{
	QueueGraphics(new GMClearUI(existingScripts));
	// Existing scripts.
	for (int i = 0; i < weaponScripts.Size(); ++i)
	{
		// Add 'em.
		// ...
		UIButton * button = new UIButton("LoadScript: "+String(i));
		button->text = weaponScripts[i]->name;
		button->sizeRatioY = 0.15f;
		tmp.AddItem(button);
	}
	QueueGraphics(new GMAddUI(tmp, "lScripts"));
	tmp.Clear();
}
void UpdateEditScriptUI()
{
	QueueGraphics(new GMClearUI(editField));
	// Fetch active script?
	if (editScript)
	{
		/// Default equip it to the ship too?
		playerShip->weaponScript = editScript;

		QueueGraphics(new GMSetUIs("ScriptName", GMUI::STRING_INPUT_TEXT, editScript->name));
		for (int i = 0; i < editScript->actions.Size(); ++i)
		{
			// Add 'em.
			ScriptAction & action = editScript->actions[i];
			// ...
			UIButton * cl = new UIButton("EditScriptAction:"+String(i));
			cl->sizeRatioY = 0.1f;
			cl->text = String(i+1)+". "+(action.name.Length()? action.name : ScriptAction::GetStringForType(action.type));
			tmp.AddItem(cl);
		}
		/// Add some extra options.
		UIButton * c2 = new UIButton("DuplicateScript");
		c2->sizeRatioY = 0.1f;
		c2->text = "Duplicate script";
		tmp.AddItem(c2);
		c2 = new UIButton("DeleteScript");
		c2->sizeRatioY = 0.1f;
		c2->text = "Delete script";
		tmp.AddItem(c2);
	}
	else 
	{
		UILabel * ul = new UILabel("No script active. Select in left or add");
		ul->sizeRatioY = 0.1f;
		tmp.AddItem(ul);
		ul = new UILabel("a new part from the right to start a new one.");
		ul->sizeRatioY = 0.1f;
		tmp.AddItem(ul);
	}
	QueueGraphics(new GMAddUI(tmp, editField));
	tmp.Clear();
}

void UpdateEditActionUI()
{
	String s = "lEditAction";
	QueueGraphics(new GMClearUI(s));
	// Depending on type, add fields?
	if (!editScript)
		return;
	ScriptAction * action = 0;
	if (editIndex >= 0 && editIndex < editScript->actions.Size())
		action = &editScript->actions[editIndex];
	else {
		QueueGraphics(new GMSetUIs("ActionName", GMUI::STRING_INPUT_TEXT, ""));
		return;
	}

	QueueGraphics(new GMSetUIs("ActionName", GMUI::STRING_INPUT_TEXT, action->name));

	switch(action->type)
	{
		case ScriptAction::SWITCH_TO_WEAPON:
			UIIntegerInput * ii = new UIIntegerInput("Weapon type", "SetWeaponIndex");
			ii->sizeRatioY = 0.1f;
			ii->CreateChildren();
			ii->SetValue(action->weaponIndex);
			tmp.AddItem(ii);
			ii = new UIIntegerInput("Milliseconds", "SetMilliseconds");
			ii->sizeRatioY = 0.1f;
			ii->CreateChildren();
			ii->SetValue(action->durationMs);
			tmp.AddItem(ii);
			break;
	}

	// o.o
	QueueGraphics(new GMAddUI(tmp, s));
	tmp.Clear();
}

void UpdateToAddList()
{
	QueueGraphics(new GMClearUI(optionsToAdd));
	// Options to add, depends on what we define.
	for (int i = 0; i < ScriptAction::MAX_TYPES; ++i)
	{
		UIButton * button = new UIButton("AddScriptAction: "+String(i));
		button->text = ScriptAction::GetStringForType(i);
		button->sizeRatioY = 0.15f;
		button->textureSource = UIElement::defaultTextureSource;
		tmp.AddItem(button);
	}
	QueueGraphics(new GMAddUI(tmp, "lToAdd"));
	tmp.Clear();
}

void UpdateWeaponScriptUI()
{
	// Edit field, load current/first script?
	UpdateScriptsUI();
	UpdateEditScriptUI();
	UpdateToAddList();
	UpdateEditActionUI();
}

extern WeaponScript * lastEdited;

void ProcessMessageWSS(Message * message)
{
	String msg = message->msg;
	int type = message->type;
	// If weapon edit active 
	ScriptAction * sa = 0;
	if (editScript)
	{
		if (editIndex < editScript->actions.Size() && editIndex >= 0);
			sa = &editScript->actions[editIndex];
	}
	lastEdited = editScript;

	switch(type)
	{
		case MessageType::INTEGER_MESSAGE:
		{
			IntegerMessage * im = (IntegerMessage*) message;
			if (msg == "SetWeaponIndex")
			{
				if (sa)
					sa->weaponIndex = im->value;
			}
			else if (msg == "SetMilliseconds")
			{
				if (sa) sa->durationMs = im->value;
			}
			else 
			{
				std::cout<<"lalll";
			}
			break;
		}
		case MessageType::STRING:
		{
			if (msg.StartsWith("LoadScript:"))
			{
				String scriptIndex = msg.Tokenize(":")[1];
				scriptIndex.RemoveSurroundingWhitespaces();
				int index = scriptIndex.ParseInt();
				if (index >= weaponScripts.Size() || index < 0)
				{
					assert(false);
					return;
				}
				editScript = weaponScripts[index];
				UpdateEditScriptUI();
				editIndex = 0;
				UpdateEditActionUI();
			}
			if (msg == "DuplicateScript")
			{
				WeaponScript * script = new WeaponScript();
				*script = *editScript;
				weaponScripts.AddItem(script);
				editScript = script;
				UpdateWeaponScriptUI();
				// Hover to the same button upon addition into UI?
				QueueGraphics(new GMSetHoverUI("DuplicateScript"));
			}
			else if (msg == "DeleteScript")
			{
				WeaponScript * sc = editScript;
				weaponScripts.RemoveItem(sc);
				editScript = weaponScripts.Size()? weaponScripts[0] : 0;
				delete sc;
				UpdateWeaponScriptUI();
				if (weaponScripts.Size())
					QueueGraphics(new GMSetHoverUI("DeleteScript"));
			}
			if (msg.StartsWith("AddScriptAction:"))
			{
				if (!editScript)
				{
					editScript = new WeaponScript();
					weaponScripts.AddItem(editScript);
					UpdateScriptsUI();
				}
				String actionType = msg.Tokenize(":")[1];
				actionType.RemoveSurroundingWhitespaces();
				editScript->actions.AddItem(ScriptAction(actionType.ParseInt()));
				UpdateEditScriptUI();
				editIndex = editScript->actions.Size() - 1;
				UpdateEditActionUI();
			}
			else if (msg.StartsWith("EditScriptAction:"))
			{
				editIndex = msg.Tokenize(":")[1].ParseInt();
				UpdateEditActionUI();
			}
		}
	}
}