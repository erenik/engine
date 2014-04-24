/// Emil Hedemalm
/// 2014-04-23
/// Some general scripts here.

#include "GeneralScripts.h"
#include "StateManager.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"

StateChanger::StateChanger(String line, Script * parent)
: Script(line, parent)
{
}
/// Regular state-machine mechanics for the events, since there might be several parralell events?
void StateChanger::OnBegin()
{
	String stateName = name.Tokenize("()")[1];
	// Queue the state.
	gs = StateMan.GetStateByName(stateName);
	if (!gs)
		std::cout<<"\nERROR: There is no such game state: "<<stateName;
	StateMan.QueueState(gs);
}
void StateChanger::Process(float time)
{
	if (gs == NULL)
	{
		scriptState = Script::ENDING;
		return;
	}
	// Wait until the state has been entered too.
	GameState * activeState = StateMan.ActiveState();
	if (activeState == gs)
		scriptState = Script::ENDING;
}

OverlayFadeEffect::OverlayFadeEffect(String line, Script * parent)
: Script(line, parent)
{
	// Nullify stuff for security reasons.
	fadeInDuration = fadeOutDuration = duration = 0;
	List<String> args = line.Tokenize("(,)");
	switch(args.Size())
	{
	default:
		if (args.Size() < 0)
			return;
	case 6:
		fadeOutDuration = (int)args[5].ParseFloat();
	case 5:
		fadeOutTextureSource = args[4];
	case 4:
		duration = (int)args[3].ParseFloat();
	case 3:
		fadeInDuration = (int)args[2].ParseFloat();
	case 2:
		fadeInTextureSource = args[1];
	case 1:
		// Function-name here.
		break;
	}
	fadeInTexture = fadeOutTexture = NULL;
	// 0 = sent first texture, 1 = sent second texture.
	scriptState = -1;
}
void OverlayFadeEffect::OnBegin()
{
	// Set start-time to current time.
	startTime = Timer::GetCurrentTimeMs();
	// Send a graphics-message to start blending the 
	Graphics.QueueMessage(new GMSetOverlay(fadeInTextureSource, fadeInDuration));
	scriptState = 0;
}

void OverlayFadeEffect::Process(float time)
{
	// If we have a texture to fade-out to, check that.
	long long currentTime = Timer::GetCurrentTimeMs();
	// Check time.
	int timePassed = (int) (currentTime - startTime);
	if (fadeOutDuration > 0 && 
		fadeState < 1 &&
		fadeInTextureSource.Length()
		)
	{
		if (timePassed > fadeInDuration + duration)
		{
			// Send the fade-out message.
			Graphics.QueueMessage(new GMSetOverlay(fadeOutTextureSource, fadeInDuration));
			fadeState = 1;
		}
	}
	long long totalTime = fadeInDuration + duration + fadeOutDuration;
	if (timePassed > totalTime)
	{
		scriptState = Script::ENDING;	
	}
}



