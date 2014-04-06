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
	switch(action){
		case TOGGLE_FULL_SCREEN:
			Graphics.ToggleFullScreen();
			break;
		case TOGGLE_MOUSE_LOCK:
			Input.SetMouseLock(!Input.MouseLocked());
			break;
		case RELOAD_BATTLERS: {
			std::cout<<"\nReloading battlers...";
			RuneBattlers.LoadFromDirectory(BATTLERS_DIRECTORY);
			RuneBattlers.LoadBattles(BATTLES_DIRECTORY);
			break;					 
		}
		
		case INTERPRET_CONSOLE_COMMAND: {
#ifdef _DEBUG
			String command = Input.GetInputBuffer();

			// Check if null-string
			if (command == NULL)
				break;

			List<String> token = command.Tokenize(" ");
			int tokensFound = token.Size();
			command.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (command == "toggle debug renders"){
				Graphics.EnableAllDebugRenders(!Graphics.renderGrid);
			}
			else if (command == "toggle physics shapes"){
				Graphics.renderPhysics = !Graphics.renderPhysics;
			}
#endif
			break;
		}
#ifdef USE_NETWORK
#endif
	}
}
