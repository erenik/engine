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

/// Map editor, including script-editors and random map generator tools.

#include "TIFSMapEditor.h"
#include "TIFS.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"

#include "Application/Application.h"
#include "StateManager.h"

#include "Maps/MapManager.h"

#include "ModelManager.h"
#include "TextureManager.h"

#include "Input/InputManager.h"

#include "Message/Message.h"
#include "Message/VectorMessage.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

TIFSMapEditor::TIFSMapEditor()
{
	
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void TIFSMapEditor::OnEnter(AppState * previousState)
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

	// Create an entity..!
	MapMan.CreateEntity("Sphere", ModelMan.GetModel("Sphere"), TexMan.GetTexture("White"));

}
/// Main processing function, using provided time since last frame.
void TIFSMapEditor::Process(int timeInMs)
{

}
/// Function when leaving this state, providing a pointer to the next StateMan.
void TIFSMapEditor::OnExit(AppState * nextState)
{

}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void TIFSMapEditor::ProcessPacket(Packet * packet)
{

}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void TIFSMapEditor::ProcessMessage(Message * message)
{	
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::VECTOR_MESSAGE:
		{
			VectorMessage * vm = (VectorMessage*) message;
			Vector4f vec = vm->GetVector4f();
			groundSize = Vector2f(vec.x, vec.y);
			if (groundSize.x <= 0)
				groundSize.x = 1.f;
			if (groundSize.y <= 0)
				groundSize.y = 1.f;
			OnGroundSizeUpdated();
			break;
		}
		case MessageType::STRING:
		{
			if (msg == "GenerateGround")
			{
				GenerateGround();
			}
			break;	
		}
	}
}


/// Creates the user interface for this state. Is called automatically when re-building the UI with the CTRL+R+U command.
void TIFSMapEditor::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/TIFSEditor.gui");
}


void TIFSMapEditor::GenerateGround()
{
	// Clear old grid/ground, if any.
	MapMan.DeleteEntities(tifs->groundEntities);
	tifs->groundEntities.Clear();

	// Generate a nice grid..

	Entity * ground = MapMan.CreateEntity("Ground", ModelMan.GetModel("plane"), TexMan.GetTexture("Grey"));
	tifs->groundEntities.Add(ground);

	// Set scale in x/z
	OnGroundSizeUpdated();
}


void TIFSMapEditor::OnGroundSizeUpdated()
{
	Physics.QueueMessage(new PMSetEntity(tifs->groundEntities, PT_SET_SCALE, Vector3f(groundSize.x, 1, groundSize.y)));	
}