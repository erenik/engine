/// Emil Hedemalm
/// 2014-03-27
/// Computer Vision Imaging application main state/settings files.

#include "IPM.h"
#include "StateManager.h"
#include "Graphics/Messages/GraphicsMessages.h"
#include "Graphics/Messages/GraphicsMessage.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "TextureManager.h"
#include "Maps/MapManager.h"
#include "ModelManager.h"
#include "Input/Keys.h"
#include "Message/Message.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Graphics/Messages/GMUI.h"
#include "Message/VectorMessage.h"
#include "Input/InputManager.h"
#include "Graphics/FrameStatistics.h"
#include "UI/UIButtons.h"
#include "UI/UIList.h"
#include "UI/UIInput.h"
#include "UI/UIFileBrowser.h"
#include "Message/FileEvent.h"
#include <fstream>
#include "Window/WindowManager.h"
#include "Viewport.h"
#include "File/File.h"
#include "Message/MessageManager.h"
#include "Graphics/GraphicsProperty.h"
#include "Script/ScriptManager.h"
#include "CV/CVRenderFilters.h"
#include "File/FileUtil.h"

/// CV Includes
#include "opencv2/opencv.hpp"
#include "opencv2/core/core_c.h"
// For reading files
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/highgui/highgui_c.h"

// For iamge manipulation (image processing)
#include "opencv2/imgproc/imgproc.hpp"
// For some enums
#include "opencv2/imgproc/types_c.h"

// To get Sleep
#include "OS/Sleep.h"

#define CAM_Z -5

// Initial/default texture.
String initialTexture = "img/logo.png";

void RegisterStates()
{
	StateMan.RegisterState(new CVIState());
	StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_EDITOR));
}

CVIState::CVIState()
{
    name = "Main CVI State";
//	keyPressedCallback = true;
	id = GameStateID::GAME_STATE_EDITOR;
	textureEntity = NULL;
	texture = NULL;
	blurSize = 3;
	cannyThresholdLow = 10;
	cornerHarrisBlockSize = 5;
	cornerHarrisKSize = 5;
	cornerHarrisK = 0.04f;
	pipeline = new CVPipeline();
	currentMode = 0;
	inputMode = CVInput::TEXTURE;
	inputAutoplay = true;
	currentEditFilter = NULL;
	filterSelectionFilter = 0;

	pipelineTextureRenderedOnProjection = false;
	syncEntity = NULL;
	frameEntity = NULL;

	pipelineMutex.Create("CVPipelineMutex");

	// Create filter names in order to use the filter constructors.
	CVFilter::CreateFilterNames();

	projectionWindow = NULL;

	/// Dynamic texture used for manipulation and streaming input? To distinguish it from when testing on other static textures
	dynamicTexture = NULL;

	
	imageSeriesPaused = false;
	timePerImage = 50;

	cviCamera = projectionCamera = NULL;
}

CVIState::~CVIState()
{

	delete pipeline;
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void CVIState::OnEnter(GameState * previousState)
{
	if (!cviCamera)
		cviCamera = CameraMan.NewCamera();
	if (!projectionCamera)
		projectionCamera = CameraMan.NewCamera();

	// Remove initial 
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));
	Graphics.QueueMessage(new GMSetUI(ui));

	Window * window = MainWindow();
	window->backgroundColor = Vector4f(0.1f,0.1f,0.1f,1.f);

	// Enter test mode on start-up.
	SetEditMode(0);

	// Create display entity 
	if (!textureEntity)
	{
		textureEntity = MapMan.CreateEntity(ModelMan.GetModel("Sprite.obj"), TexMan.GetTexture(initialTexture));
		ToggleRenderPipelineTextureOnProjection();
	}

	// Load initial texture
	LoadImage(initialTexture);

	/// Set texture in UI so it looks correct.
	Graphics.QueueMessage(new GMSetUIs("MainTexture", GMUI::TEXTURE_INPUT_SOURCE, initialTexture));
	// Reset camera propertiiies.
	ResetCamera(cviCamera);
	ResetCamera(projectionCamera);
	cviCamera->name = "CVICamera";
	projectionCamera->name = "ProjectionCamera";
	// Set camera
	Graphics.QueueMessage(new GMSetCamera(cviCamera));
	
	cviCamera->AdjustProjectionMatrixToWindow(MainWindow());

	ExtractLines();

	Viewport * mainViewport = MainWindow()->MainViewport();
	mainViewport->renderGrid = false;
	mainViewport->renderFPS = false;

	/// For reducing performance demand
	Graphics.outOfFocusSleepTime = 50;
	Graphics.sleepTime = 15;

	Input.ForceNavigateUI(true);

	/// Apply proper visuals
	OnTextureUpdated();


	// Run startup.ini if it is available, running all its arguments through the message manager, one line at a time.
	
	Script::rootEventDir = "";
	Script * script = new Script();
	script->SetDeleteOnEnd(true);
	script->Load("onEnter.ini");
	ScriptMan.PlayScript(script);

	/*
	List<String> lines = File::GetLines("onEnter.ini");
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		if (line.StartsWith("//"))
			continue;
		MesMan.QueueMessages(line);
	}
	*/
}

bool Same(cv::Mat & one, cv::Mat & two)
{
	if (one.rows != two.rows)
		return false;

	int channels = one.channels();
	unsigned char * dataOne = one.data,
		* dataTwo = two.data;

	for (int y = 0; y < one.rows; y++)
	{
		for (int x = 0; x < one.cols; ++x)
		{
			for (int i = 0; i < channels; ++i)
			{
				int index = y * one.cols * channels + x * channels + i;
				if (dataOne[index] != dataTwo[index])
					return false;
			}
		}
	}
	return true;
}

