/// Emil Hedemalm
/// 2015-10-04
/// UI and logic updates for script-editing/loading/saving/selection.

#include "SpaceShooter2D.h"
#include "Base/WeaponScript.h"
#include "UI/UIButtons.h" 
#include "UI/UIList.h"

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
	ScriptAction & action = editScript->actions[editIndex];
	QueueGraphics(new GMSetUIs("ActionName", GMUI::STRING_INPUT_TEXT, action.name));

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
}

void ProcessMessageWSS(Message * message)
{
	String msg = message->msg;
	int type = message->type;
	switch(type)
	{
		case MessageType::STRING:
		{
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
			}
			else if (msg.StartsWith("EditScriptAction:"))
			{
				editIndex = msg.Tokenize(":")[1].ParseInt();
				UpdateEditActionUI();
			}
		}
	}
}