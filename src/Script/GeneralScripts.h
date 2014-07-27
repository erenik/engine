/// Emil Hedemalm
/// 2014-04-23
/// Some general scripts here.

#ifndef GENERAL_SCRIPTS_H
#define GENERAL_SCRIPTS_H

#include "Script.h"
class Texture;

class WaitScript : public Script 
{
public:
	WaitScript(String line, Script * parent);
	virtual void OnBegin();
	virtual void Process(int timeInMs);
	virtual void OnEnd();
private:
	int duration;
	int timeWaitedSoFar;
};

// Custom scripts here! Or new files for that later?
class StateChanger : public Script 
{
public:
	StateChanger(String line, Script * parent);
	/// Regular state-machine mechanics for the events, since there might be several parralell events?
	virtual void OnBegin();
	virtual void Process(int timeInMs);
	virtual void OnEnd();
private:
	AppState * gs;
};

class UIElement;

namespace FadeState{
enum fadeState 
{
	FADING_IN,
	RESTING,
	FADING_OUT,
	DONE,
};
};



/// Base class for fade-effects using the System-global ui.
class OverlayScript : public Script 
{
public:
	OverlayScript(String line, Script * parent);
	virtual void OnBegin();
	virtual void Process(int timeInMs);
	virtual void OnEnd();
	
protected:
	// For accessing the ui-elements.
	void SetLayerAlpha(int layer, float alpha);
	float GetLayerAlpha(int layer);
	// Set main text.
	void SetText(String text);

	// Some variables to use.
	int fadeState;
	long long startTime, totalTime;
	int duration;
	
	// For handling overlay-layer effects.
	enum overlayLayerNames{
		BACKGROUND,
		LAYER_1,
		LAYER_2,
		TEXT, // Separate layer to contain text.
		MAX_OVERLAY_LAYERS,
	};
private:
	static int overlayLayers;
	// Elements used for the overlay-fade effect. Placed in the global UI.
	static UIElement * overlayUI[MAX_OVERLAY_LAYERS];
	/** 0 = sent first texture, fading it, 
		1 = resting at full alpha, 
		2 = sent second texture, fading out, 
		3 = done.
	*/
};

/// Fade-operation on the background behind the overlay image and texts.
class FadeInBackground : public OverlayScript 
{
public:
	FadeInBackground(String line, Script * parent);
	virtual void OnBegin();
	virtual void Process(int timeInMs);
	virtual void OnEnd();
	
private:
	String fadeInTextureSource;
	int fadeInDuration;
};

class FadeOutBackground : public OverlayScript 
{
public:
	FadeOutBackground(String line, Script * parent);
	virtual void OnBegin();
	virtual void Process(int timeInMs);
	virtual void OnEnd();
	
private:
	int fadeOutDuration;
	float startAlpha;
};

class FadeInEffect : public OverlayScript
{
public:
	FadeInEffect(String line, Script * parent);
	virtual void OnBegin();
	virtual void Process(int timeInMs);
	virtual void OnEnd();
	
private:
	String fadeInTextureSource;
	int fadeInDuration;
	
};

class FadeOutEffect : public OverlayScript
{
public:
	FadeOutEffect(String line, Script * parent);
	virtual void OnBegin();
	virtual void Process(int timeInMs);
	virtual void OnEnd();
	
private:
	// Take note of alpha when starting fade-out.
	float startAlpha;
	int fadeOutDuration;
};

class FadeTextEffect : public OverlayScript
{
public:
	FadeTextEffect(String line, Script * parent);
	virtual void OnBegin();
	virtual void Process(int timeInMs);
	virtual void OnEnd();
	
private:
	int fadeInDuration;
	int fadeOutDuration;
	String text;
};


#endif

