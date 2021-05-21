// Emil Hedemalm
// 2013-03-17

#include "GraphicsMessage.h"
#include "../GraphicsManager.h"
#include "GraphicsMessages.h"

GMBufferUI::GMBufferUI(UIElement * i_element): GraphicsMessage(GM_BUFFER_UI) 
{
	this->element = i_element;
}

void GMBufferUI::Process(GraphicsState* graphicsState)
{
	element->Rebufferize(*graphicsState);
}