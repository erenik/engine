// Awesome Author

#include "RuneGlobalState.h"
#include "Actions.h"
// Don't include all managers. Ever.
#include "Network/NetworkManager.h"
#include "RuneRPG/Battle/RuneBattlerManager.h"
#include "Input/InputManager.h"
#include "Graphics/GraphicsManager.h"

void RuneGlobalState::InputProcessor(int action, int inputDevice/* = 0*/){
	/// Don't do some stuff if entering text man..
	if (Input.IsInTextEnteringMode())
		return;
	switch(action)
	{
		case TOGGLE_MOUSE_LOCK:
			Input.SetMouseLock(!Input.MouseLocked());
			break;
		case RELOAD_BATTLERS: {
			std::cout<<"\nReloading battlers...";
			RuneBattlers.LoadFromDirectory(BATTLERS_DIRECTORY);
			RuneBattlers.LoadBattles(BATTLES_DIRECTORY);
			break;					 
		}
	}
}