/// Main processing function, using provided time since last frame.
void CVIState::Process(int timeInMs)
{
	/// If projection window is up, print debug stats of it and its rendering onto the sync page
	if (projectionWindow)
	{
		Graphics.QueueMessage(new GMSetUIv2i("WindowResolution", GMUI::VECTOR_INPUT, projectionWindow->WorkingArea()));
		if (syncEntity)
		{
			Vector2i size = projectionWindow->WorkingArea();
			Physics.QueueMessage(new PMSetEntity(SET_SCALE, syncEntity, Vector3f(size, 1)));
//			Physics.QueueMessage(new PMSetEntity(POSITION, syncEntity, Vector3f(size * 0.5f, 1)));
		}
	}

	/// Input modes and interactive/dynamic display below
	switch(inputMode)
	{
		case CVInput::TEXTURE:
			Sleep(100);
			return;
		default:
			Sleep(10);
	}

	bool newFrameGotten = false;
	/// Fetch new data for all interactive input-modes.
	switch(inputMode)
	{
		case CVInput::WEBCAM:
		{
			// Grab camera if not done already
			if (!cap.isOpened())
			{
				Log("Attempting to grab camera...");
				cap.open(0);
				if (!cap.isOpened()){
					Log("Unable to open camera for reading.");
					Sleep(100);
					return;
				}
				else {
					Log("Camera grabbed!");
				}
			}
			
			// Grab new frame from the capturer into the cvImage, and mark it for rendering later! ovo
			// Save old image.
// #define PROFILE_FPS
#ifdef PROFILE_FPS
			static cv::Mat lastFrame;
			cvImage.copyTo(lastFrame);
#endif
			cap >> cvImage;
			// Compare them
#ifdef PROFILE_FPS
			//... stuff.
			if (lastFrame.type() == cvImage.type() && 
				lastFrame.cols == cvImage.cols)
			{
				bool eq = false;
				if (Same(cvImage, lastFrame))
					eq = true;
		//		std::cout<<"\nEqual: "<<eq;
			}
			static int imagesCaptured = 0;
			static int64 lastSecond = 0;
			++imagesCaptured;
			int64 now = Timer::GetCurrentTime();
			if (now > lastSecond)
			{
				std::cout<<"\nImages captured this past second: "<<imagesCaptured;
				imagesCaptured = 0;
				lastSecond = now;
			}
#endif
			newFrameGotten = true;
			break;
		}
		case CVInput::IMAGE_SERIES: 
		{
			if (!imageSeriesPaused)
			{
				static int64 lastFrame = 0;
				int64 currentTime = Timer::GetCurrentTimeMs();
				if (currentTime > lastFrame + timePerImage)
				{
					++imageSeriesIndex;
					lastFrame = currentTime;
				}
				// No new image, wait til next frame is due
				else
					break;
			}
			// No new image, is paused.
			else
				break;
			// ciiiiircularrr
			if (imageSeriesIndex > filesInImageSeries.Size())
				imageSeriesIndex = 0;
			else if (imageSeriesIndex < 0)
				imageSeriesIndex = 0;
			if (!filesInImageSeries.Size())
			{
				std::cout<<"NO FILSE IN IAMGE AEKOWRKASLERIOES;";
				return;
			}
			/// Check if it's already loaded.
			if (imageSeriesImages.Size() > imageSeriesIndex)
			{
				cv::Mat & frame = imageSeriesImages[imageSeriesIndex];
				frame.copyTo(cvImage); 
				// Just use it!
				newFrameGotten = true;
				break;
			}
			/// Grab the current index
			String source = imageSeriesDir + "/" + filesInImageSeries[imageSeriesIndex];
			// Load it
			cv::Mat image = cv::imread(source.c_str());
			imageSeriesImages.Add(image);
			image.copyTo(cvImage);
			newFrameGotten = true;
			break;
		}
	}

	/// Check if we got a new frame before doing more processing.
	if (newFrameGotten)
	{
		/// If pipeline is selected/active, process it on the image!
		if (true){
			/// Do note that ProcessPipeline will automatically set up rendering and update the texture, which is why we don't call it here.
			int result = ProcessPipeline();
		}
		/// No pipeline active, just display new raw data.
		else 
		{
			OnCVImageUpdated();
		}
	}
	/// Display FPS
	Graphics.QueueMessage(new GMSetUIs("FPS", GMUI::TEXT, "FPS: "+String::ToString((int)FrameStats.FPS())));
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void CVIState::OnExit(GameState * nextState)
{
	// Remove UI.
	Graphics.QueueMessage(new GMSetUI(NULL));
}


/// Callback function that will be triggered via the MessageManager when messages are processed.
void CVIState::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::FILE_EVENT:
		{
			FileEvent * fe = (FileEvent*) message;
			if (msg == "SavePipelineConfig")
			{
				String fileName = fe->files[0];
				std::fstream file;
				if (!fileName.Contains(PIPELINE_CONFIG_FILE_ENDING))
					fileName += PIPELINE_CONFIG_FILE_ENDING;
				file.open(fileName.c_str(), std::ios_base::out | std::ios_base::binary);
				if (file.is_open()){
					pipelineMutex.Claim(-1);
					pipeline->WriteTo(file);
					pipelineMutex.Release();
					Log("Pipeline configuration saved to file: "+fileName);
				}
				else {
					Log("Unable to save to file: "+fileName);
				}
				file.close();
			}
			else if (msg == "LoadPipelineConfig")
			{
				// Close filter editor so no old stuff will be manipulated.
				Graphics.QueueMessage(new GMPopUI("FilterEditor", ui));

				String fileName = fe->files[0];
				std::fstream file;
				file.open(fileName.c_str(), std::ios_base::in | std::ios_base::binary);
				if (file.is_open()){
					pipelineMutex.Claim(-1);
					bool success = pipeline->ReadFrom(file);
					pipelineMutex.Release();
					if (!success){
						file.close();
						OnPipelineUpdated();
						Log ("Error loading file: "+pipeline->GetLastError());
						return;
					}
					else 
						Log("Pipeline configuration loaded from file: "+fileName);
				}
				else {
					Log("Unable to open file: "+fileName);
				}
				file.close();
				OnPipelineUpdated();
			}
			break;
		}
		case MessageType::SET_STRING: 
		{
			SetStringMessage * ssm = (SetStringMessage*) message;
			if (msg.Contains("SetString:"))
			{
				String settingName = msg.Tokenize(":")[1];
				CVFilterSetting * setting = currentEditFilter->GetSetting(settingName);
				assert(setting && "Setting not found. You missed something while coding");
				if (!setting)
					return;
				setting->sValue = ssm->value;
				TestPipeline();
			}
			else if (msg.Contains("SetImageSeriesDirectory"))
			{
				SetImageSeriesDirectory(ssm->value);
			}
			break;	
		}
		case MessageType::TEXTURE_MESSAGE:
		{
			TextureMessage * tm = (TextureMessage*) message;
			if (msg == "SetMainTexture")
			{
				LoadImage(tm->texSource);
				OnCVImageUpdated();
			}
			break;
		}
		case MessageType::INTEGER_MESSAGE:
		{
			IntegerMessage * im = (IntegerMessage*) message;
			// For pipeline filter setting editing
			if (msg.Contains("SetInteger:"))
			{
				String settingName = msg.Tokenize(":")[1];
				CVFilterSetting * setting = currentEditFilter->GetSetting(settingName);
				assert(setting && "Setting not found. You missed something while coding");
				if (!setting)
					return;
				setting->iValue = im->value;
				TestPipeline();
			}
			else if (msg.Contains("ImageSeriesFrame"))
			{
				imageSeriesIndex = im->value;
			}
			else if (msg.Contains("ImageSeriesDurationPerFrame"))
			{
				timePerImage = im->value;	
			}
		}
		case MessageType::FLOAT_MESSAGE: 
		{
			FloatMessage * fm = (FloatMessage*) message;
			// For pipeline filter setting editing
			if (msg.Contains("SetFloat:"))
			{
				String settingName = msg.Tokenize(":")[1];
				CVFilterSetting * setting = currentEditFilter->GetSetting(settingName);
				assert(setting && "Setting not found. You missed something while coding");
				if (!setting)
					return;
				setting->fValue = fm->value;
				TestPipeline();
			}
			// For manual testing
			else if (msg == "SetBlurSize")
				blurSize = fm->value;
			else if (msg == "SetCannyThreshold")
			{
				cannyThresholdLow = (int)fm->value;
				CannyEdge();
			}
			else if (msg == "SetHarrisBlockSize"){
				cornerHarrisBlockSize = fm->value;
				CornerHarris();
			}
			else if (msg == "SetHarrisKSize")
			{
				cornerHarrisKSize = (int)fm->value;
				CornerHarris();
			}
			else if (msg == "SetHarrisKFree"){
				cornerHarrisK = fm->value;
				CornerHarris();
			}
		}
		case MessageType::STRING: 
		{
			if (false){}

			/// Image series!
			else if (msg.Contains("SetImageSeriesDirectory("))
			{
				String dir = msg.Tokenize("()")[1];
				SetImageSeriesDirectory(dir);
			}
			else if (msg == "PauseImageSeries")
			{
				imageSeriesPaused = !imageSeriesPaused;
			}
			// Window management
			else if (msg.Contains("CreateDedicatedOutputWindow"))
			{
				CreateProjectionWindow();
				// Bring to front?
				projectionWindow->Show();
				projectionWindow->BringToTop();
			}
			else if (msg == "ToggleRenderOntoEditor")
			{
				// Grab active filter
				if (this->currentEditFilter->Type() != CVFilterType::RENDER_FILTER)
					return;
				CVRenderFilter * rf = (CVRenderFilter*)this->currentEditFilter;
				List<Entity*> entities = rf->GetEntities();
				rf->renderOntoEditor = !rf->renderOntoEditor;
				/// Get first state?
				if (rf->renderOntoEditor)
					Graphics.QueueMessage(new GMSetEntity(entities, ADD_CAMERA_FILTER, cviCamera));
				else 
					Graphics.QueueMessage(new GMSetEntity(entities, REMOVE_CAMERA_FILTER, cviCamera));
			}
			else if (msg == "HideWindows")
			{
				if (projectionWindow)
					projectionWindow->Hide();
			}
			else if (msg == "DeleteWindows")
			{
				if (projectionWindow)
					projectionWindow->Destroy();
			}
			else if (msg.Contains("MoveProjectionWindowTo(SecondaryMonitor)"))
			{
				projectionWindow->MoveToMonitor(1);
			}
			else if (msg.Contains("ToggleProjectionRenderPipelineTexture"))
			{
				ToggleRenderPipelineTextureOnProjection();
			}
			else if (msg == "ToggleFrame")
			{
				if (frameEntity)
				{
					if (frameEntity->graphics)
						Graphics.QueueMessage(new GMSetEntityb(frameEntity, VISIBILITY, !frameEntity->graphics->visible));
					else
						Graphics.QueueMessage(new GMSetEntityb(frameEntity, VISIBILITY, false));
				}
			}
			else if (msg == "ResetProjectionCamera")
			{
				ResetProjectionCamera();
			}
			else if (msg == "ToggleProjectionImage")
			{
				if (!syncEntity)
					return;
				if (syncEntity->graphics)
				{
					Graphics.QueueMessage(new GMSetEntityb(syncEntity, VISIBILITY, !syncEntity->graphics->visible));
				}
			}
			else if (msg.Contains("LoadPipelineConfig("))
			{
				String fileName = msg.Tokenize("()")[1];
				std::fstream file;
				file.open(fileName.c_str(), std::ios_base::in | std::ios_base::binary);
				if (file.is_open()){
					pipelineMutex.Claim(-1);
					bool success = pipeline->ReadFrom(file);
					pipelineMutex.Release();
					if (!success){
						file.close();
						OnPipelineUpdated();
						Log ("Error loading file: "+pipeline->GetLastError());
						return;
					}
					else 
						Log("Pipeline configuration loaded from file: "+fileName);
				}
				else {
					Log("Unable to open file: "+fileName);
				}
				file.close();
				OnPipelineUpdated();
			}
			/// For synchronization 
			else if (msg.Contains("ProjectSynchronizationImage"))
			{
				List<String> args = msg.Tokenize("()");
				String type = "RedGrid";
				if (args.Size() >= 2)
				{
					type = args[1];
				}
				ProjectSynchronizationImage(type);
			}
			else if (msg.Contains("ExtractFrame"))
			{
				static String usingPipeline = "data/140613_OutputDetectionRed.pcfg";
				List<String> args = msg.Tokenize("()");
				if (args.Size() >= 2)
				{
					usingPipeline = args[1];
				}
				ExtractProjectionOutputFrame(usingPipeline);
			}
			else if (msg == "ReloadOnEnterScript")
			{
				Script * script = new Script();
				bool loaded = script->Load("onEnter.ini");
				assert(loaded);
				ScriptMan.PlayScript(script);
			}
			else if (msg.Contains("AdjustCameraToFrame"))
			{
				AdjustProjectionCameraToFrame();
			}
			/// Edit-mode selection
			else if (msg.Contains("ActivateButtonSetting:"))
			{
				// Sent from a checkbox.. probably.
				String settingName = msg.Tokenize(":")[1];
				CVFilterSetting * setting = currentEditFilter->GetSetting(settingName);
				assert(setting && "Setting not found. You missed something while coding");
				if (!setting)
					return;
				setting->bValue = true;
				TestPipeline();
			}
			else if (msg.Contains("SetBool:"))
			{
				// Sent from a checkbox.. probably.
				String settingName = msg.Tokenize(":")[1];
				CVFilterSetting * setting = currentEditFilter->GetSetting(settingName);
				assert(setting && "Setting not found. You missed something while coding");
				if (!setting)
					return;
				setting->bValue = message->element->toggled;
				TestPipeline();
			}
			else if (msg == "ManualTest")
			{
				SetEditMode(TESTING);
			}
			else if (msg == "Pipeline")
			{
				SetEditMode(PIPELINE);
			}
			else if (msg == "InputSelection")
			{
				SetEditMode(INPUT);
			}
			else if (msg == "Synchronization")
			{
				SetEditMode(SYNC);
			}
			/// Filter-pipeline editing
			else if (msg == "Save")
			{
				// Open file dialogue for saving pipeline configuration.
				UIFileDialogue * saveDialogue = new UIFileDialogue("Save pipeline configuration", "SavePipelineConfig", PIPELINE_CONFIG_FILE_ENDING);
				saveDialogue->CreateChildren();
				saveDialogue->SetPath("data", false);
				saveDialogue->LoadDirectory(false);
				Graphics.QueueMessage(new GMAddUI(saveDialogue));
				Graphics.QueueMessage(new GMPushUI(saveDialogue, ui));
			}
			else if (msg == "Load")
			{
				// Open file dialogue for loading pipeline configuration.
				UIFileDialogue * saveDialogue = new UIFileDialogue("Load pipeline configuration", "LoadPipelineConfig", PIPELINE_CONFIG_FILE_ENDING);
				saveDialogue->CreateChildren();
				saveDialogue->SetPath("data", false);
				saveDialogue->LoadDirectory(false);
				Graphics.QueueMessage(new GMAddUI(saveDialogue));
				Graphics.QueueMessage(new GMPushUI(saveDialogue, ui));
			}
			else if (msg.Contains("ToggleFilter:"))
			{
				String indexString = msg.Tokenize(":")[1];
				int index = indexString.ParseInt();
				CVFilter * filter = pipeline->filters[index];
				filter->SetEnabled(!filter->enabled);
				// Clear times in pipeline too.
				pipeline->totalProcessingTimes.ClearAndDelete();

				/// Update the enabled-text of the filter.
				Graphics.QueueMessage(new GMSetUIs("ToggleFilterButton"+indexString, GMUI::TEXT, filter->enabled? "Disable" : "Enable"));
				/// Test the pipeline again.
				TestPipeline();
			}
			else if (msg == "InsertFilter")
			{
				append = false;	
				OpenFilterSelectionMenu();
			}
			else if (msg == "AppendFilter")
			{
				append = true;
				OpenFilterSelectionMenu();
			}
			else if (msg == "ImageFilters")
			{
				filterSelectionFilter = CVFilterType::IMAGE_FILTER;
				OnFilterSelectionFilterUpdated();
			}
			else if (msg == "DataFilters")
			{
				filterSelectionFilter = CVFilterType::DATA_FILTER;
				OnFilterSelectionFilterUpdated();
			}
			else if (msg == "RenderFilters")
			{
				filterSelectionFilter = CVFilterType::RENDER_FILTER;
				OnFilterSelectionFilterUpdated();
			}
			else if (msg.Contains("AddFilter:"))
			{
				String filterName = msg.Tokenize(":")[1];
				CVFilter * newFilter = CreateFilterByName(filterName);
				/// Add it to the pipeline.
				if (newFilter)
				{
					pipelineMutex.Claim(-1);
					if (append)
						pipeline->filters.Add(newFilter);
					else
						pipeline->filters.Add(newFilter, 0);
					pipelineMutex.Release();
					OnPipelineUpdated();
				}
			}
			else if (msg.Contains("MoveFilterUp:"))
			{
				/// Get index after the colon.
				String indexString = msg.Tokenize(":")[1];
				int index = indexString.ParseInt();
				if (index < 1)
					return;
				pipelineMutex.Claim(-1);
				pipeline->filters.Swap(index, index-1);
				pipelineMutex.Release();
				OnPipelineUpdated();
			}
			else if (msg.Contains("MoveFilterDown:"))
			{
				/// Get index after the colon.
				String indexString = msg.Tokenize(":")[1];
				int index = indexString.ParseInt();
				if (index >= pipeline->filters.Size() - 1)
					return;
				pipelineMutex.Claim(-1);
				pipeline->filters.Swap(index, index+1);
				pipelineMutex.Release();
				OnPipelineUpdated();
			}
			else if (msg.Contains("EditFilter:"))
			{
				/// Get index after the colon.
				String indexString = msg.Tokenize(":")[1];
				int index = indexString.ParseInt();
				OnFilterSelected(index);
			}
			else if (msg.Contains("DeleteFilter:"))
			{
				/// Get index after the colon.
				String indexString = msg.Tokenize(":")[1];
				int index = indexString.ParseInt();
				pipelineMutex.Claim(-1);
				CVFilter * filterDeleted = pipeline->DeleteFilterByIndex(index);
				if (filterDeleted == currentEditFilter)
				{
					// Hide the UI for the filter editor.
					currentEditFilter = NULL;
					Graphics.QueueMessage(new GMPopUI("FilterEditor", ui));
				}
				pipelineMutex.Release();
				OnPipelineUpdated();
			}
			else if (msg == "ClearFilters")
			{
				pipelineMutex.Claim(-1);
				pipeline->filters.ClearAndDelete();
				pipelineMutex.Release();
				OnPipelineUpdated();
				// Hide the UI for the filter editor.
				currentEditFilter = NULL;
				Graphics.QueueMessage(new GMPopUI("FilterEditor", ui));
			}
			/// Input-selection, webcam, etc.
			else if (msg == "Webcam")
			{
				SetInput(CVInput::WEBCAM);
			}
			else if (msg == "ImageSeries")
			{
				SetInput(CVInput::IMAGE_SERIES);
			}
			else if (msg == "Texture")
			{
				SetInput(CVInput::TEXTURE);
				// Load initial texture
				LoadImage(initialTexture);
				OnCVImageUpdated();
			}
			/// Test and standard stuff below.
			else if (msg == "OnReloadUI")
			{
				SetEditMode(currentMode);
				OnPipelineUpdated();
			}
			else if (msg == "Lines")
			{
				ExtractLines();
			}
			else if (msg == "Greyscale")
			{
				ConvertToBW();
			}
			else if (msg == "ScaleUp")
				ScaleUp();
			else if (msg == "ScaleDown")
				ScaleDown();
			else if (msg == "Test")
			{
				Test();
			} 
			else if (msg == "Blur")
				Blur();
			else if (msg == "Canny")
				CannyEdge();
			else if (msg == "CornerHarris")
			{
				CornerHarris();
			}
			else if (msg == "ReloadOriginal")
			{
				cvOriginalImage.copyTo(cvImage);
				OnCVImageUpdated();
				Log("Original reloaded");
			}
			else if (msg == "PrintScreenshot")
			{
				// Do stuff
				std::cout<<"\nWooo.";
			}
			else if (msg == "TestPipeline")
			{
				TestPipeline();
				// Check time!
				PrintProcessingTimes();
			}
			else if (msg == "Print Processing time")
			{
				PrintProcessingTimes();
			}
			break;	
		}
	}
	
	if (HandleCameraMessages(message->msg))
		return;
}

