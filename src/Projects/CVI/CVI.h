/// Emil Hedemalm
/// 2014-03-27
/// Computer Vision Imaging application main state/settings files.

#include "GameStates/GameState.h"
#include "Graphics/Camera/Camera.h"


class CVIState : public GameState 
{
public:
	/// Constructor
	CVIState();
	/// Virtual destructor to discard everything appropriately.
	virtual ~CVIState();
	/// Function when entering this state, providing a pointer to the previous StateMan.
	virtual void OnEnter(GameState * previousState);
	/// Main processing function, using provided time since last frame.
	virtual void Process(float time);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	virtual void OnExit(GameState * nextState);

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

	/** Function to handle custom actions defined per state.
		This function is called by the various bindings that the state defines.
	*/
	virtual void InputProcessor(int action, int inputDevice = 0);
	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();

	/// Creates input-bindings for camera navigation.
	void CreateCameraBindings();
	bool HandleCameraMessages(String message);

	/// Creates the user interface for this state
	virtual void CreateUserInterface();

	/// For handling drag-and-drop files.
	virtual void HandleDADFiles(List<String> & files);

	/// For rendering what we have identified in the target image.
	virtual void Render(GraphicsState & graphicsState);

	/// Analyze the current/target texture to find any visible lines.
	void ExtractLines();

private:

	List<Line> lines;

	/// Dedicated camera
	Camera cviCamera;
	/// Entity to display the texture in 3D-space
	Entity * textureEntity;
	/// Active texture we're manipulating.
	Texture * texture;
};

