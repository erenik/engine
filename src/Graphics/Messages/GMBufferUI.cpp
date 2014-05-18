// Emil Hedemalm
// 2013-03-17

#include "GraphicsMessage.h"
#include "../GraphicsManager.h"
#include "GraphicsMessages.h"

GMBufferUI::GMBufferUI(UIElement * i_element): GraphicsMessage(GM_BUFFER_UI) 
{
	this->element = i_element;
}

void GMBufferUI::Process()
{
	
	// Check if already buffered. If so re-create the mesh in it's entirety.
	if (element->IsBuffered())
	{
		// Recalculate bounds depending on parent.
		element->AdjustToParent();
		// MAybe dont have to free  the buffers though?
	//	element->FreeBuffers();
		if (element->IsGeometryCreated())
			element->ResizeGeometry();
		else
			element->CreateGeometry();
	}
	
//	std::cout<<"\nBuffering UI..";
	element->Bufferize();
//	std::cout<<"\nFinished";
}