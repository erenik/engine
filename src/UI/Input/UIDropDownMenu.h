/// Emil Hedemalm
/// 2015-11-30
/// Drop-down menu. Single-selection for starters.

#ifndef UI_DROP_DOWN_MENU_H
#define UI_DROP_DOWN_MENU_H

#include "UI/UIElement.h"
class UIList;

// I mix them...
#define UIDropDownList UIDropDownMenu 

class UIDropDownMenu : public UIElement 
{
public:
	UIDropDownMenu(String name);
	virtual ~UIDropDownMenu();

	void CreateChildren();
	/// Sets text, queueing recalculation of the rendered variant. If not force, will ignore for active ui input elements.
	virtual void SetText(CTextr newText, bool force = false);
	virtual void RenderText();
	/// Call from graphics thread only. Or set via GMSetUIContents
	virtual void SetContents(List<String> contents);
	/// Opens UI to select among children. How this will be done depends on settings.
	virtual void Open();
	virtual void PopulateList();
	virtual void Close();
	virtual void Select(int index);

	/// Used on left side always.
	String labelText;
	/// Selected content.
	String selection;
	/// List of available content.
	List<String> available;
	/// Default true.
	bool separateLabel; 
private:
	UIElement * selectButton;
	UIList * selectionList;
};

#endif