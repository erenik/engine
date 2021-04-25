/// Emil Hedemalm
/// 2015-01-16
/// Utility functions for handling cameras, including setting up and linking input to navigational actions.

#include "CameraUtil.h"
#include "Input/Keys.h"
#include "Input/Action.h"

/** Creates default input-bindings that may be used in an AppState for a game or general camera control.
	These should then be added to the AppState's InputMapping. Each binding will trigger a message that is posted to the messageManager,
	which the AppState may then react to in the ProcessMessage(Message*) functions.
*/
List<Binding*> CreateDefaultCameraBindings()
{
	/// Need to either filter out messages, or make some trigger only under special circumstances...
	Binding::requireNoCameraFocusEntity = true;

	List<Binding*> cameraBindings;
	cameraBindings.Add(new Binding(new Action("ResetCamera"), KEY::HOME),
		new Binding(Action::CreateStartStopAction("CameraForward"), KEY::W),
		new Binding(Action::CreateStartStopAction("CameraBackward"), KEY::S),
		new Binding(Action::CreateStartStopAction("CameraLeft"), KEY::A),
		new Binding(Action::CreateStartStopAction("CameraRight"), KEY::D));
	cameraBindings.Add(new Binding(Action::CreateStartStopAction("CameraUp"), KEY::E),
		new Binding(Action::CreateStartStopAction("CameraDown"), KEY::Q));
	cameraBindings.Add(new Binding(Action::CreateStartStopAction("CameraRotUp"), KEY::UP),
		new Binding(Action::CreateStartStopAction("CameraRotDown"), KEY::DOWN),
		new Binding(Action::CreateStartStopAction("CameraRotRight"), KEY::RIGHT),
		new Binding(Action::CreateStartStopAction("CameraRotLeft"), KEY::LEFT));

	cameraBindings.Add(new Binding(Action::FromString("CamIncreaseSpeed"), KEY::PLUS),
		new Binding(Action::FromString("CamDecreaseSpeed"), KEY::MINUS));

	Binding::requireNoCameraFocusEntity = false;
	return cameraBindings;
}

/// Default processor to react to messages created by the default bindings declared above.
void ProcessCameraMessages(String msg, Camera * forCamera)
{
	if (!forCamera->inputFocus)
		return;
	if (msg == "ResetCamera")
	{
		forCamera->Reset();		
	}
	else if (msg == "CamIncreaseSpeed")
	{
		forCamera->flySpeed = ClampedFloat(forCamera->flySpeed * 1.1f + 0.1f, 0.1f, 100.f);
	}
	else if (msg == "CamDecreaseSpeed")
	{
		forCamera->flySpeed = ClampedFloat(forCamera->flySpeed * 0.9f - 0.1f, 0.1f, 100.f);
	}
	else if (msg.Contains("StartCameraRot"))
	{
		String dirStr = msg - "StartCameraRot";
		Direction direction = GetDirection(dirStr);
		forCamera->BeginRotate(direction);	
	}
	else if (msg.Contains("StopCameraRot"))
	{
		String dirStr = msg - "StopCameraRot";
		Direction direction = GetDirection(dirStr);
		forCamera->EndRotate(direction);	
	}
	else if (msg.Contains("StartCamera"))
	{
		String dirStr = msg - "StartCamera";
		Direction direction = GetDirection(dirStr);
		forCamera->Begin(direction);
	}
	else if (msg.Contains("StopCamera"))
	{
		String dirStr = msg - "StopCamera";
		Direction direction = GetDirection(dirStr);
		forCamera->End(direction);
	}
}