void CVIState::PrintProcessingTimes()
{
	pipeline->PrintProcessingTime();

	/*
	int totalProcess = 0;
	int totalRender = 0;
	for (int i = 0; i < pipeline->filters.Size(); ++i)
	{
		CVFilter * filter = pipeline->filters[i];
		totalProcess += filter->processingTime;
		totalRender += filter->renderTime;
		std::cout<<"\nFilter: "<<filter->name<<" processingTime: "<<filter->processingTime<<" renderTime: "<<filter->renderTime;
	}
	std::cout<<"\nTotal process: "<<totalProcess<<" Total Render: "<<totalRender;
	std::cout<<"\nTotal pipeline time consumption: "<<pipeline->pipelineTimeConsumption;
	*/
}

/// Tests the pipeline, reloading base/original image.
void CVIState::TestPipeline()
{
	/// Check what input is active. If we have a dynamic input the pipeline will be tested automatically each frame, making this call unnecessary.
	if (inputAutoplay == true )
	{
		switch(inputMode)
		{
			case CVInput::IMAGE_SERIES:
				if (imageSeriesPaused)
					break;
			case CVInput::WEBCAM:
				return;
		}
	}
	Log("Testing pipeline...");
	// Copy previous image.
	cvOriginalImage.copyTo(cvImage);
	// Reload original image before testing?
	int output = ProcessPipeline();
	if (output != CVOutputType::NO_OUTPUT){
		Log("Pipeline working.");
	}
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
	
	im->CreateBinding("Print Processing time", KEY::P);

	/// Save!
	im->CreateBinding("Save", KEY::CTRL, KEY::S);
	im->CreateBinding("Load", KEY::CTRL, KEY::L);

	CreateCameraBindings();

	im->CreateBinding("PrintScreenshot", KEY::PRINT_SCREEN);
}

