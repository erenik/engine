/// Emil Hedemalm
/// 2015-11-30
/// Actions for UI. Meant to be used primarily on graphics/UI-thread.

#ifndef UI_ACTION_H
#define UI_ACTION_H

class UIElement;

class UIAction
{
public:
	UIAction();
	UIAction(int type, int target);
	UIAction(int type, UIElement * targetElement);
	UIAction(int type, UIElement * targetElement, int argument);
	void Nullify();
	void Process(UIElement * forElement);
	enum 
	{
		STRING,
		OPEN_DROP_DOWN_MENU,
		CLOSE_DROP_DOWN_MENU,
		SELECT_DROP_DOWN_MENU,
	};
	int type;
	enum 
	{
		SELF,
		PARENT,
	};
	int target;
	int argument;
	UIElement * targetElement;
//	String msg;
};



#endif
