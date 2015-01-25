/// Emil Hedemalm
/// 2014-04-23
/// Some general scripts here.

#include "GeneralScripts.h"
#include "StateManager.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMUI.h"
#include "UI/UIImage.h"
#include "Window/WindowManager.h"

WaitScript::WaitScript(String line, Script * parent)
: Script(line, parent)
{
	duration = line.Tokenize("()")[1].ParseInt();
}
void WaitScript::OnBegin()
{
	timeWaitedSoFar = 0;
}
void WaitScript::Process(int timeInMs)
{
	timeWaitedSoFar += timeInMs;
	if (timeWaitedSoFar > duration)
	{
		scriptState = Script::ENDING;
	}
}
void WaitScript::OnEnd()
{
	Script::OnEnd();
}


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
void StateChanger::Process(int timeInMs)
{
	if (gs == NULL)
	{
		scriptState = Script::ENDING;
		return;
	}
	// Wait until the state has been entered too.
	AppState * activeState = StateMan.ActiveState();
	if (activeState == gs)
		scriptState = Script::ENDING;
}

void StateChanger::OnEnd()
{
	// Notify parent etc. when finishing.
	Script::OnEnd();
}


/// Base class for fade-effects using the System-global ui.
UIElement * OverlayScript::overlayUI[OverlayScript::MAX_OVERLAY_LAYERS];

OverlayScript::OverlayScript(String line, Script * parent)
: Script(line, parent)
{
	// Delete when ended!
	flags |= DELETE_WHEN_ENDED;
	// In-case multiple scripts of this type are active. Check it.

	// Nullify stuff for security reasons.
	duration = 0;
	// 0 = sent first texture, 1 = sent second texture.
	scriptState = -1;
}

#define OVERLAY_BACKGROUND_1 "overlayFadeBackground1"
#define OVERLAY_BACKGROUND_2 "overlayFadeBackground2"
#define OVERLAY_FADE_UI_1 "overlayFadeUI1"
#define OVERLAY_FADE_UI_2 "overlayFadeUI2"
#define OVERLAY_UI_TEXT "overlayTextUI"

void OverlayScript::OnBegin()
{
	// Set start-time to current time.
	startTime = Timer::GetCurrentTimeMs();
	// Check if the overlay UI are already present.
	UserInterface * ui = GlobalUI();
	// Create ui as needed.
	if (!ui || !ui->GetElementByName(OVERLAY_FADE_UI_1))
	{
		UIImage * image = new UIImage("NULL");
		image->color[3] = 0.0f;
		String imageUIName = OVERLAY_FADE_UI_1;
		image->name = imageUIName;
		Graphics.QueueMessage(new GMAddGlobalUI(image));
		overlayUI[LAYER_1] = image;

		// Create second image to blend between.
		image = new UIImage("NULL");
		image->color[3] = 0.0f;
		image->color = Vector4f(1,1,1,0);
		image->name = OVERLAY_FADE_UI_2;
		overlayUI[LAYER_2] = image;
		Graphics.QueueMessage(new GMAddGlobalUI(image));

		// Create text-ui. Default it center and a bit down?
		UILabel * label = new UILabel();
		label->name = OVERLAY_UI_TEXT;
		label->sizeRatioX = 0.8f;
		label->sizeRatioY = 0.2f;
		label->alignmentY = 0.2f;
		label->textureSource = "80Gray50Alpha.png";
		overlayUI[GT_TEXT] = label;
		Graphics.QueueMessage(new GMAddGlobalUI(label));
	}
}

void OverlayScript::Process(int timeInMs)
{
}

void OverlayScript::OnEnd()
{
}

// For accessing the ui-elements.
void OverlayScript::SetLayerAlpha(int layer, float alpha)
{
	switch(layer)
	{
		case BACKGROUND:
		case LAYER_1:
		case LAYER_2:
			Graphics.QueueMessage(new GMSetGlobalUIf(overlayUI[layer]->name, GMUI::ALPHA, alpha, WindowMan.MainWindow()));
			break;
	}
}

float OverlayScript::GetLayerAlpha(int layer)
{
	switch(layer)
	{
		case BACKGROUND:
		case LAYER_1:
		case LAYER_2:
			return overlayUI[layer]->color[3];
			break;
	}
	throw 3;
}

// Set main text.
void OverlayScript::SetText(String text)
{
	Graphics.QueueMessage(new GMSetGlobalUIs(OVERLAY_UI_TEXT, GMUI::TEXT, text));
}