/// Reset/center camera
void CVIState::ResetCamera(Camera * camera)
{
	camera->projectionType = Camera::ORTHOGONAL;
	camera->rotation = Vector3f();
	camera->position = Vector3f(0,0,-5);
	camera->zoom = 1.f;
	camera->scaleSpeedWithZoom = true;
	camera->position = Vector3f(0, 0, CAM_Z);
	camera->flySpeed = 100.f;

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

	inputMapping.CreateBinding("ToggleProjectionRenderPipelineTexture", KEY::CTRL, KEY::T, KEY::R);
}

/// Call this in the ProcessMessage() if you want the base state to handle camera movement! Returns true if the message was indeed a camera-related message.
bool CVIState::HandleCameraMessages(String message)
{
	Window * window = WindowMan.GetCurrentlyActiveWindow();
	if (!window)
		return false;
	Camera * camera = window->MainViewport()->camera;
	if (!camera)
		return false;

	if (message == "Up")
		camera->Begin(Direction::UP);
	else if (message == "Down")
		camera->Begin(Direction::DOWN);
	else if (message == "Left")
		camera->Begin(Direction::LEFT);
	else if (message == "Right")
		camera->Begin(Direction::RIGHT);
	else if (message == "StopUp")
		camera->End(Direction::UP);
	else if (message == "StopDown")
		camera->End(Direction::DOWN);
	else if (message == "StopLeft")
		camera->End(Direction::LEFT);
	else if (message == "StopRight")
		camera->End(Direction::RIGHT);
	else if (message == "ResetCamera"){
		ResetCamera(camera);
	}
	else if (message == "SetProjection")
;//		SetCameraProjection3D();
	else if (message == "SetOrthogonal")
;//		SetCameraOrthogonal();
	else if (message == "Increase camera translation velocity")
	{
		camera->flySpeed *= 1.25f;
	}
	else if (message == "Decrease camera translation velocity")
	{
		camera->flySpeed *= 0.8f;
	}
	else if (message == "Zoom in")
	{
		camera->zoom = camera->zoom * 0.95f - 0.01f;
#define CLAMP_DISTANCE ClampFloat(camera->zoom, 0.01f, 10000.0f);
		CLAMP_DISTANCE;
	}
	else if (message == "Zoom out"){
		camera->zoom = camera->zoom * 1.05f + 0.01f;
		CLAMP_DISTANCE;
	}
	else
		return false;
	return true;
}


/// Creates the user interface for this state
void CVIState::CreateUserInterface()
{
	std::cout<<"\nState::CreateUserInterface called for "<<name;
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/CVIMain.gui");
}

/// For handling drag-and-drop files.
void CVIState::HandleDADFiles(List<String> & files)
{
	if (files.Size())
	{
		// Switch to texture mode
		SetInput(CVInput::TEXTURE);
		// Check if image
		String fileName = files[0];
		if (!fileName.Contains(".png"))
			return;
		// If so, load it into the editor.
		LoadImage(fileName);
		OnCVImageUpdated();
		TestPipeline();
	}	
}


