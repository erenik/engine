// Emil Hedemalm
// 2013-03-30

#include "AppState.h"
#include "../UI/UserInterface.h"
#include "Message/Message.h"
#include "../StateManager.h"
#include "Input/InputManager.h"
#include "Network/NetworkManager.h"
#include "OS/Sleep.h"
#include "Chat/ChatMessage.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"

#include "File/LogFile.h"

void EmptyFunction(){
	std::cout<<"Empty function";
};


AppState::AppState()
{
	/// Initialize default values, even if inherited.
	lastTime = 0;
	ui = NULL;
    name = "DefaultStateName";
	keyPressedCallback = false;
};
AppState::~AppState()
{
	if (ui)
	{
		Graphics.QueueMessage(new GMDelete(ui));
		ui = NULL;
	}
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void AppState::ProcessPacket(Packet * packet){
	// DON'T worite this line D:
	//std::cout<<"\nState::ProcessPacket called:";
}
/// Callback function that will be triggered via the MessageManager when messages are processed.
void AppState::ProcessMessage(Message * message)
{
//	std::cout<<"\nState::ProcessMessage called:";
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
					InputMan.EnterTextInputMode(element->onTrigger);
					if (element->text)
						InputMan.SetInputBuffer(element->text.c_str());
				}
				else
					assert(false && "NULL-element :<");
			}
			else {
//				std::cout<<"\nUndefined message received: "<<message->msg;
			}
			break;
		}
	}
}

void AppState::OnChatMessageReceived(ChatMessage * cm){
    std::cout<<"\nDefault AppState::OnChatMessageReceived triggered.";
}

/// Input functions for the various states
/** Handles a mouse click.
	Argument true indicate that the button was pressed, while false indicates that it was just released.
	Default arguments for x and y indicate that they should not be refreshed.
*/
void AppState::MouseClick(AppWindow * AppWindow, bool down, int x, int y, UIElement * elementClicked){
	std::cout<<"\nDefault AppState::MouseClick activated.";
}
/** Handles a mouse click.
	Argument true indicate that the button was pressed, while false indicates that it was just released.
	Default arguments for x and y indicate that they should not be refreshed.
*/
void AppState::MouseRightClick(AppWindow * AppWindow, bool down, int x, int y, UIElement * elementClicked){
#ifdef _DEFAULT_STATE_DEBUG
	std::cout<<"\nDefault AppState::MouseRightClick activated.";
#endif
}
/// Interprets a mouse-move message to target position.
void AppState::MouseMove(AppWindow * AppWindow, int x, int y, bool lDown, bool rDown, UIElement * elementOver){
#ifdef _DEFAULT_STATE_DEBUG
	std::cout<<"\nDefault AppState::MouseMove activated.";
#endif
}
/** Handles mouse wheel input.
	Positive delta signifies scrolling upward or away from the user, negative being toward the user.
*/
void AppState::MouseWheel(AppWindow * AppWindow, float delta)
{
#ifdef _DEFAULT_STATE_DEBUG
	std::cout<<"\nDefault AppState::MouseWheel activated.";
#endif
}


/// Callback from the Input-manager, query it for additional information as needed.
void AppState::KeyPressed(int keyCode, bool downBefore){
	std::cout<<"\nDefault AppState::KeyPressed activated.";
}

/*
void AppState::OnExit(AppState * nextState){
	print("\nLeaving state: "<<this->id);
}
*/


/** Function to handle custom actions defined per state.
		This function is called by the various bindings that the state defines.
	*/
void AppState::InputProcessor(int action, int inputDevice/* = 0*/)
{
//	std::cout<<"\nState::InputProcessor called.";
}
/// Creates default key-bindings for the state.
void AppState::CreateDefaultBindings()
{
//	std::cout<<"\nState::CreateDefaultBindings called.";
}

/// Creates the user interface for this state
void AppState::CreateUserInterface()
{
	std::cout<<"\nState::CreateUserInterface called for "<<name;
	LogMain("State::CreateUserInterface called for state "+name+". Empty UI is created by default!", INFO);
	// Delete old one in graphics thread?
//	if (ui)
//		delete ui;
	ui = NewA(UserInterface);
	ui->CreateRoot();
}
/** Attempts to free the resources used by the user interface before deleting it.
	Aborts and returns false if any errors occur along the way.
*/
bool AppState::DeallocateUserInterface()
{
    std::cout<<"\nCalling DeallocateUserInterface for state: "<<name;
	if (ui)
	{
		if (ui->IsBuffered())
		{
            Graphics.QueueMessage(new GMDelete(ui));
        }
        else {
            //  Just delete it if it isn't buffered.
            if (ui->IsGeometryCreated())
                ui->DeleteGeometry();
	// Delete old one in graphics thread?
            delete ui;
        }

	}
	ui = NULL;
	return true;
}

/// For handling drag-and-drop files.
void AppState::HandleDADFiles(List<String> & files){
	// Default. Do nothing.
}


/// What happens.. when we rendar?!
void AppState::Render(GraphicsState * graphgics)
{
//	std::cout<<"\nDefault AppState::Render called. Overloading failed or AppWindow got state rendering enabled in vain.";
}

#include "../Initializer.h"
#ifdef WINDOWS
#include <process.h>
#endif

extern THREAD_HANDLE deallocatorThread;

Exit::Exit()
{
    id = GameStateID::GAME_STATE_EXITING;
    name = "Exiting state";
}

void Exit::OnEnter(AppState * previousState)
{
    if (previousState == this){
        std::cout<<"\nWARNING: Already in exit state.";
        return;
    }
	assert(previousState != this);
	std::cout<<"\nEntering Exit state. Calling Deallocate with thread.";

    CREATE_AND_START_THREAD(Deallocate, deallocatorThread);
}

void Exit::Process(int timeInMs)
{
	// Exiting is actually done elsewhere.
};
