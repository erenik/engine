// Emil Hedemalm
// 2013-03-30

#include "GameState.h"
#include "../UI/UserInterface.h"
#include "Message/Message.h"
#include "../StateManager.h"
#include "Graphics/GraphicsManager.h"
#include "Input/InputManager.h"
#include "Network/NetworkManager.h"
#include "OS/Sleep.h"
#include "Chat/ChatMessage.h"

//#include <windows.h>

extern UserInterface * ui[GameStateID::MAX_GAME_STATES];

#ifdef WINDOWS
extern HWND hWnd;			// Window handle
#endif

// User interfaces for all states
UserInterface * ui[GameStateID::MAX_GAME_STATES];


void EmptyFunction(){
	std::cout<<"Empty function";
};


GameState::GameState()
{
	/// Initialize default values, even if inherited.
	lastTime = 0;
	ui = NULL;
    name = "DefaultStateName";
	keyPressedCallback = false;
};
GameState::~GameState(){
	assert(ui == NULL);
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void GameState::ProcessPacket(Packet * packet){
	// DON'T worite this line D:
	//std::cout<<"\nState::ProcessPacket called:";
}
/// Callback function that will be triggered via the MessageManager when messages are processed.
void GameState::ProcessMessage(Message * message){
	std::cout<<"\nState::ProcessMessage called:";
	switch(message->type){
		case MessageType::STRING: {
			String s = message->msg;
			s.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (s == "go_to_main_menu")
				StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_MAIN_MENU));
			else if (s == "go_to_editor")
				StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_EDITOR));
			else if (s == "exit")
				StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_EXITING));
			else if (s == "begin_input(this)"){
				UserInterface * ui = StateMan.ActiveState()->GetUI();
				UIElement * element = ui->GetActiveElement();
				if (element != NULL){
					assert(element->onTrigger);
					Input.EnterTextInputMode(element->onTrigger);
					if (element->text)
						Input.SetInputBuffer(element->text.c_str());
				}
				else
					assert(false && "NULL-element :<");
			}
			else {
				std::cout<<"\nUndefined message received: "<<message->msg;
			}
			break;
		}
	}
}

void GameState::OnChatMessageReceived(ChatMessage * cm){
    std::cout<<"\nDefault GameState::OnChatMessageReceived triggered.";
}

/// Input functions for the various states
/** Handles a mouse click.
	Argument true indicate that the button was pressed, while false indicates that it was just released.
	Default arguments for x and y indicate that they should not be refreshed.
*/
void GameState::MouseClick(bool down, int x, int y, UIElement * elementClicked){
	std::cout<<"\nDefault GameState::MouseClick activated.";
}
/** Handles a mouse click.
	Argument true indicate that the button was pressed, while false indicates that it was just released.
	Default arguments for x and y indicate that they should not be refreshed.
*/
void GameState::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){
#ifdef _DEFAULT_STATE_DEBUG
	std::cout<<"\nDefault GameState::MouseRightClick activated.";
#endif
}
/// Interprets a mouse-move message to target position.
void GameState::MouseMove(int x, int y, bool lDown, bool rDown, UIElement * elementOver){
#ifdef _DEFAULT_STATE_DEBUG
	std::cout<<"\nDefault GameState::MouseMove activated.";
#endif
}
/** Handles mouse wheel input.
	Positive delta signifies scrolling upward or away from the user, negative being toward the user.
*/
void GameState::MouseWheel(float delta)
{
#ifdef _DEFAULT_STATE_DEBUG
	std::cout<<"\nDefault GameState::MouseWheel activated.";
#endif
}


/// Callback from the Input-manager, query it for additional information as needed.
void GameState::KeyPressed(int keyCode, bool downBefore){
	std::cout<<"\nDefault GameState::KeyPressed activated.";
}

/*
void GameState::OnExit(GameState * nextState){
	print("\nLeaving state: "<<this->id);
}
*/


/** Function to handle custom actions defined per state.
		This function is called by the various bindings that the state defines.
	*/
void GameState::InputProcessor(int action, int inputDevice/* = 0*/){
	std::cout<<"\nState::InputProcessor called.";
}
/// Creates default key-bindings for the state.
void GameState::CreateDefaultBindings(){
	std::cout<<"\nState::CreateDefaultBindings called.";
}

/// Creates the user interface for this state
void GameState::CreateUserInterface()
{
	std::cout<<"\nState::CreateUserInterface called for "<<name;
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/Empty.gui");
}
/** Attempts to free the resources used by the user interface before deleting it.
	Aborts and returns false if any errors occur along the way.
*/
bool GameState::DeallocateUserInterface()
{
    std::cout<<"\nCalling DeallocateUserInterface for state: "<<name;
	if (ui){
		if (ui->IsBuffered()){
            Graphics.QueueMessage(new GMDelete(ui));
        }
        else {
            //  Just delete it if it isn't buffered.
            if (ui->IsGeometryCreated())
                ui->DeleteGeometry();
            delete ui;
        }

	}
	ui = NULL;
	return true;
}

/// For handling drag-and-drop files.
void GameState::HandleDADFiles(List<String> & files){
	// Default. Do nothing.
}


/// What happens.. when we rendar?!
void GameState::Render(GraphicsState & graphicsState){

}

#include "../Initializer.h"
#ifdef WINDOWS
#include <process.h>
extern uintptr_t deallocatorThread;
#elif defined LINUX | defined OSX
extern pthread_t deallocatorThread;
#endif

Exit::Exit()
{
    id = GameStateID::GAME_STATE_EXITING;
    name = "Exiting state";
}

void Exit::OnEnter(GameState * previousState){
    if (previousState == this){
        std::cout<<"\nWARNING: Already in exit state.";
        return;
    }
	assert(previousState != this);
	std::cout<<"\nEntering Exit state. Calling Deallocate with thread.";
#ifdef WINDOWS
	// Call the deallocator thread!
	deallocatorThread = _beginthread(Deallocate, NULL, NULL);
#elif defined LINUX | defined OSX
    int iret1 = pthread_create(&deallocatorThread , NULL, Deallocate, NULL);
#endif
}
void Exit::Process(float time){

#ifdef WINDOWS
	if (deallocatorThread){
		Sleep(10);
	}
	std::cout<<"Deallocation finished. Exiting program!";
	/// Post quit message if the deallocator thread has finished! :)
	PostQuitMessage(0);
#endif
};
