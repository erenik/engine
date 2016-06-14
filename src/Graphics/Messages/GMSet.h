// Emil Hedemalm
// 2013-08-03

#ifndef GM_SET_H
#define GM_SET_H

#include "GraphicsMessage.h"
#include "GraphicsMessages.h"

class Viewport;

/** General utility Setter "function" message for the rendering pipeline,
	like overlay images, default values and debug renders.
*/
class GMSet : public GraphicsMessage {
public:
	GMSet(int target, void *pData);
	GMSet(int target, const Vector3f & vec3fValue);
	GMSet(int target, float floatValue);
	GMSet(int target, bool bValue);
	void Process();
private:
	bool bValue;
	float floatValue;
	int target;
	Vector3f vec3fValue;
	void * pData;
};

class GMSeti : public GraphicsMessage {
public:
	GMSeti(int target, int iValue);
	virtual void Process();
private:
	int target;
	int iValue;
};

class GMSetf : public GraphicsMessage {
public:
	GMSetf(int target, float value);
	void Process();
private:
	float floatValue;
	int target;
};

/** General utility Setter "function" message for the rendering pipeline,
	like overlay images, default values and debug renders.
*/
class GMSets: public GraphicsMessage {
public:
	GMSets(int target, String str);
	void Process();
private:
	String str;
	int target;
};

/** For setting arbirary data to be rendered, in order to ensure safe setting and adjustment 
	for rendering in e.g. State-specific render functions.. */
class GMSetData : public GraphicsMessage 
{
public:
	GMSetData(List<Vector3f> * targetList, List<Vector3f> newData);
	void Process();
private:
	List<Vector3f> * targetList, newData;
};

/// For setting system-global UI. 
class GMSetGlobalUI : public GraphicsMessage 
{
public:
	GMSetGlobalUI(UserInterface * ui, AppWindow * forWindow = NULL);
	void Process();
private:
	UserInterface * ui;
	AppWindow * window;
};

/// For setting UI to be rendered.
class GMSetUI : public GraphicsMessage {
public:
	/// Regular UI setter for the main AppWindow (Assumes 1 main AppWindow)
	GMSetUI(UserInterface * ui);
	// Regular UI setter per AppWindow.
	GMSetUI(UserInterface * ui, AppWindow * forWindow);
	// For setting viewport-specific windows (e.g. old localhost multiplayer games).
	GMSetUI(UserInterface * ui, Viewport * viewport);
	void Process();
private:
	AppWindow * window;
	Viewport * viewport;
	UserInterface * ui;
};

class GMSetOverlay : public GraphicsMessage {
public:
	// Fade-time in milliseconds.
	GMSetOverlay(String textureName, int fadeInTimeInMs = 0);
	GMSetOverlay(Texture * tex, int fadeInTimeInMs = 0);
	void Process();
private:
	Texture * tex;
	String textureName;
	int fadeInTimeInMs;
};


#endif