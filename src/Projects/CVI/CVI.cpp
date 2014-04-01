/// Emil Hedemalm
/// 2014-03-27
/// Computer Vision Imaging application main state/settings files.

#include "CVI.h"
#include "StateManager.h"
#include "Graphics/Messages/GraphicsMessages.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "TextureManager.h"
#include "Maps/MapManager.h"
#include "ModelManager.h"
#include "Input/Keys.h"
#include "Message/Message.h"

/// CV Includes
#include "opencv2/core/core_c.h"
// For reading files
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/highgui/highgui_c.h"

// For iamge manipulation (image processing)
#include "opencv2/imgproc/imgproc.hpp"
// For some enums
#include "opencv2/imgproc/types_c.h"

void RegisterStates()
{
	StateMan.RegisterState(new CVIState());
	StateMan.QueueState(GAME_STATE_EDITOR);
}

CVIState::CVIState()
{
    stateName = "Main CVI State";
//	keyPressedCallback = true;
	id = GAME_STATE_EDITOR;
	textureEntity = NULL;
	texture = NULL;
}

CVIState::~CVIState()
{

}

/// Function when entering this state, providing a pointer to the previous StateMan.
void CVIState::OnEnter(GameState * previousState)
{
	// Remove initial 
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));
	Graphics.QueueMessage(new GMSetUI(ui));

	// Fetch some default-texture
	texture = TexMan.GetTexture("logo.png");
	// 
	if (!textureEntity)
		textureEntity = MapMan.CreateEntity(ModelMan.GetModel("Sprite.obj"), texture);
	
	// Reset camera propertiiies.
	Graphics.cameraToTrack = &cviCamera;
	cviCamera.projectionType = Camera::ORTHOGONAL;
	cviCamera.rotation = Vector3f();
	cviCamera.position = Vector3f(0,0,-5);
	cviCamera.zoom = 5.0f;
	cviCamera.flySpeed = 5.0f;

	ExtractLines();
}

/// Main processing function, using provided time since last frame.
void CVIState::Process(float time)
{

}

/// Function when leaving this state, providing a pointer to the next StateMan.
void CVIState::OnExit(GameState * nextState)
{

}


/// Callback function that will be triggered via the MessageManager when messages are processed.
void CVIState::ProcessMessage(Message * message)
{
	if (HandleCameraMessages(message->msg))
		return;
}

/** Function to handle custom actions defined per state.
	This function is called by the various bindings that the state defines.
*/
void CVIState::InputProcessor(int action, int inputDevice /*= 0*/)
{

}

/// Creates default key-bindings for the state.
void CVIState::CreateDefaultBindings()
{
	InputMapping * im = &this->inputMapping;
	CreateCameraBindings();
}


/// Creates input-bindings for camera navigation.
void CVIState::CreateCameraBindings()
{
	inputMapping.CreateBinding("Up", KEY::W)->stringStopAction = "StopUp";
	inputMapping.CreateBinding("Down", KEY::S)->stringStopAction = "StopDown";
	inputMapping.CreateBinding("Left", KEY::A)->stringStopAction = "StopLeft";
	inputMapping.CreateBinding("Right", KEY::D)->stringStopAction = "StopRight";
	
	inputMapping.CreateBinding("Zoom in", KEY::PG_DOWN)->activateOnRepeat = true;
	inputMapping.CreateBinding("Zoom out", KEY::PG_UP)->activateOnRepeat = true;

	inputMapping.CreateBinding("Increase camera translation velocity", KEY::PLUS)->activateOnRepeat = true; 
	inputMapping.CreateBinding("Decrease camera translation velocity", KEY::MINUS)->activateOnRepeat = true;

	inputMapping.CreateBinding("ResetCamera", KEY::HOME);
	inputMapping.CreateBinding("SetProjection", KEY::F1);
	inputMapping.CreateBinding("SetOrthogonal", KEY::F2);
}

/// Call this in the ProcessMessage() if you want the base state to handle camera movement! Returns true if the message was indeed a camera-related message.
bool CVIState::HandleCameraMessages(String message)
{
	Camera * mainCamera = &cviCamera;

	if (mainCamera == NULL)
		return false;
	if (message == "Up")
		mainCamera->Begin(Direction::UP);
	else if (message == "Down")
		mainCamera->Begin(Direction::DOWN);
	else if (message == "Left")
		mainCamera->Begin(Direction::LEFT);
	else if (message == "Right")
		mainCamera->Begin(Direction::RIGHT);
	else if (message == "StopUp")
		mainCamera->End(Direction::UP);
	else if (message == "StopDown")
		mainCamera->End(Direction::DOWN);
	else if (message == "StopLeft")
		mainCamera->End(Direction::LEFT);
	else if (message == "StopRight")
		mainCamera->End(Direction::RIGHT);
	else if (message == "ResetCamera"){
		mainCamera->position = Vector3f();
	}
	else if (message == "SetProjection")
;//		SetCameraProjection3D();
	else if (message == "SetOrthogonal")
;//		SetCameraOrthogonal();
	else if (message == "Increase camera translation velocity")
	{
		mainCamera->flySpeed *= 1.25f;
	}
	else if (message == "Decrease camera translation velocity")
	{
		mainCamera->flySpeed *= 0.8f;
	}
	else if (message == "Zoom in")
	{
		mainCamera->zoom = mainCamera->zoom * 0.95f - 0.01f;
#define CLAMP_DISTANCE Clamp(mainCamera->zoom, 0.01f, 10000.0f);
		CLAMP_DISTANCE;
	}
	else if (message == "Zoom out"){
		mainCamera->zoom = mainCamera->zoom * 1.05f + 0.01f;
		CLAMP_DISTANCE;
	}
	else
		return false;
	return true;
}


/// Creates the user interface for this state
void CVIState::CreateUserInterface()
{
	std::cout<<"\nState::CreateUserInterface called for "<<stateName;
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/CVIMain.gui");
}

/// For handling drag-and-drop files.
void CVIState::HandleDADFiles(List<String> & files)
{
	// Check if image
	// If so, load it into the editor.
}


/// For rendering what we have identified in the target image.
void CVIState::Render(GraphicsState & graphicsState)
{
	// lalll
}


/// Analyze the current/target texture to find any visible lines.
void CVIState::ExtractLines()
{
	if (!texture)
		return;
	cv::Mat cvImage;
	cvImage = cv::imread(texture->source.c_str(), CV_LOAD_IMAGE_COLOR);
	
	// Convert to grayscale
	cv::Mat greyscaleVersion;
	cv::cvtColor(cvImage, greyscaleVersion, CV_BGR2GRAY);
	
	/// Save copy.
	cv::imwrite( (texture->source + "gray.png").c_str(), greyscaleVersion);
}

