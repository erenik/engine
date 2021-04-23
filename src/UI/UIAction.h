/// Emil Hedemalm
/// 2015-11-30
/// Actions for UI. Meant to be used primarily on graphics/UI-thread.

#ifndef UI_ACTION_H
#define UI_ACTION_H

#include "String/AEString.h"

class UIElement;
class GraphicsState;

class UIAction
{
public:
	UIAction();
	UIAction(int type, int target);
	UIAction(int type, UIElement * targetElement);
	UIAction(int type, UIElement * targetElement, int argument);
	UIAction(int type, UIElement * targetElement, String argument);
	void Nullify();
	void Process(GraphicsState* graphicsState, UIElement * forElement);
	enum 
	{
		POP_UI,
		STRING,
		// Drop down
		OPEN_DROP_DOWN_MENU,
		CLOSE_DROP_DOWN_MENU,
		SELECT_DROP_DOWN_MENU,
		// File Browser
		SELECT_FILE_BROWSER_FILE,
		SELECT_FILE_BROWSER_DIRECTORY,
		CONFIRM_FILE_BROWSER_SELECTION,
		SET_FILE_BROWSER_FILE_FROM_INPUT,
	};
	int type;
	enum 
	{
		SELF,
		PARENT,
	};
	int target;
	int argument;
	String argumentStr;
	UIElement * targetElement;
//	String msg;
};



#endif
