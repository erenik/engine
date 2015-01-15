/// Emil Hedemalm
/// 2014-09-15
/// Filter for games. Contains relevant functions for sizing the game content relative to filter settings.

#include "CVGame.h"
#include "CV/CVPipeline.h"
#include "Game/Game2D.h"

#include "Physics/Integrator.h"
#include "Physics/PhysicsManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"

CVGame::CVGame(int filterID)
: CVRenderFilter(filterID)
{
	
	sizeRatio = new CVFilterSetting("SizeRatio", Vector2f(1.f,1.f));
	z = new CVFilterSetting("Z", 1.f);
	useMouse = new CVFilterSetting("Use mouse", false);
	resetButton = new CVFilterSetting("Reset");

	settings.Add(4,
		sizeRatio, z, useMouse,
		resetButton);
	
	integrator = NULL;
	game = NULL;
}

CVGame::~CVGame()
{
	if (game)
		delete game;
	game = NULL;
}

/// Called upon adding the filter to the active pipeline.
void CVGame::OnAdd()
{
	game->Initialize();
}


// Should be called when deleting a filter while the application is running. Removes things as necessary.
void CVGame::OnDelete()
{
	if (game)
		delete game;
	game = NULL;
}


int CVGame::Process(CVPipeline * pipe)
{
	Vector2i frameSize = pipe->initialInputSize;
	if (frameSize != lastFrameSize || sizeRatio->HasChanged())
	{
		Vector2f relFrame = sizeRatio->GetVec2f() * frameSize;
		game->SetFrameSize(relFrame);
		lastFrameSize = frameSize;
	}
	if (z->HasChanged())
	{
		game->SetZ(z->GetFloat());
	}
	if (resetButton->HasChanged())
		game->Reset();
	if (useMouse->HasChanged())
		game->UseMouseInput(useMouse->GetBool());

	if (!game->paused)
		game->Process();

	returnType = CVReturnType::RENDER;
	return returnType;
}


/// Build-in messaging system. Used for advanced applications to communicate with the game engine properly.
void CVGame::ProcessMessage(Message * message)
{
	game->ProcessMessage(message);
}


/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
void CVGame::SetEnabled(bool value)
{
	CVRenderFilter::SetEnabled(value);
	if (!value)
	{
		Physics.Pause();
	}
	else 
	{
		Physics.Resume();
	}
	/// Set pause-state.
	game->SetPause(!value);
	// Hide/Reveal entities 
	Graphics.QueueMessage(new GMSetEntityb(GetEntities(), GT_VISIBILITY, value, true));
}

/// Fetches all dynamically created entities associated with this filter. Must be overloaded.
List<Entity*> CVGame::GetEntities()
{
	return game->GetEntities();
}


// Should be overloaded? If you paint to the output-texture?
void CVGame::Paint(CVPipeline * pipe)
{
	// ,,
	if (previousFilter)
		previousFilter->Paint(pipe);
}