/// For rendering what we have identified in the target image.
void CVIState::Render(GraphicsState & graphicsState)
{
	// lalll

}


bool TextureToCVMat(Texture * texture, cv::Mat * mat)
{
	mat->create(cv::Size(texture->width, texture->height), CV_8UC3);
	int channels = 3;
	int bytesPerChannel = 1;
	unsigned char * data = mat->data;
	for (int y = 0; y < mat->rows; ++y)
	{
		for (int x = 0; x < mat->cols; ++x)
		{
			
			Vector4f pixel = texture->GetPixel(x,y);
			/// Pixel start index.
			int psi = (mat->step * y) + (x * channels) * bytesPerChannel;
			/// Depending on the step count...
			switch(channels)
			{			
				/// RGB!
				case 3:
					data[psi+0] = (unsigned char) (pixel.z * 255.f);
					data[psi+1] = (unsigned char) (pixel.y * 255.f);
					data[psi+2] = (unsigned char) (pixel.x * 255.f);
					break;
				// Default gray scale?
				default:
					break;
			}
		}
	}
	return true;
}


/// Loads target image
void CVIState::LoadImage(String fromSource)
{
	Texture * newTexture = TexMan.GetTexture(fromSource);
	if (!newTexture)
		return;
	texture  = newTexture;
	Graphics.QueueMessage(new GMSetEntityTexture(textureEntity, DIFFUSE_MAP, texture));

	try {
		// Load cvOriginalImage using the loaded texture
		TextureToCVMat(texture, &cvOriginalImage);
	//	String sourceWithBackslashes = texture->source;
	//	sourceWithBackslashes.Replace('/', '\\');
	//	cvOriginalImage = cv::imread(sourceWithBackslashes.c_str());
	//	cvOriginalImage = cv::imread(texture->source.c_str());
	} catch(...)
	{
		std::cout<<"\nNo workie. D:";
	}
	cvOriginalImage.copyTo(cvImage);

	if (cvOriginalImage.rows)
		Log("Image loaded from source: "+fromSource);
	else 
		Log("No rows in image. Might be bad file or library error.");
}

void LoadCVMatIntoTexture(cv::Mat * mat, Texture * texture)
{

	assert(mat && texture);
	/// First update size of our texture depending on the given one.
	if (texture->width != mat->cols || texture->height != mat->rows)
	{
		texture->Resize(Vector2i(mat->cols, mat->rows));
	}
	
//	std::cout<<"\nMat step: "<<mat->step;
	int channels = mat->channels();
	/// Depending on the depth, parse differently below.
	int channelDepth = mat->depth();
	int bytesPerChannel = 1;
	switch(channelDepth)
	{
		case CV_8U: case CV_8S: 
			bytesPerChannel = 1;
			break;
		case CV_16U: case CV_16S: 
			// Convert to single-channel if needed...
			mat->convertTo(*mat, CV_8UC3);
			bytesPerChannel = 1;
			break;
		case CV_32S: case CV_32F: 
			bytesPerChannel = 4;
			break;
		default:
			return;
	}

	/// Fetch data pointer now that any needed conversions are done.
	unsigned char * data = (unsigned char*) (mat->data);
	float minFloat = 0, maxFloat = 0;	
	
	for (int y = 0; y < mat->rows; ++y)
	{
		for (int x = 0; x < mat->cols; ++x)
		{
			unsigned char b,g,r;
			/// Pixel start index.
			int psi = (mat->step * y) + (x * channels) * bytesPerChannel;
			/// Depending on the step count...
			switch(channels)
			{			
				case 1:
				{
					if (bytesPerChannel == 1)
						b = g = r = data[psi];
					else if (bytesPerChannel == 4)
					{
						float * fPtr = (float*)&data[psi];
						float fValue = *fPtr;
						if (fValue > maxFloat)
							maxFloat = fValue;
						if (fValue < minFloat)
							minFloat = fValue;
						unsigned char cValue = (unsigned char) fValue;
						b = g = r = cValue;
					}
					break;
					
				}
				/// RGB!
				case 3:
					b = data[psi+0];
					g = data[psi+1];
					r = data[psi+2];
					break;
				// Default gray scale?
				default:
					b = g = r = data[psi];
					break;
			}
			texture->SetPixel(x, mat->rows - y - 1, Vector4f(r / 255.f,g / 255.f,b / 255.f,1));
		}
	}

//	std::cout<<"\nMin float: "<<minFloat<<" Max float: "<<maxFloat;

	/// Save it to file for debuggin too?
//	texture->Save(texture->source+"nativeBWd.png", true);

	/// Mark it as dynamic so buffering works properly...
	texture->dynamic = true;

	/// Send a message so that the texture is re-buffered.
	Graphics.QueueMessage(new GMBufferTexture(texture));
}

/// Converts to black and white
void CVIState::ConvertToBW()
{	
	if (cvImage.rows == 0)
	{
		Log("Nothing to convert.");	
		return;
	}
	// Convert to grayscale
	cv::Mat greyscaleVersion;
	std::cout<<"\nConverting to greyscale.";
	
	cv::cvtColor(cvImage, greyscaleVersion, CV_BGR2GRAY);

	// Update our relevant image with new greyscale one.
	cvImage = greyscaleVersion;

	// Update textures and ui.
	OnCVImageUpdated();
	Log("Conversion done");
}

void CVIState::ScaleUp()
{	
	/*
	/// Don't scale up if already huge...
	if (cvImage.cols * cvImage.rows > 2048 * 2048){
		Log("Skipping. Image size already above 4 million pixels.");
		return;
	}
	
	Vector2i newScale(cvImage.cols*2, cvImage.rows*2);
	Log("Scaling up to: "+String::ToString(newScale.x)+"x"+String::ToString(newScale.y));
	// Convert to grayscale
	cv::Mat upScaled;
#ifdef WINDOWS
	cv::pyrUp(cvImage, upScaled, cv::Size( newScale.x, newScale.y));	
#endif
	cvImage = upScaled;

	/// Update displayed texture.
	OnCVImageUpdated();
	*/
}

void CVIState::ScaleDown()
{
	/*
	// Convert to grayscale
	cv::Mat downScaled;
	Vector2i newScale((int)cvImage.cols*0.5f, (int)cvImage.rows*0.5f);
	if (newScale.GeometricSum() <= 0)
		return;
	Log("Scaling down to: "+String::ToString(newScale.x)+"x"+String::ToString(newScale.y));
#ifdef WINDOWS
	cv::pyrDown(cvImage, downScaled, cv::Size( newScale.x, newScale.y));	
#endif
	cvImage = downScaled;

	/// Update displayed texture.
	OnCVImageUpdated();
	*/
}


/// Analyze the current/target texture to find any visible lines.
void CVIState::ExtractLines()
{
	if (!texture)
		return;
	cv::Mat cvImage;
	cvImage = cv::imread(texture->source.c_str(), CV_LOAD_IMAGE_COLOR);

}

/// Initial test of all stuff.
void CVIState::Test()
{
	cv::Mat cvImage;
	cvImage = cv::imread(texture->source.c_str(), CV_LOAD_IMAGE_COLOR);
	
	// Convert to grayscale
	cv::Mat greyscaleVersion;
	cv::cvtColor(cvImage, greyscaleVersion, CV_BGR2GRAY);
	
	/// Save copy.
	cv::imwrite( (texture->source + "gray.png").c_str(), greyscaleVersion);
}

/// Applies gaussian blur
void CVIState::Blur()
{
	/*
	/// Blur size has to be odd, so increase it if needed by 1.
	int blurSizeOdd = blurSize;
	if (blurSizeOdd % 2 == 0)
		blurSizeOdd +=1;
	cv::Mat blurredImage;
	cv::GaussianBlur(cvImage, blurredImage, cv::Size(blurSizeOdd, blurSizeOdd), 0, 0);
	cvImage = blurredImage;
	OnCVImageUpdated();
	*/
}

