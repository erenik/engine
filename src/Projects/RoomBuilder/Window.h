/// Emil Hedemalm
/// 2015-05-10
/// A window.

#ifndef WINDOW_H
#define WINDOW_H

class Window : public RoomObject
{
public:
	Window();
	virtual ~Window();

	/// Call after changing any contents inside and it will re-create the entire room for display.
	void Create();
	
private:

};

#endif
