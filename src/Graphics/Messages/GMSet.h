// Emil Hedemalm
// 2013-08-03

#ifndef GM_SET_H
#define GM_SET_H

#include "GraphicsMessage.h"
#include "GraphicsMessages.h"

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
	GMSetGlobalUI(UserInterface * ui);
	void Process();
private:
	UserInterface * ui;
};

/// For setting UI to be rendered.
class GMSetUI : public GraphicsMessage {
public:
	// If viewport is unspecified (-1) the global UI will be swapped.
	GMSetUI(UserInterface * ui, int viewport = -1);
	void Process();
private:
	int viewport;
	UserInterface * ui;
};

class GMSetOverlay : public GraphicsMessage {
public:
	// Fade-time in milliseconds.
	GMSetOverlay(String textureName, int fadeTime = 0);
	void Process();
private:
	String textureName;
	int fadeInTime;
};


#endif