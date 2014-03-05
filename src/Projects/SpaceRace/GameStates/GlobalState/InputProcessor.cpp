// Emil Hedemalm
// 2014-03-05
// 

#include "GlobalState.h"
#include "Actions.h"
// Don't include all managers. Ever.
#include "Network/NetworkManager.h"
#include "Input/InputManager.h"
#include "Graphics/GraphicsManager.h"

void GlobalState::InputProcessor(int action, int inputDevice/* = 0*/){
	/// Don't do some stuff if entering text man..
	if (Input.IsInTextEnteringMode())
		return;
	switch(action){
		/*
		/// Menu navigation, yush!
		case NEXT_UI_ELEMENT:
			Input.GoToNextElement();
			break;
		case PREVIOUS_UI_ELEMENT:
			Input.GoToPreviousElement();
			break;
		case ACTIVATE_UI_ELEMENT:
			Input.ActivateElement();
			break;
			*/
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
										/*
#ifdef USE_NETWORK
		case START_HOSTING:
			std::cout<<"\nInput>>START_HOSTING\n";
			NetworkMan.StartServer(DEFAULT_PORT, "");
			break;
		case START_JOINING:
			std::cout<<"\nInput>>START_JOINING\n";
			Network.JoinServer("127.0.0.1", DEFAULT_PORT, "");
			break;
        case START_JOINING_MY_WINDOWS_MACHINE:
			std::cout<<"\nInput>>START_JOINING\n";
			Network.JoinServer("130.240.132.45", DEFAULT_PORT, "");
			break;
		case STOP_NETWORK:
			std::cout << "\nInput>>STOP_NETWORK\n";
			Network.Shutdown();
			break;
		case SEND_PACKET_1:
			break;
#ifdef USE_FTP
		case MAKE_FTP_REQUEST:
			FtpRequest *request = Ftp.CreateFtpRequest();
			Ftp.AddFileToFtpRequest(request, "ftpTest.txt");
			Ftp.MakeFtpRequest(request, PACKET_TARGET_OTHERS);
			break;
#endif // End of USE_FTP
#endif // End of USE_NETWORK
			*/
	}
}