/// Calculates canny edge detection
void CVIState::CannyEdge()
{
	// Make greyscale if not already.
	if (cvImage.channels() > 1)
		ConvertToBW();

	int edgeThresh = 1;
	int max_lowThreshold = 100;
	
	int ratio = 3;
	int kernel_size = 3;
	
	// Adjustments as needed.
	if (cannyThresholdLow > max_lowThreshold)
		cannyThresholdLow = max_lowThreshold;
	
	cv::Mat detected_edges;
	cv::Canny(cvImage, detected_edges, cannyThresholdLow, cannyThresholdLow * ratio, kernel_size);

	/// Do something with the result, preparing for masking...
	cvResultImage = cv::Scalar::all(0);

	/// Copy using the detected edges as a mask.
	cvImage.copyTo(cvResultImage, detected_edges);

//	cv::imwrite("CannyResults.png", cvResultImage);

	OnCVResultImageUpdated();
}

/// Calculates Harris corner detection
void CVIState::CornerHarris()
{
	/*
	// Make greyscale if not already.
	if (cvImage.channels() > 1)
		ConvertToBW();


	cv::Mat detected_corners;

	if (cornerHarrisKSize > 31)
		cornerHarrisKSize = 31;
	if (cornerHarrisKSize % 2 == 0)
		cornerHarrisKSize++;
	/// Result is a 32-bit floating point signed image
	cv::cornerHarris(cvImage, detected_corners, cornerHarrisBlockSize, cornerHarrisKSize, cornerHarrisK);

	// Do some magic... 
	cv::Mat thresholdCulledCorners;
	threshold(detected_corners, thresholdCulledCorners, 0.00001, 255, cv::THRESH_BINARY_INV);

	// Convert to gray-scale.

	/// Do something with the result, preparing for masking...
//	cvResultImage = cv::Scalar::all(0);


	/// Copy to result image for display
	thresholdCulledCorners.copyTo(cvResultImage);
	/// Make it greyscale afterwards?
	
	//	thresholdCulledCorners.copyTo(cvResultImage);

	// Display it
	OnCVResultImageUpdated();
	*/
}

/// Call to process active image in the current pipeline, displaying error messages if applicable and rendering the results. Returns -1 on error.
int CVIState::ProcessPipeline()
{
	pipelineMutex.Claim(-1);
	int output = pipeline->Process(&cvImage);
	pipelineMutex.Release();
	// Render stuff
	switch(output)
	{
		case CVReturnType::NO_OUTPUT:
		{
			if (pipeline->Filters() == 0)
			{
				Log("No filters in pipeline, displaying raw data/last texture.");
				OnCVImageUpdated();
				Sleep(100);
				return output;
			}
			Log(pipeline->GetLastError());
			break;
		}
		default:
		case CVReturnType::LINES:
		case CVReturnType::BOUNDING_BOX:
		case CVReturnType::POINTS:
		case CVReturnType::CV_CONTOURS:
		case CVReturnType::CV_CONVEX_HULLS:
		case CVReturnType::CV_IMAGE:
			Log("");
			break;
	}

	pipeline->output.copyTo(cvImage);
	OnCVImageUpdated();	
	Graphics.QueueMessage(new GMSetUIs("PipelineTimeConsumption", GMUI::TEXT, "PipelineTimeConsumption (us): "+String::ToString(pipeline->pipelineTimeConsumption)));
		
	return output;
}

/// Sets edit mode, 0 for Testing, 1 for editing the Pipeline and 2 for selecting input?
void CVIState::SetEditMode(int mode)
{
#define FILTER_SELECTION_MENU String("FilterSelectionMenu")

	List<List<String> > modeUIs;
	List<String> uiNames;
	/// Testing
	uiNames.Add("TestMenu");
	modeUIs.Add(uiNames);
	uiNames.Clear();
	/// Pipeline editing
	uiNames.Add("PipelineMenu");
	uiNames.Add(FILTER_SELECTION_MENU);
	uiNames.Add("FilterEditor");
	modeUIs.Add(uiNames);
	uiNames.Clear();
	/// Input stream selection
	uiNames.Add("InputMenu");
	modeUIs.Add(uiNames);
	uiNames.Clear();
	/// Synchronization 
	uiNames.Add("SyncMenu");
	modeUIs.Add(uiNames);
	uiNames.Clear();

	// Pop and hide other UIs
	for (int m = 0; m < modeUIs.Size(); ++m)
	{
		List<String> modeUINames = modeUIs[m];
		for (int i = 0; i < modeUINames.Size(); ++i)
		{
			Graphics.QueueMessage(new GMPopUI(modeUINames[i], ui));
		}
	}
	// Push the ones for the new state?
	currentMode = mode;
	List<String> modeUINames = modeUIs[currentMode];
	Graphics.QueueMessage(new GMPushUI(modeUINames[0], ui));
	
	/// Update ui appropriately
	switch(mode)
	{
	}
}

/// See CVInput enum above.
void CVIState::SetInput(int newInputMode)
{
	inputMode = newInputMode;
	switch(inputMode)
	{
		case CVInput::WEBCAM:
		{
			break;
		}
		case CVInput::IMAGE_SERIES:
		{
			break;
		}
		case CVInput::TEXTURE:
		{
			break;
		}
	}
}

/// For toggling if the debug pipeline output should be rendered onto the projection viewport.
void CVIState::ToggleRenderPipelineTextureOnProjection()
{
	// LALL
	pipelineTextureRenderedOnProjection = !pipelineTextureRenderedOnProjection;
	// Filter works by exlusion by default.
	if (!pipelineTextureRenderedOnProjection)
	{
		// Make it so that the texture entity with debug data is NOT rendered onto the projection surface!
		Graphics.QueueMessage(new GMSetEntity(textureEntity, CAMERA_FILTER, projectionCamera));
	}
	else 
	{
		Graphics.QueueMessage(new GMSetEntity(textureEntity, CLEAR_CAMERA_FILTER));
	}
}


/// Opens menu for selecting new filter to add to the filter-pipeline.
void CVIState::OpenFilterSelectionMenu()
{
	// Populate filters-list.
	OnFilterSelectionFilterUpdated();
	Graphics.QueueMessage(new GMPushUI(FILTER_SELECTION_MENU, ui));
}

void CVIState::OnFilterSelectionFilterUpdated()
{
	/// Populate the list.
	Graphics.QueueMessage(new GMClearUI("FilterSelectionList"));
	/// Add new filters.
	List<String> filterList = CVFilter::filterNames;
	for (int i = 0; i < filterList.Size(); ++i)
	{
		CVFilter * filter = GetSampleFilter(i);
		if (!filter)
			continue;
		String filterName = filterList[i];
		if (!filterName.Length())
			continue;
		if (filter->Type() != filterSelectionFilter)
			continue;
		UIButton * button = new UIButton("AddFilter:"+filterName);
		button->text = filterName;
		button->sizeRatioY = 0.05f;
		button->activationMessage = "AddFilter:"+filterList[i]+"&&PopUI("+FILTER_SELECTION_MENU+")";
		Graphics.QueueMessage(new GMAddUI(button, "FilterSelectionList"));
	}
}

