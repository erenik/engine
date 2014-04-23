/// Emil Hedemalm
/// 2014-01-07
/// General events manipulating text-strings in UI elements!

#ifndef TEXT_ANIMATION_EVENT_H
#define TEXT_ANIMATION_EVENT_H

#include "Script.h"
class UIElement;
class Viewport;

class TextAnimationEvent : public Script {
public:
	TextAnimationEvent(int type, String elementName, int viewport);
	enum types {
		NULL_TYPE,
		NOTICE,	/// A brief notice that appears, lingers for a few seconds before disappearing. Only minor fading should be used if at all.
	};

	virtual void OnBegin();
	virtual void Process(float time);
	virtual void OnEnd();

	/// Yup.
	int fadeInDuration, fadeOutDuration;
	/// Duration in milliseconds.
	int duration;
	/// Type of event, since this is a general class (or will become, if there are many variations).
	int type;
private:
	/// Total duration of this event, including fade-ins.
	int totalDuration;
	/// Yeah.
	long long startTime;
	/// Reference name to the element.
	String elementName;
	/// Viewport where the element belongs, might be required for the messages to be sent.
	int viewport;
	/// Pointer if needed... shouldn't be needed.
	UIElement * element;
};

#endif

