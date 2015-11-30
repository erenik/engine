// Emil Hedemalm
// 2013-06-17

#ifndef GMUI_H
#define GMUI_H

#include "GraphicsMessage.h"
#include "GraphicsMessages.h"
#include <String/Text.h>

class Viewport;
class UserInterface;

class GMUI : public GraphicsMessage{
public:
	/// Default constructor, will target active global UI.
	GMUI(int messageType);
	/// Default constructor.
	GMUI(int messageType, UserInterface * targetUI);
	/// Default constructor.
	GMUI(int messageType, Viewport * viewport);
	void Nullify();
	virtual void Process() = 0;

	enum gmUITargets {
		NULL_TARGET,
		/// Pointer-targets
		TEXTURE,
		/// Lists of booleans
		MATRIX_DATA,
		// Vectors
		COLOR,
		TEXT_COLOR,
		VECTOR_INPUT, // Whole input for UIVectorInput class.
		/// Vector2i
		MATRIX_SIZE,
		/// Integers,
		INTEGER_INPUT, // Used for RadioButtons as well as IntegerInput
		// Floats
		SCROLL_POSITION_Y,
		FLOAT_INPUT, // Single nested input for UIFloatInput class.
		TEXT_SIZE_RATIO, // Yup.
		ALPHA, // Assuming color only varies by alpha...
		TEXT_ALPHA,
		// Bools
		VISIBILITY,
		CHILD_VISIBILITY, // Used to specify the visibility of all children via GMSetUIb instead of having an own message-class.
		ACTIVATABLE,
		HOVERABLE,
		TOGGLED,
		CHILD_TOGGLED, // Used to specify the toggled-parameter of all children via GMSetUIb. Function: together with a TOGGLED message to change the active button in a .. button list.
		ENABLED, // For temporarily enabling/disabling things like buttons.
		// Strings
		TEXT,
		TEXTURE_SOURCE,
		STRING_INPUT_TEXT, // For UIStringInput
		INTEGER_INPUT_TEXT, // For UIIntegerInput
		TEXTURE_INPUT_SOURCE, // For UITextureInput
	};
protected:
	/// By looking at viewport. Returns false if none was found.
	bool GetUI();
	/// Fetches global UI. 
	bool GetGlobalUI();
	Viewport * viewport;
	AppWindow * window;
	UserInterface * ui;
	/// If targeting the global UI.
	bool global;
	bool retryOnFailure;
};

class GMSetHoverUI : public GMUI 
{
public:
	GMSetHoverUI(String uiName, UserInterface * inUI = NULL);
	virtual void Process();
private:
	String name;
};

/// Used to set pointers. Mainly textures or some custom data.
class GMSetUIp : public GMUI{
public:
	GMSetUIp(String uiName, int target, Texture * tex, UserInterface * ui);
	void Process();
private:
	String name;
	int target;
	Texture * texture;
};


/// Used to set arbitrary amounts of booleans. Mainly used for binary matrices (UIMatrix).
class GMSetUIvb : public GMUI{
public:
	GMSetUIvb(String uiName, int target, List<bool*> boolData, Viewport * viewport = NULL);
	void Process();
private:
	String name;
	int target;
	List<bool*> data;
};

class GMSetUIi : public GMUI 
{
public:
	GMSetUIi(String uiName, int target, int value, UserInterface * ui = NULL);
	void Process();
private:
	String uiName;
	int target;
	int value;
};

/// Used to set UI vector2i data. Primarily used to specify size of matrix or maybe later aboslute-size of an element.
class GMSetUIv2i : public GMUI {
public:
	GMSetUIv2i(String UIname, int target, Vector2i v, Viewport * viewport = NULL);
	GMSetUIv2i(String UIname, int target, Vector2i v, UserInterface * targetUI);
	void Process();
private:
	String name;
	int target;
	UIElement * element;
	Vector2i value;
};

/// Used to set Vector-based data, like input for vector-input UI or RGB colors.
class GMSetUIv3f : public GMUI {
public:
	GMSetUIv3f(String UIname, int target, const Vector3f & v, Viewport * viewport = NULL);
	GMSetUIv3f(String uiName, int target, const Vector3f & v, UserInterface * ui);
	void Process();
private:
	void AssertTarget();
	String name;
	int target;
	UIElement * element;
	Vector3f value;
};


/// Used to set Vector-based data, mainly colors.
class GMSetUIv4f : public GMUI {
public:
	GMSetUIv4f(String UIname, int target, const Vector4f & v, UserInterface * ui);
	GMSetUIv4f(String UIname, int target, const Vector4f & v, Viewport * viewport = NULL);
	void Process();
private:
	void AssertTarget();
	String name;
	int target;
	UIElement * element;
	Vector4f value;
};

