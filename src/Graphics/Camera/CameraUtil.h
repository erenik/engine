/// Emil Hedemalm
/// 2015-01-16
/// Utility functions for handling cameras, including setting up and linking input to navigational actions.

#include "Camera.h"
#include "Input/Binding.h"

/** Creates default input-bindings that may be used in an AppState for a game or general camera control.
	These should then be added to the AppState's InputMapping. Each binding will trigger a message that is posted to the messageManager,
	which the AppState may then react to in the ProcessMessage(Message*) functions.
*/
List<Binding*> CreateDefaultCameraBindings();
/// Default processor to react to messages created by the default bindings declared above.
void ProcessCameraMessages(String msg, Camera * forCamera);