/// Called every time the pipeline is changed. Generates new UI for handling it.
void CVIState::OnPipelineUpdated()
{
	/// Populate the list.
	Graphics.QueueMessage(new GMClearUI("Filters"));
	/// Add filters
	List<CVFilter*> filterList = pipeline->filters;
	for (int i = 0; i < filterList.Size(); ++i)
	{
		CVFilter * filter = filterList[i];
		String filterName = filter->name;
		String indexString = String::ToString(i);
		// Column-list to handle the buttons for editing this filter.
		UIColumnList * filterCList = new UIColumnList(filterList[i]->name+"ColumnList");
		filterCList->sizeRatioY = 0.1f;

		UIButton * button = new UIButton("EditFilter"+filterList[i]->name);
		button->text = filterName;
		button->sizeRatioX = 0.4f;
		button->activationMessage = "EditFilter:"+String::ToString(i);
		filterCList->AddChild(button);
		// For temporarily disabling a specific filter (don't wanna delete 'em all the time! o.o)
		UIButton * toggleButton = new UIButton("ToggleFilterButton"+indexString);
		toggleButton->sizeRatioX = 0.15f;
		if (filter->enabled)
			toggleButton->text = "Disable";
		else 
			toggleButton->text = "Enable";
		toggleButton->activationMessage = "ToggleFilter:"+indexString;
		filterCList->AddChild(toggleButton);
		// For changing order in the pipeline.
		UIButton * moveUpButton = new UIButton(filterList[i]->name+"MoveUp");
		moveUpButton->sizeRatioX = 0.15f;
		moveUpButton->text = "Move up";
		moveUpButton->activationMessage = "MoveFilterUp:"+String::ToString(i);
		filterCList->AddChild(moveUpButton);
		UIButton * moveDownButton = new UIButton(filterList[i]->name+"MoveDown");
		moveDownButton->sizeRatioX = 0.15f;
		moveDownButton->text = "Move down";
		moveDownButton->activationMessage = "MoveFilterDown:"+String::ToString(i);
		filterCList->AddChild(moveDownButton);
		// Add a button to delete this filter too.
		UIButton * deleteButton = new UIButton(filterList[i]->name+"DeleteButton");
		deleteButton->text = "Delete"; 
		deleteButton->sizeRatioX = 0.15f;
		deleteButton->activationMessage = "DeleteFilter:"+String::ToString(i);
		filterCList->AddChild(deleteButton);
		/// Finally add the entire button/ui for this filter
		Graphics.QueueMessage(new GMAddUI(filterCList, "Filters"));
	}
	/// Test the pipeline straight away!
	TestPipeline();
}

/// When selecting a filter for editing. Opens up UI and configures it appropriately.
void CVIState::OnFilterSelected(int index)
{
	assert(index >= 0 && index < pipeline->filters.Size());
	CVFilter * filter = pipeline->filters[index];
	/// Make visible the filter-editor.
	Graphics.QueueMessage(new GMPushUI("FilterEditor", ui));
	/// If the edit filter is the same as the current one (meaning UI has already been loaded), don't do any more.
//	if (currentEditFilter == filter)
	//	return;
	Graphics.QueueMessage(new GMClearUI("FilterEditor"));

	/// Get all parameters of the selected filter and add them to the edit-ui!
	UILabel * name = new UILabel();
	name->text = filter->name;
	name->sizeRatioY = 0.1f;
	Graphics.QueueMessage(new GMAddUI(name, "FilterEditor"));

	List<CVFilterSetting*> settings = filter->GetSettings();
	for (int i = 0; i < settings.Size(); ++i)
	{
		CVFilterSetting * setting = settings[i];
		UIElement * settingUI = NULL;
		switch(setting->type)
		{
			case CVSettingType::STRING: 
			{
				UIStringInput * stringInput = new UIStringInput(setting->name, "SetString:"+setting->name);
				stringInput->CreateChildren();
				stringInput->input->text = setting->sValue;
				settingUI = stringInput;
				break;
			}
			case CVSettingType::FLOAT:
			{
				UIFloatInput * floatInput = new UIFloatInput(setting->name, "SetFloat:"+setting->name);
				floatInput->CreateChildren();
				floatInput->SetValue(setting->fValue);
				settingUI = floatInput;
				break;
			}
			case CVSettingType::INT:
			{
				UIIntegerInput * intInput = new UIIntegerInput(setting->name, "SetInteger:"+setting->name);
				intInput->CreateChildren();
				intInput->SetValue(setting->iValue);
				settingUI = intInput;
				break;
			}
			case CVSettingType::BUTTON: 
			{
				UIButton * buttonInput = new UIButton(setting->name);
				buttonInput->activationMessage = "ActivateButtonSetting:"+setting->name;
				settingUI = buttonInput;
				break;	
			}
			case CVSettingType::BOOL:
			{
				UICheckBox * boolInput = new UICheckBox(setting->name);
				boolInput->toggled = setting->bValue;
				settingUI = boolInput;
				break;
			}
		}
		if (settingUI)
		{
			settingUI->sizeRatioY = 0.1f;
			Graphics.QueueMessage(new GMAddUI(settingUI, "FilterEditor"));
		}
	}
	
	// If a render-filter, add toggles for rendering the output into the editor or not.
	if (filter->Type() == CVFilterType::RENDER_FILTER)
	{
		CVRenderFilter * rf = (CVRenderFilter*) filter;
		UIButton * button = new UIButton("ToggleRenderOntoEditor");
		button->sizeRatioY = 0.1f;
		Graphics.QueueMessage(new GMAddUI(button, "FilterEditor"));
	}

	if (filter->about.Length())
	{
		// Add about text at the end.
		UILabel * about = new UILabel();
		// One row per.. each newline?
		about->sizeRatioY = 0.1f * (1 + filter->about.Count('\n'));
		about->text = filter->about;
		Graphics.QueueMessage(new GMAddUI(about, "FilterEditor"));
	}
	if (filter->about.Length())
	{
		UILabel * status = new UILabel();
		status->name = filter->name+"Status";
		status->text = filter->status;
		status->sizeRatioY = 0.1f;
		Graphics.QueueMessage(new GMAddUI(status, "FilterEditor"));
	}

	// Set as current for future edits.
	currentEditFilter = filter;
}

/// Replaces log-message in the UI
void CVIState::Log(String message)
{
	Graphics.QueueMessage(new GMSetUIs("Log", GMUI::TEXT, message));
}


/// Fills the native Texture-class with new data and then calls OnTextureUpdated to update visuals.
void CVIState::OnCVImageUpdated()
{
	LoadCVMatIntoTexture(&cvImage, texture);
	OnTextureUpdated();
}

/// Called after a calculation procedure is done, will display the results instead of the base image.
void CVIState::OnCVResultImageUpdated()
{
	LoadCVMatIntoTexture(&cvResultImage, texture);
	OnTextureUpdated();
}

/// Update texture on the entity that is rendered.
void CVIState::OnTextureUpdated()
{
	Graphics.QueueMessage(new GMSetEntityTexture(textureEntity, DIFFUSE_MAP, texture));
	/// Scale the entity depending on the texture size? .. or just ensure the ratio is good.
	Physics.QueueMessage(new PMSetEntity(SET_SCALE, textureEntity, Vector3f(texture->width, texture->height, 1)));
}


void CVIState::CreateProjectionWindow()
{
	if (!projectionWindow){
		projectionWindow = WindowMan.NewWindow("ProjectionWindow");
		projectionWindow->CreateGlobalUI();
		projectionWindow->requestedSize = Vector2i(400,300);
		projectionWindow->requestedRelativePosition = Vector2i(-400, 0);
		projectionWindow->backgroundColor = Vector4f(0,0,0,1);
		projectionWindow->Create();
		projectionWindow->DisableAllRenders();
		projectionWindow->renderViewports = true;
		projectionWindow->MainViewport()->EnableAllDebugRenders(false);
		projectionCamera->AdjustProjectionMatrixToWindow(projectionWindow);
		Graphics.QueueMessage(new GMSetCamera(projectionCamera, projectionWindow));
	}
}

