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
	/// Default constructor, if viewPortTarget is NULL it will target the active global UI.
	GMUI(int messageType, int viewportID = NULL);
	virtual void Process() = 0;

	enum gmUITargets {
		UI_NULL_TARGET,
		/// Lists of booleans
		MATRIX_DATA,
		// Vectors
		TEXT_COLOR,
		VECTOR_INPUT, // Whole input for UIVectorInput class.
		/// Vector2i
		MATRIX_SIZE,
		// Floats
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
	int viewportID;
	// Helps flag if the messages are directed to global or active-state ui.
	bool global;
	UserInterface * ui;
};


/// Used to set arbitrary amounts of booleans. Mainly used for binary matrices (UIMatrix).
class GMSetUIvb : public GMUI{
public:
	GMSetUIvb(String uiName, int target, List<bool*> boolData, int viewport = NULL);
	void Process();
private:
	String name;
	int target;
	List<bool*> data;
};

/// Used to set UI vector2i data. Primarily used to specify size of matrix or maybe later aboslute-size of an element.
class GMSetUIv2i : public GMUI {
public:
	GMSetUIv2i(String UIname, int target, Vector2i v, int viewport = NULL);
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
	GMSetUIv3f(String UIname, int target, Vector3f v, int viewport = NULL);
	void Process();
private:
	String name;
	int target;
	UIElement * element;
	Vector3f value;
};


/// Used to set Vector-based data, mainly colors.
class GMSetUIv4f : public GMUI {
public:
	GMSetUIv4f(String UIname, int target, Vector4f v, int viewport = NULL);
	void Process();
private:
	String name;
	int target;
	UIElement * element;
	Vector4f value;
};

/// For setting floating point values, like relative sizes/positions, scales etc.
class GMSetUIf : public GMUI {
public:
	GMSetUIf(String UIname, int target, float value, int viewport = NULL);
	void Process();
private:
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
	GMSetGlobalUIf(String uiName, int target, float value);
};



class GMSetUIb : public GMUI {
public:
	GMSetUIb(String UIname, int target, bool v, int viewport = NULL);
	void Process();
private:
	String name;
	int target;
	UIElement * element;
	bool value;
};

class GMSetUIs: public GMUI {
public:
	GMSetUIs(String uiName, int target, Text text, int viewport = NULL);
	GMSetUIs(String uiName, int target, Text text, bool force, int viewport = NULL);
	/** Explicitly declared constructor to avoid memory leaks.
		No explicit constructor may skip subclassed variable deallocation!
	*/
	~GMSetUIs();
	void Process();
private:
	int target;
	String uiName;
	Text text;
	bool force;
};

// System-global version for setting strings of ui.
class GMSetGlobalUIs : public GMSetUIs
{
public:
	GMSetGlobalUIs(String uiName, int target, Text text, bool force = false);
};


// Deletes contents (children) of specified UI element. Primarily used on UILists.
class GMClearUI : public GMUI{
public:
	GMClearUI(String uiName, int viewport = NULL);
	void Process();
private:
	String uiName;
};

class GMScrollUI : public GMUI{
public:
    GMScrollUI(String uiName, float scrollDistance, int viewport = NULL);
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
	GMAddUI(UIElement * element, String toParent = "root", int viewport = NULL);
	void Process();
private:
	UIElement * element;
	String parentName;
};

class GMPushUI : public GMUI{
public:
	GMPushUI(String elementName, UserInterface * ontoUI, int viewport = NULL);
	GMPushUI(UIElement * element, UserInterface * ontoUI, int viewport = NULL);
	void Process();
private:
	UIElement * element;
	String uiName;
};

class GMPopUI : public GMUI{
public:
	/// If force is specified, it will pop the UI no matter what it's exitable property says.
	GMPopUI(String uiName, UserInterface * ui, bool force = false, int viewport = NULL);
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


#endif
