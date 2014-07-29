/// Emil Hedemalm
/// 2014-07-29
/** Reboot of the TIFS/Virtus project as was conducted during 10 weeks in the spring of 2013 with the following members:
	- Emil Hedemalm
	- Aksel Kornesjö
	- Cheng Wu
	- Andreas Söderberg
	- Michaela Sjöström
	- Fredric Lind

	Old dev blog:	http://focus.gscept.com/gp13-3/
	Old facebook page:	https://www.facebook.com/VirtusLTU
	Our final release for that first iteration of the project: http://svn.gscept.com/gp13-3/public/Virtus.zip
*/

/// The main/global Application state for the game.

#include "TIFS.h"
#include "TIFSMapEditor.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"

#include "Message/Message.h"

#include "Application/Application.h"
#include "StateManager.h"

#include "Input/InputManager.h"

TIFS * tifs = NULL;
TIFSMapEditor * mapEditor = NULL;

void SetApplicationDefaults()
{
	Application::name = "The Invader from Space / VIRTUS";
	TextFont::defaultFontSource = "font3";
}

void RegisterStates()
{
	tifs = new TIFS();
	StateMan.RegisterState(tifs);
	mapEditor = new TIFSMapEditor();
	StateMan.RegisterState(mapEditor);
	StateMan.QueueGlobalState(tifs);
}




TIFS::TIFS()
{
	
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void TIFS::OnEnter(AppState * previousState)
{
	// Remove shit.

	// Do shit.

	if (!ui)
		CreateUserInterface();
	// Set ui as active?
	Graphics.QueueMessage(new GMSetUI(ui));

	// Remove shit
	Graphics.QueueMessage(new GMSetOverlay(NULL));

	Input.ForceNavigateUI(true);

}
/// Main processing function, using provided time since last frame.
void TIFS::Process(int timeInMs)
{

}
/// Function when leaving this state, providing a pointer to the next StateMan.
void TIFS::OnExit(AppState * nextState)
{

}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void TIFS::ProcessPacket(Packet * packet)
{

}
/// Callback function that will be triggered via the MessageManager when messages are processed.
void TIFS::ProcessMessage(Message * message)
{	
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg == "Editor" || msg == "GoToEditor")
			{
				// Go there?
				StateMan.QueueState(mapEditor);
			}
			else if (msg == "OpenMainMenu")
			{
				Graphics.QueueMessage(new GMSetUI(ui));
				StateMan.QueueState(NULL);
			}
			break;	
		}
	}
}


/// Creates the user interface for this state. Is called automatically when re-building the UI with the CTRL+R+U command.
void TIFS::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/TIFS.gui");

}