/// For setting floating point values, like relative sizes/positions, scales etc.
class GMSetUIf : public GMUI {
public:
	/// Targets global UI
	GMSetUIf(String UIname, int target, float value);
	GMSetUIf(String UIname, int target, float value, UserInterface * inUI);
	GMSetUIf(String UIname, int target, float value, Viewport * viewport);
	void Process();
private:
	void AssertTarget();
	String name;
	int target;
	UIElement * element;
	float value;
};

/** For setting floating point values, like relative sizes/positions, scales etc. 
	of elements in the system-global UI. Makes used of the regular GMSetUIf class for processing.
*/
class GMSetGlobalUIf : public GMSetUIf 
{
public:
	GMSetGlobalUIf(String uiName, int target, float value, AppWindow * window = NULL);
};



class GMSetUIb : public GMUI {
public:
	GMSetUIb(String UIname, int target, bool v, UserInterface * inUI);
	GMSetUIb(String UIname, int target, bool v, Viewport * viewport = NULL);
	void Process();
private:
	void AssertTarget();
	String name;
	int target;
	UIElement * element;
	bool value;
};

class GMSetUIs: public GMUI 
{
public:
	// Targets the main windows ui.
	GMSetUIs(String uiName, int target, Text text);
	GMSetUIs(String uiName, int target, Text text, UserInterface * inUI);
	GMSetUIs(String uiName, int target, Text text, bool force, UserInterface * inUI);
	GMSetUIs(String uiName, int target, Text text, Viewport * viewport);
	GMSetUIs(String uiName, int target, Text text, bool force, Viewport * viewport = NULL);
	/** Explicitly declared constructor to avoid memory leaks.
		No explicit constructor may skip subclassed variable deallocation!
	*/
	~GMSetUIs();
	void Process();
private:
	void AssertTarget();
	int target;
	String uiName;
	Text text;
	bool force;
};

// System-global version for setting strings of ui.
class GMSetGlobalUIs : public GMSetUIs
{
public:
	/// Force default should be false.
	GMSetGlobalUIs(String uiName, int target, Text text, bool force = false, AppWindow * window = NULL);
};


// Deletes contents (children) of specified UI element. Primarily used on UILists.
class GMClearUI : public GMUI{
public:
	GMClearUI(List<String> uiNames);
	GMClearUI(String uiName, UserInterface * inUI);
	GMClearUI(String uiName, Viewport * viewport = NULL);
	void Process();
private:
	List<String> uiNames;
};

class GMScrollUI : public GMUI{
public:
    GMScrollUI(String uiName, float scrollDistance, Viewport * viewport = NULL);
    void Process();
private:
    String uiName;
    float scrollDistance;
};


/// Message to add a newly created UI to the global state's UI, mostly used for overlay-effects and handling error-messages.
class GMAddGlobalUI : public GMUI 
{
public:
	GMAddGlobalUI(UIElement * element, String toParent = "root");
	void Process();
private:
	UIElement * element;
	String parentName;

};

/// Message to add a newly created UI to the active game state's UI.
class GMAddUI : public GMUI{
public:
	GMAddUI(List<UIElement*> elements, String toParent, UserInterface * inUI);
	GMAddUI(List<UIElement*> elements, String toParent = "root", Viewport * viewport = NULL);
	void Process();
private:
	List<UIElement*> elements;
	String parentName;
};

class GMPushUI : public GMUI{
public:
	GMPushUI(String elementName, UserInterface * ontoUI);
	GMPushUI(UIElement * element, UserInterface * ontoUI);
	void Process();
private:
	UIElement * element;
	String uiName;
};

class GMPopUI : public GMUI{
public:
	/// If force is specified, it will pop the UI no matter what it's exitable property says.
	GMPopUI(String uiName, UserInterface * ui, bool force = false, Viewport * viewport = NULL);
	void Process();
private:
	UIElement * element;
	String uiName;
	bool force;
};
/*
/// Unbuffers and deletes target element.
class GMDeleteUI : public GMUI{
public:
    GMDeleteUI(UIElement * element);
    void Process();
private:
    UIElement * element;
};
*/
class GMRemoveUI : public GMUI{
public:
	GMRemoveUI(UIElement * element);
	void Process();
private:
	UIElement * element;
};


/// Used to set contents in e.g. a UIMatrix element.
class GMSetUIContents : public GMUI 
{
public:
	/// Used for setting String-list in Drop-down menus
	GMSetUIContents(String uiName, List<String> contents);
	GMSetUIContents(UserInterface * ui, String uiName, List<String> contents);
	/// Used for setting elements in Matrix.
	GMSetUIContents(List<UIElement*> elements, String uiName);
	void Process();
private:
	String uiName;
	List<String> contents;
	List<UIElement*> elements;
};

#endif
