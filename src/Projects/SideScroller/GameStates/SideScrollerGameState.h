/// Emil Hedemalm
/// 2014-03-04
/// Base game state for a side-scroller game, that delivers camera functions and stuff.

#ifndef SIDE_SCROLLER_GAME_STATE_H
#define SIDE_SCROLLER_GAME_STATE_H

#include "GameStates/GameState.h"

class SideScrollerGameState : public GameState 
{
public:


	/// Creates camera key-bindings! :)
	void CreateCameraBindings();
	/// Call this in the ProcessMessage() if you want the base state to handle camera movement! Returns true if the message was indeed a camera-related message.
	bool HandleCameraMessages(String message);
	/// Modes for the camera. Creates and makes the camera active if it wasn't already.
	void SetCameraTrackEntity(Entity * entity);
	void SetCameraFreeFly();	
	void SetCameraOrthogonal();
	void SetCameraProjection3D();
	/// Assume one main camera for this game. Add more cameras in your states if you're gonna do magic stuff.
	static Camera * mainCamera;
 
private:
	/// Creates the camera.
	void CreateCamera();
	/// Registers it with the graphics-manager!
	void MakeCameraActive();
	/// Resets velocities and stuff
	void ResetCamera();
};

#endif