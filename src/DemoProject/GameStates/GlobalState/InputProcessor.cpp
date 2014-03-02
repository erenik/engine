// Awesome Author

#include "GlobalState.h"
#include "Actions.h"
// Don't include all managers. Ever.
#include "Network/NetworkManager.h"
#include "Input/InputManager.h"
#include "Graphics/GraphicsManager.h"

void DemoProjectGlobalState::InputProcessor(int action, int inputDevice/* = 0*/){
	/// Don't do some stuff if entering text man..
	if (Input.IsInTextEnteringMode())
		return;
	switch(action){
		case TOGGLE_MOUSE_LOCK:
			Input.SetMouseLock(!Input.MouseLocked());
			break;
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
	}
}
