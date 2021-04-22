/// Emil Hedemalm
/// 2015-02-18
/// Tools for setting up and managing input for a default scene editor camera (3D), via en Entity.

#include "Entity/EntityProperty.h"
#include "Graphics/Camera/Camera.h"

/// Creates an entity featuring the EditorCameraProperty. If entity argument is provided, link to the entity is given as well.
Camera * CreateEditorCamera(Entity** entityPointer = NULL);

class EditorCameraProperty : public EntityProperty 
{
public:
	EditorCameraProperty(Entity* owner = NULL);

	/// Upon being unregistered from rendering. Delete the camera.
	virtual void OnUnregistrationFromGraphics();

	/// Time passed in seconds..! Will steer if inputFocus is true.
	virtual void Process(int timeInMs);
	virtual void ProcessMessage(Message * message);


	void ToggleAutorun();

	/// o.o Default 5?
	float movementSpeed;

	Camera * CreateCamera();

protected:
	/** Checks states via InputManager. Regular key-bindings should probably still be defined in the main game state 
		and then passed on as messages to the character with inputFocus turned on.
	*/
	void ProcessInput(float timeInSeconds); 
	void UpdateVelocity(ConstVec3fr newVel);
	// Checks mouse position and casts a ray. Will return all entities along the ray, starting with the closest one.
	void UpdateTargetsByCursorPosition();

	// Targets dictated by the latest call to UpdateTargets
	List< Entity* > targets;
	Entity* primaryTarget;
	/// Set to be the first raycast target position when calling UpdateTargetsByCursorPosition
	Vector3f lastRaycastTargetPosition;

	/// For handling movement.
	Vector3f lastAcc;
	float lastRight;
	bool autoFly;

	Vector3f lastVelocity;
	
	/// o.o
	bool jumping;
	Time lastJump;

	/// o.o
	Camera * editorCamera;
};