/// For fading the overlay background.
FadeInBackground::FadeInBackground(String line, Script * parent)
: OverlayScript(line, parent)
{
	fadeInDuration = 0;
	// First fetch stuff within parenthesis.
	String parenthesisContents = line.Tokenize("()")[1]; 
	List<String> args = parenthesisContents.Tokenize("(,)");
	switch(args.Size())
	{
	default:
		if (args.Size() < 0)
			return;
	case 3:
		duration = (int)args[2].ParseFloat();
	case 2:
		fadeInDuration = (int)args[1].ParseFloat();
	case 1:
		fadeInTextureSource = args[0];
	case 0:
		// Function-name here.
		break;
	}
}
void FadeInBackground::OnBegin()
{
	OverlayScript::OnBegin();

	// Set alpha of primary texture to 1 and secondary to 0 to start fading properly.
	Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_BACKGROUND_2, GMUI::ALPHA, 0.0f));
	// Set texture of second one to fade to.
	Graphics.QueueMessage(new GMSetGlobalUIs(OVERLAY_BACKGROUND_2, GMUI::TEXTURE_SOURCE, fadeInTextureSource));

	// Send a graphics-message to start blending the 
//	Graphics.QueueMessage(new GMSetOverlay(fadeInTextureSource, fadeInDuration));
	scriptState = 0;
	fadeState = 0;
	totalTime = fadeInDuration + duration;
}

void FadeInBackground::Process(int timeInMs)
{
	// If we have a texture to fade-out to, check that.
	timePassed += (int)timeInMs;

	/** 0 = sent first texture, fading it, 
		1 = resting at full alpha, 
		2 = sent second texture, fading out, 
		3 = ended, swapping textures, 
		4 = done.
	*/
	float alpha;
	switch(fadeState)
	{
	case FadeState::FADING_IN: 
		alpha = float(timePassed) / fadeInDuration;
//		std::cout<<"\nAlpha: "<<alpha;
		// Fade-in new one as we fade-out the other?
		Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_BACKGROUND_2, GMUI::ALPHA, alpha));
		// Swap them once alpha is full.
		if (alpha > 1.0f || timePassed > fadeInDuration)
		{
			fadeState++;
		}

		break;
	case FadeState::RESTING:
		// Resting.
		// Check time.
		if (timePassed > fadeInDuration + duration)
		{
			// If not, go to fadeState 4 and end this! p.o
			fadeState = FadeState::DONE;
			scriptState = Script::ENDING;
		}
		break;
	default:
	case FadeState::DONE:	
		if (timePassed > totalTime)
		{
			scriptState = Script::ENDING;	
		}
		break;
	}
}

void FadeInBackground::OnEnd()
{
	Graphics.QueueMessage(new GMSetGlobalUIs(OVERLAY_BACKGROUND_1, GMUI::TEXTURE_SOURCE, fadeInTextureSource));
	Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_BACKGROUND_1, GMUI::ALPHA, 1.0f));
	Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_BACKGROUND_2, GMUI::ALPHA, 1.0f));
	// Notify parent etc. when finishing.
	Script::OnEnd();
}


FadeOutBackground::FadeOutBackground(String line, Script * parent)
: OverlayScript(line, parent)
{
	fadeOutDuration = 0;
	// First fetch stuff within parenthesis.
	String parenthesisContents = line.Tokenize("()")[1]; 
	List<String> args = parenthesisContents.Tokenize("(,)");
	switch(args.Size())
	{
	default:
		if (args.Size() < 0)
			return;
	case 1:
		fadeOutDuration = (int)args[0].ParseFloat();
		break;
	}
}
void FadeOutBackground::OnBegin()
{
	OverlayScript::OnBegin();
	// Take note of alpha when starting fade-out.
	startAlpha = 1.0f;
	scriptState = 0;
	fadeState = FadeState::FADING_OUT;
	totalTime = fadeOutDuration;
}


void FadeOutBackground::Process(int timeInMs)
{
	timePassed += timeInMs;
	/** 0 = sent first texture, fading it, 
		1 = resting at full alpha, 
		2 = sent second texture, fading out, 
		3 = ended, swapping textures, 
		4 = done.
	*/
	float alpha;
	switch(fadeState)
	{
	case FadeState::FADING_OUT: 
#define Maximum(a,b) ((a) > (b)? (a) : (b))
		alpha = float(fadeOutDuration - timePassed) / Maximum(fadeOutDuration,1) * startAlpha;
		alpha = Maximum(alpha, 0.0f);
		// Fade-in new one as we fade-out the other?
		Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_BACKGROUND_1, GMUI::ALPHA, alpha));
		if (alpha <= 0.0f)
		{
			fadeState++;
		}
		break;
	default:
	case FadeState::DONE:	
	case FadeState::RESTING:
		// Resting.
		// Check time.
		if (timePassed > fadeOutDuration + duration)
		{
			fadeState = FadeState::DONE;
			scriptState = Script::ENDING;	
		}
		break;
	}
}

