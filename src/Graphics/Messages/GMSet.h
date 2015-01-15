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
	GMSet(int target, Vector3f vec3fValue);
	GMSet(int target, float floatValue);
	void Process();
private:
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

/// For setting system-global UI. 
class GMSetGlobalUI : public GraphicsMessage 
{
public:
	GMSetGlobalUI(UserInterface * ui, Window * forWindow = NULL);
	void Process();
private:
	UserInterface * ui;
	Window * window;
};

/// For setting UI to be rendered.
class GMSetUI : public GraphicsMessage {
public:
	/// Regular UI setter for the main window (Assumes 1 main window)
	GMSetUI(UserInterface * ui);
	// Regular UI setter per window.
	GMSetUI(UserInterface * ui, Window * forWindow);
	// For setting viewport-specific windows (e.g. old localhost multiplayer games).
	GMSetUI(UserInterface * ui, Viewport * viewport);
	void Process();
private:
	Window * window;
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