void CVIState::ProjectSynchronizationImage(String type)
{
	if (!projectionWindow)
	{
		this->CreateProjectionWindow();
	}
	if (!projectionWindow->IsVisible())
	{
		projectionWindow->Show();
	}
	// Remove pipeline projection if it's currently there.
	if (pipelineTextureRenderedOnProjection){
		ToggleRenderPipelineTextureOnProjection();
	}	

	Texture * tex = TexMan.GetTexture(type);
	// Fetch texture.
	if (!tex)
	{
		tex = TexMan.New();
		tex->name = type;			
		// Just render a big effing .. ting.
		int width = 41, height = 31;
		tex->Resize(Vector2i(width, height));	
		tex->SetColor(Vector4f(0,0,0,1));
		if (type.Contains("Grid"))
		{
			Vector4f color;
			if (type == "RedGrid")
				color = Vector4f(1,0,0,1);
			else if (type == "WhiteGrid")
				color = Vector4f(1,1,1,1);
			// Rows?
			for (int i = 0; i < width; i+=2)
			{
				tex->SetColorOfColumn(i, color);
			}
			// Rows?
			for (int i = 0; i < height; i+=2)
			{
				tex->SetColorOfRow(i, color);
			}
		}
		else 
			tex->SetColor(Vector4f(1,1,1,1));
	}
	if (!syncEntity)
	{
		syncEntity = MapMan.CreateEntity(ModelMan.GetModel("Sprite"), tex);
	}
	else {
		Graphics.QueueMessage(new GMSetEntityTexture(syncEntity, DIFFUSE_MAP, tex));
	}


	// Set it to not render in our main viewport.
	Graphics.QueueMessage(new GMSetEntity(syncEntity, CAMERA_FILTER, cviCamera));
}
void CVIState::ExtractProjectionOutputFrame(String usingPipeline)
{
	// Get contents out of the projection window.
//	Texture frame;
	/// Fills contents of frame into target texture.
//	projectionWindow->GetFrameContents(&frame);
	
	/// Get camera input!
//	frame = cvImage;

//	frame.Save("output/Frame", true);
	// Build pipeline?
	CVPipeline syncPipe;
	cv::Mat input = cvImage;
//	bool success = TextureToCVMat(&frame, &input);

	pipeline->initialInput = &input;

	std::fstream file;
	file.open(usingPipeline.c_str());
	if (!file.is_open())
		return;
	syncPipe.ReadFrom(file);
	// Convert mat to texture..
	int returnType = syncPipe.Process(&input);
	/// Found a polypegon!
	if (syncPipe.approximatedPolygons.size())
	{
		// Make it into a box! ?
		Vector2i min;
		Vector2i max;
		std::vector<cv::Point> poly = syncPipe.approximatedPolygons.at(0);
		for (int i = 0; i < poly.size(); ++i)
		{
			cv::Point & p = poly.at(i);
			p.y = input.rows - p.y;
		}
		cv::Point firstPoint = poly.at(0);
		min.x = max.x = firstPoint.x;
		min.y = max.y = firstPoint.y;
		for (int i = 1; i < poly.size(); ++i)
		{
			cv::Point p = poly.at(i);
			if (p.x < min.x)
				min.x = p.x;
			if (p.x > max.x)
				max.x = p.x;
			if (p.y < min.y)
				min.y = p.y;
			if (p.y > max.y)
				max.y = p.y;
		}
		// Convert to screen-space window co-ordinates now? Just deduct half of the texture size?
		min.x -= input.cols / 2;
		max.x -= input.cols / 2;
		min.y -= input.rows / 2;
		max.y -= input.rows / 2;
		/// Swap y since openCV counts from top-down instead of bottom-up like I do.
//		min.y = input.rows - min.y;
	//	max.y = input.rows - max.y;
		projectionFrameInInput = Vector4f(min.x, min.y, max.x, max.y);
		projectionFrameCenter = 0.5f*(max+min);
		projectionFrameSize = max - min;
		Graphics.QueueMessage(new GMSetUIv4f("Frame", GMUI::VECTOR_INPUT, projectionFrameInInput, ui));
		Graphics.QueueMessage(new GMSetUIv2i("Center", GMUI::VECTOR_INPUT, projectionFrameCenter, ui));

		if (!frameEntity)
		{
			frameEntity = MapMan.CreateEntity(ModelMan.GetModel("Sprite.obj"), TexMan.GetTexture("Blue"));
			// Relocate it as needed.
		}
		if (frameEntity)
		{
			Physics.QueueMessage(new PMSetEntity(SET_SCALE, frameEntity, Vector3f(projectionFrameSize,1)));
			Physics.QueueMessage(new PMSetEntity(SET_POSITION, frameEntity, Vector3f(projectionFrameCenter,1)));
		}
	}
	// Fetch output?
}

void CVIState::ResetProjectionCamera()
{
	projectionCamera->position = Vector3f(0,0,CAM_Z);
	projectionCamera->zoom = 1.f;
}
void CVIState::AdjustProjectionCameraToFrame()
{
	// Set stuff in the buff.

	/// Distance from the center of the entire window (initial setup) to the center of the detected projection frame.
	Vector2i workingArea = projectionWindow->WorkingArea();
	// Window center is 0,0.
	Vector2i windowCenter;
	Vector2i centerToCenter = projectionFrameCenter - windowCenter;
	projectionFrameCenter = projectionFrameCenter;
	projectionCamera->position = Vector3f(-centerToCenter, CAM_Z);
//	projectionCamera.position = Vector3f(0,0,CAM_Z);
	
	// Compare sizes.
	float relativeSizeX = projectionFrameSize.x / (float)workingArea.x;
	float relativeSizeY = projectionFrameSize.y / (float)workingArea.y;
	
	// Use average of both to set zoom?
	float shouldZoom = (relativeSizeX + relativeSizeY) * 0.5f;
//	shouldZoom = 1 / shouldZoom;

//	projectionCamera.SetRatio(workingArea.x, workingArea.y);
	// Is the camera orthogonal? How will the zoom affect?
	// Should a manual view or projection matrix be constructed maybe?
	projectionCamera->zoom = shouldZoom;
}

/// Sets dirr
void CVIState::SetImageSeriesDirectory(String dirPath)
{
	/// Already loaded? Skip.
//	if (dirPath == imageSeriesDir)
	//	return;
	imageSeriesDir = dirPath;


	// Fetch files in dir.
	filesInImageSeries.Clear();
	bool result = GetFilesInDirectory(imageSeriesDir, filesInImageSeries);
	if (!result)
	{
		Log("Unable to open directory for readdiiinnggggg!");
		Graphics.QueueMessage(new GMSetUIs("imageSeriesLog", GMUI::TEXT, "Unable to open directory"));
		
		std::cout<<"\nERRORR SET IMAGE SERIES DIRECTORAERYYY";
		return;
	}

	Graphics.QueueMessage(new GMSetUIs("ImageSeriesDirectory", GMUI::STRING_INPUT_TEXT, imageSeriesDir));

	/// Remove non-images
	for (int i = 0; i < filesInImageSeries.Size(); ++i)
	{
		String file = filesInImageSeries[i];
		if (!file.Contains(".png"))
		{
			filesInImageSeries.RemoveIndex(i);
			--i;
		}
	}


	//// Sort the LIST!
	List<String> sorted;
	while(filesInImageSeries.Size())
	{
		int least = 1000000000;
		int leastIndex = -1;
		for (int i = 0; i < filesInImageSeries.Size(); ++i)
		{

			String file = filesInImageSeries[i];
			file.ConvertToChar();
			file = file.Numberized();
			int value = file.ParseInt();
			if (value < least)
			{
				leastIndex = i;
				least = value;
			}
		}
		assert(leastIndex >= 0);
		sorted.Add(filesInImageSeries[leastIndex]);
		filesInImageSeries.RemoveIndex(leastIndex);
	}
	filesInImageSeries = sorted;

	//	assert(result);

	/// Assume image series is already made active.

	/// Start loading the pictures as we go?

	/// Clear arrays
	imageSeriesImages.Clear();

	// Reset index, will be incremented to 0 in the main loop.
	imageSeriesIndex = -1;

	///// Dir.
	//String imageSeriesDir;
	//List<String> filesInImageDir;
	//List<cv::Mat> images;



}