void FadeOutBackground::OnEnd()
{
	// Set alpha to both channels.
	Graphics.QueueMessage(new GMSetGlobalUIs(OVERLAY_BACKGROUND_1, GMUI::TEXTURE_SOURCE, "NULL"));
	Graphics.QueueMessage(new GMSetGlobalUIs(OVERLAY_BACKGROUND_2, GMUI::TEXTURE_SOURCE, "NULL"));
	// Notify parent etc. when finishing.
	Script::OnEnd();
}


/// For fading in images.
FadeInEffect::FadeInEffect(String line, Script * parent)
: OverlayScript(line, parent)
{
	fadeInDuration = 0;
	// First fetch stuff within parenthesis.
	String parenthesisContents = line.Tokenize("()")[1]; 
	List<String> args = parenthesisContents.Tokenize("(,)");
	switch(args.Size())
	{
	default:
		if (args.Size() < 0)
			return;
	case 3:
		duration = (int)args[2].ParseFloat();
	case 2:
		fadeInDuration = (int)args[1].ParseFloat();
	case 1:
		fadeInTextureSource = args[0];
	case 0:
		// Function-name here.
		break;
	}
}
void FadeInEffect::OnBegin()
{
	OverlayScript::OnBegin();

	// Set alpha of primary texture to 1 and secondary to 0 to start fading properly.
	Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_FADE_UI_2, GMUI::ALPHA, 0.0f));
	// Set texture of second one to fade to.
	Graphics.QueueMessage(new GMSetGlobalUIs(OVERLAY_FADE_UI_2, GMUI::TEXTURE_SOURCE, fadeInTextureSource));

	// Send a graphics-message to start blending the 
//	Graphics.QueueMessage(new GMSetOverlay(fadeInTextureSource, fadeInDuration));
	scriptState = 0;
	fadeState = 0;
	totalTime = fadeInDuration + duration;
}

void FadeInEffect::Process(int timeInMs)
{
	// If we have a texture to fade-out to, check that.
	timePassed += (int)timeInMs;

	/** 0 = sent first texture, fading it, 
		1 = resting at full alpha, 
		2 = sent second texture, fading out, 
		3 = ended, swapping textures, 
		4 = done.
	*/
	float alpha;
	switch(fadeState)
	{
	case FadeState::FADING_IN: 
		alpha = float(timePassed) / Maximum(fadeInDuration,1);
		std::cout<<"\nAlpha: "<<alpha;
		// Fade-in new one as we fade-out the other?
		Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_FADE_UI_2, GMUI::ALPHA, alpha));
		// Swap them once alpha is full.
		if (alpha > 1.0f || timePassed > fadeInDuration)
		{
			fadeState++;
		}
		break;
	case FadeState::RESTING:
		// Resting.
		// Check time.
		if (timePassed > fadeInDuration + duration)
		{
			// If not, go to fadeState 4 and end this! p.o
			fadeState = FadeState::DONE;
			scriptState = Script::ENDING;
		}
		break;
	default:
	case FadeState::DONE:	
		if (timePassed > totalTime)
		{
			scriptState = Script::ENDING;	
		}
		break;
	}
}

void FadeInEffect::OnEnd()
{
	Graphics.QueueMessage(new GMSetGlobalUIs(OVERLAY_FADE_UI_1, GMUI::TEXTURE_SOURCE, fadeInTextureSource));
	Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_FADE_UI_1, GMUI::ALPHA, 1.0f));
	Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_FADE_UI_2, GMUI::ALPHA, 0.0f));
	// Notify parent etc. when finishing.
	Script::OnEnd();
}


FadeOutEffect::FadeOutEffect(String line, Script * parent)
: OverlayScript(line, parent)
{
	fadeOutDuration = 0;
	// First fetch stuff within parenthesis.
	String parenthesisContents = line.Tokenize("()")[1]; 
	List<String> args = parenthesisContents.Tokenize("(,)");
	switch(args.Size())
	{
	default:
		if (args.Size() < 0)
			return;
	case 1:
		fadeOutDuration = (int)args[0].ParseFloat();
		break;
	}
	if (fadeOutDuration < 1)
		fadeOutDuration = 1;
}
void FadeOutEffect::OnBegin()
{
	OverlayScript::OnBegin();
	// Take note of alpha when starting fade-out.
	startAlpha = 1.0f;
	scriptState = 0;
	fadeState = FadeState::FADING_OUT;
	totalTime = fadeOutDuration;
}


