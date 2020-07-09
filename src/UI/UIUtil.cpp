/// Emil Hedemalm
/// 2015-05-10
/// Contains utility functions for handling UI. Moving code out from MessageManager so not everything is there...

#include "UIUtil.h"

#include "UserInterface.h"
#include "UIElement.h"
#include "Graphics/Messages/GMUI.h"
#include "StateManager.h"

#include "File/LogFile.h"

/// Pushes and makes visible ui by given name or source. Queues messages to the graphics thread for creation if needed.
bool PushUI(String nameOrSource)
{
	/// The user might want to enter either an element or a whole new UI here, so investigate it!
	String uiName = nameOrSource;
	String uiSrc = nameOrSource;
	/// Check if the element exists...!
	UserInterface * ui = ActiveUI();
	if (!ui)
		ui = MainUI();
	if (!ui)
	{
		LogMain("No UI to interact with in PushUI: "+nameOrSource, ERROR);
		return false;
	}
	UIElement * element = NULL;
	/// Check if it's a source file, if so try and load that first.
	if (uiName.Contains(".gui"))
	{
		element = ui->GetElementBySource(uiName);
		if (!element){
			// Load it.
			element = UserInterface::LoadUIAsElement(uiSrc);
			/// Return if we fail to load.
			if (!element){
				std::cout<<"\nUnable to load ui: "<<uiName;
				return false;
			}
			/// If we load here, reset elementName since uiName since it probably changed.
			uiName = element->name;
			/// Make sure it is exitable. No.
//			element->exitable = true;
			QueueGraphics(new GMAddUI(element));
		}
	}
	/// Regular name
	else {
		element = ui->GetElementByName(uiName);		
	}
	/// Push it to stack if not.
	if (element)
		QueueGraphics(GMPushUI::ToUI(element, ui));
	/// Just try pushing it.
	else 
		QueueGraphics(GMPushUI::ToUI(uiName, ui));
	return true;
}

bool PopUI(String nameOrSource)
{
	String uiName = nameOrSource;
	/// Force pop it?
	QueueGraphics(new GMPopUI(uiName, RelevantUI(), true));
	return true;
}

