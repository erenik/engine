/// Emil Hedemalm
/// 2014-03-27
/// Computer Vision Imaging application main state/settings files.

#include "GameStates/GameState.h"
#include "Graphics/Camera/Camera.h"
#include "Mutex/Mutex.h"
#include "CV/CVPipeline.h"
#include "Window/Window.h"

#include "opencv2/opencv.hpp"

#define PIPELINE_CONFIG_FILE_ENDING	".pcfg"

namespace CVInput
{
	enum 
	{
		WEBCAM,
		IMAGE_SERIES,
		TEXTURE,
	};
};

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

	void PrintProcessingTimes();
	/// Tests the pipeline, reloading base/original image.
	void TestPipeline();

	/** Function to handle custom actions defined per state.
		This function is called by the various bindings that the state defines.
	*/
	virtual void InputProcessor(int action, int inputDevice = 0);
	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();

	/// Reset/center camera
	void ResetCamera();

	/// Creates input-bindings for camera navigation.
	void CreateCameraBindings();
	bool HandleCameraMessages(String message);

	/// Creates the user interface for this state
	virtual void CreateUserInterface();

	/// For handling drag-and-drop files.
	virtual void HandleDADFiles(List<String> & files);

	/// For rendering what we have identified in the target image.
	virtual void Render(GraphicsState & graphicsState);

	/// Loads target image
	void LoadImage(String fromSource);
	/// Converts to black and white
	void ConvertToBW();
	void ScaleUp();
	void ScaleDown();
	/// Analyze the current/target texture to find any visible lines.
	void ExtractLines();
	/// Initial test of all stuff.
	void Test();
	/// Applies gaussian blur
	void Blur();
	/// Calculates canny edge detection
	void CannyEdge();
	/// Calculates Harris corner detection
	void CornerHarris();

	/// Call to process active image in the current pipeline, displaying error messages if applicable and rendering the results. Returns -1 on error.
	int ProcessPipeline();

	/// Sets edit mode, 0 for Testing, 1 for editing the Pipeline and 2 for selecting input?
	void SetEditMode(int mode);
	/// See CVInput enum above.
	void SetInput(int newInputMode);
private:

	/// Opens menu for selecting new filter to add to the filter-pipeline.
	void OpenFilterSelectionMenu();
	void OnFilterSelectionFilterUpdated();
	/// Called every time the pipeline is changed. Generates new UI for handling it.
	void OnPipelineUpdated();
	/// When selecting a filter for editing. Opens up UI and configures it appropriately.
	void OnFilterSelected(int index);

	/// Current filter being edited.
	CVFilter * currentEditFilter;

	/// Current editor mode, 0 for testing, 1 for pipeline, 2 for input-selection (drag-textures, stored image sequences, webcam, etc.)
	int currentMode;
	/// See CVInput namespace/enum above.
	int inputMode;
	/// If false will pause all dynamic inputs from processing. Default is true.
	bool inputAutoplay;
	/// If appending filter, if false will insert from top.
	int append;

	/// 0 for Image-filters, 1 for Data-filters and 2 for Render-filters.
	int filterSelectionFilter;

	/// Used to control access to the pipeline when deleting/creating filters primarily.
	Mutex pipelineMutex;

	/// Current/active pipeline for image processing.
	CVPipeline * pipeline;

	/// Video capture to grab webcam data via straight via CV.
	cv::VideoCapture cap;

	/// Set in the UI, for applying blur
	float blurSize;
	/// For CannyEdge
	int cannyThresholdLow;
	/// Free variable in CornerHarris calculation
	float cornerHarrisBlockSize;
	int cornerHarrisKSize;
	float cornerHarrisK;

	/// Replaces log-message in the UI
	void Log(String message);

	/// Fills the native Texture-class with new data and then calls OnTextureUpdated to update visuals.
	void OnCVImageUpdated();
	/// Called after a calculation procedure is done, will display the results instead of the base image.
	void OnCVResultImageUpdated();

	/// Update texture on the entity that is rendered.
	void OnTextureUpdated();

	/// Original is set when loading a new texture.
	cv::Mat cvOriginalImage;
	/// Current manipulated image.
	cv::Mat cvImage;
	/// Results after calculating e.g. Canny Edge detection on the cvImage
	cv::Mat cvResultImage;

	/// Dedicated camera
	Camera cviCamera;
	/// Entity to display the texture in 3D-space
	Entity * textureEntity;
	/// Active texture we're manipulating.
	Texture * texture;

	/// Dedicated output window to be used with a projector
	Window * projectionWindow;
};

