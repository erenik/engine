/// Emil Hedemalm
/// 2014-04-23
/// Some general scripts here.

#ifndef GENERAL_SCRIPTS_H
#define GENERAL_SCRIPTS_H

#include "Script.h"
class Texture;

// Custom scripts here! Or new files for that later?
class StateChanger : public Script 
{
public:
	StateChanger(String line, Script * parent);
	/// Regular state-machine mechanics for the events, since there might be several parralell events?
	virtual void OnBegin();
	virtual void Process(float time);
private:
	GameState * gs;
};


class OverlayFadeEffect : public Script 
{
public:
	OverlayFadeEffect(String line, Script * parent);
	virtual void OnBegin();
	virtual void Process(float time);
private:
	// 0 = sent first texture, 1 = sent second texture.
	int fadeState;
	long long startTime;
	String fadeInTextureSource;
	String fadeOutTextureSource;
	Texture * fadeInTexture, * fadeOutTexture;
	int fadeInDuration;
	int duration;
	int fadeOutDuration;
};

#endif