void FadeOutEffect::Process(int timeInMs)
{
	timePassed += timeInMs;
	/** 0 = sent first texture, fading it, 
		1 = resting at full alpha, 
		2 = sent second texture, fading out, 
		3 = ended, swapping textures, 
		4 = done.
	*/
	float alpha;
	switch(fadeState)
	{
	case FadeState::FADING_OUT: 
#define Maximum(a,b) ((a) > (b)? (a) : (b))
		alpha = float(fadeOutDuration - timePassed) / Maximum(fadeOutDuration * startAlpha, 1);
		alpha = Maximum(alpha, 0.0f);
		// Fade-in new one as we fade-out the other?
		Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_FADE_UI_1, GMUI::ALPHA, alpha));
		if (alpha <= 0.0f)
		{
			fadeState++;
		}
		break;
	default:
	case FadeState::DONE:	
	case FadeState::RESTING:
		// Resting.
		// Check time.
		if (timePassed > fadeOutDuration + duration)
		{
			fadeState = FadeState::DONE;
			scriptState = Script::ENDING;	
		}
		break;
	}
}

void FadeOutEffect::OnEnd()
{
	// Set alpha to both channels.
	Graphics.QueueMessage(new GMSetGlobalUIs(OVERLAY_FADE_UI_1, GMUI::TEXTURE_SOURCE, "NULL"));
	Graphics.QueueMessage(new GMSetGlobalUIs(OVERLAY_FADE_UI_2, GMUI::TEXTURE_SOURCE, "NULL"));
	// Notify parent etc. when finishing.
	Script::OnEnd();
}


FadeTextEffect::FadeTextEffect(String line, Script * parent)
: OverlayScript(line, parent)
{
	pausesExecution = false;
	fadeOutDuration = 0;
	// First fetch stuff within parenthesis.
	String parenthesisContents = line.Tokenize("()")[1]; 
	List<String> args = parenthesisContents.Tokenize("(,)");
	if(args.Size() < 4)
		return;
	fadeOutDuration = (int)args[3].ParseFloat();	
	duration = (int)args[2].ParseFloat();		
	fadeInDuration = (int)args[1].ParseFloat();	
	text = args[0];
	// Remove quote-marks.
	text.Remove('\"', true);
}
void FadeTextEffect::OnBegin()
{
	OverlayScript::OnBegin();
	// Take note of alpha when starting fade-out.
	scriptState = 0;
	fadeState = FadeState::FADING_IN;
	totalTime = fadeOutDuration;
	// Send initial text
	SetText(text);
	Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_UI_TEXT, GMUI::TEXT_ALPHA, 0.0f));
}
void FadeTextEffect::Process(int timeInMs)
{
	// If we have a texture to fade-out to, check that.
	timePassed += timeInMs;
	/** 0 = sent first texture, fading it, 
		1 = resting at full alpha, 
		2 = sent second texture, fading out, 
		3 = ended, swapping textures, 
		4 = done.
	*/
	float alpha;
	switch(fadeState)
	{
	case FadeState::FADING_IN:
		alpha = float(timePassed) / fadeInDuration;
		Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_UI_TEXT, GMUI::TEXT_ALPHA, alpha));
		if (alpha > 1.0f)
			fadeState = FadeState::RESTING;
		break;
	case FadeState::RESTING:
		if (timePassed > fadeInDuration + duration)
			fadeState = FadeState::FADING_OUT;
		break;
	case FadeState::FADING_OUT: 
#define Maximum(a,b) ((a) > (b)? (a) : (b))
		alpha = float((fadeOutDuration + duration + fadeInDuration) - timePassed) / fadeOutDuration;
		alpha = Maximum(alpha, 0.0f);
		// Fade-in new one as we fade-out the other?
		Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_UI_TEXT, GMUI::TEXT_ALPHA, alpha));
		if (alpha <= 0.0f)
		{
			fadeState++;
		}
		break;
	default:
	case FadeState::DONE:	
		// Resting.
		// Check time.
		if (timePassed > fadeOutDuration + duration)
		{
			fadeState = FadeState::DONE;
			scriptState = Script::ENDING;	
		}
		break;
	}
}

void FadeTextEffect::OnEnd()
{
	// Set alpha to both channels.
	Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_UI_TEXT, GMUI::TEXT_ALPHA, 0.0f));
	Graphics.QueueMessage(new GMSetGlobalUIf(OVERLAY_UI_TEXT, GMUI::ALPHA, 0.0f));	
	// Notify parent etc. when finishing.
	Script::OnEnd();
}


