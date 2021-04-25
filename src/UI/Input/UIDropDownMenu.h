/// Emil Hedemalm
/// 2015-11-30
/// Drop-down menu. Single-selection for starters.

#ifndef UI_DROP_DOWN_MENU_H
#define UI_DROP_DOWN_MENU_H

#include "UI/Input/UIInput.h"
class UIList;

// I mix them...
#define UIDropDownList UIDropDownMenu 

class UIDropDownMenu : public UIInput
{
public:
	UIDropDownMenu(String name);
	virtual ~UIDropDownMenu();

	void CreateChildren(GraphicsState* graphicsState);
	/// Sets text, queueing recalculation of the rendered variant. If not force, will ignore for active ui input elements.
	virtual void SetText(CTextr newText, bool force = false);
	virtual void RenderText();
	/// Call from graphics thread only. Or set via GMSetUIContents
	virtual void SetContents(GraphicsState* graphicsState, List<String> contents);

	virtual bool OnProceed(GraphicsState* graphicsState) override;
	/// Opens UI to select among children. How this will be done depends on settings.
	virtual void Open(GraphicsState* graphicsState);
	virtual void PopulateList(GraphicsState* graphicsState);
	virtual void Close(GraphicsState* graphicsState);
	// Selects the specified entry. If silent is specified, no messages will be generated (e.g. game to UI)
	virtual void Select(int index, bool silent = false);
	// Attempts to select the specified entry. If silent is specified, no messages will be generated (e.g. game to UI)
	bool Select(String entry, bool silent = false);

	// For sub-classes to adjust children as needed (mainly for input elements).
	virtual void OnStateAdded(GraphicsState* graphicsState, int state) override;

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