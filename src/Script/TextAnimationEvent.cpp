/// Emil Hedemalm
/// 2014-01-07
/// General events manipulating text-strings in UI elements!

#include "TextAnimationEvent.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Render/RenderViewport.h"

TextAnimationEvent::TextAnimationEvent(int type, String elementName, int viewport)
: Script(), type(type), elementName(elementName), viewport(viewport)
{
	fadeInDuration = fadeOutDuration = 1000;
	duration = 2000;
	SetDeleteOnEnd(true);
	loaded = true;
}

void TextAnimationEvent::OnBegin(){
	totalDuration = duration + fadeInDuration + fadeOutDuration;
	switch(type){
		case NOTICE:
			// Default alpha to 0 first on the text.
			Graphics.QueueMessage(new GMSetUIv4f(elementName, GMUI::TEXT_COLOR, Vector4f(0,0,0,0), viewport));
			// Make UI visible if not already.
			Graphics.QueueMessage(new GMSetUIb(elementName, GMUI::VISIBILITY, true, viewport));

			break;
	}
	startTime = Timer::GetCurrentTimeMs();
}
void TextAnimationEvent::Process(float time)
{
	switch(type){
		case NOTICE:
			long long currentTime = Timer::GetCurrentTimeMs();
			float timePassed = (float)(currentTime - startTime);
			/// Fade-in
			if (timePassed < fadeInDuration){
				/// Fade-in the text.
				float relativeAlpha = timePassed / fadeInDuration;
			//	std::cout<<"\nRelative Alpha: "<<relativeAlpha;
				Graphics.QueueMessage(new GMSetUIv4f(elementName, GMUI::TEXT_COLOR, Vector4f(1,1,1, relativeAlpha), viewport));
			}
			/// Stay
			else if (timePassed < duration + fadeInDuration){
				/// Do nothing! :D
			}
			// Fade-out
			else if (timePassed < totalDuration){
				/// Fade-out the text.
				float relativeAlpha = 1.0f - (timePassed - duration - fadeInDuration) / fadeOutDuration;
			//	std::cout<<"\nRelative Alpha: "<<relativeAlpha;
				Graphics.QueueMessage(new GMSetUIv4f(elementName, GMUI::TEXT_COLOR, Vector4f(1,1,1, relativeAlpha), viewport));
			}
			else
				this->scriptState = ENDING;
			break;
	}
}
void TextAnimationEvent::OnEnd(){
	switch(type){
		case NOTICE:
			// Default alpha to 0 first on the text.
			Graphics.QueueMessage(new GMSetUIv4f(elementName, GMUI::TEXT_COLOR, Vector4f(0,0,0,0), viewport));
			// Make UI visible if not already.
			Graphics.QueueMessage(new GMSetUIb(elementName, GMUI::VISIBILITY, false, viewport));
			break;
	}
}
