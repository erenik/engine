/// Emil Hedemalm
/// 2014-04-09
/// Filter class for processing cv-based images or data related to them.

#include "CVFilter.h"
#include <fstream>
#include "CV/CVPipeline.h"
#include "ImageFilters/CVImageFilters.h"
#include "DataFilters/CVDataFilters.h"
#include "DataFilters/CVSwipeGesture.h"

#include "DataFilters/CVHandFilters.h"
#include "DataFilters/ContourFilters.h"
#include "OpticalFlow/CVOpticalFlowFilters.h"

#include "RenderFilters/CVRenderFilters.h"
#include "RenderFilters/CVParticleRenderFilters.h"

#include "CV/Games/CVPong.h"
#include "CV/Games/CVBreakout.h"
#include "CV/Games/CVSpaceShooter.h"

List<String> CVFilter::filterNames;
CVFilter * sampleFilters[CVFilterID::MAX_FILTERS];

/// Call once before trying to create/use any filters!
void CVFilter::CreateFilterNames()
{
	// Names already created, skipping.
	if (filterNames.Size())
		return;
	// Fill it with amount of strings corresponding to amount of known filters.
	for (int i = 0; i < CVFilterID::MAX_FILTERS; ++i)
	{
		filterNames.Add(String());
		sampleFilters[i] = NULL;
	}
	// And add the actual strings too.
	////////////////////////////////////////////////////////////////////
	/// Image-manipulation filters.
	////////////////////////////////////////////////////////////////////	
	filterNames[CVFilterID::GREYSCALE] = "Greyscale";
	filterNames[CVFilterID::SCALE_UP] = "Scale up";
	filterNames[CVFilterID::SCALE_DOWN] = "Scale down";
	filterNames[CVFilterID::CANNY_EDGE] = "Canny Edge detection";
	filterNames[CVFilterID::INVERT] = "Invert";
	filterNames[CVFilterID::HARRIS_CORNER] = "Harris Corner detection";
	filterNames[CVFilterID::GAUSSIAN_BLUR] = "Gaussian blur";
	filterNames[CVFilterID::INVERT] = "Invert";
	filterNames[CVFilterID::THRESHOLD] = "Threshold";
	filterNames[CVFilterID::ABS] = "Abs";
	filterNames[CVFilterID::SATURATION_FILTER] = "Saturation filter";
	filterNames[CVFilterID::REMOVE_BACKGROUND] = "Background removal";
	filterNames[CVFilterID::ERODE] = "Erode";
	filterNames[CVFilterID::DILATE] = "Dilate";
	filterNames[CVFilterID::EXTRACT_CHANNELS] = "Extract channels";
	filterNames[CVFilterID::HUE_FILTER] = "Hue filter";
	filterNames[CVFilterID::VALUE_FILTER] = "Value filter";
	filterNames[CVFilterID::FILL_THREE] = "Fill three";
	filterNames[CVFilterID::ADAPTIVE_THRESHOLD] = "Adaptive threshold";
	filterNames[CVFilterID::PER_PIXEL_ADAPTIVE_THRESHOLD] = "Per-pixel Adaptive threshold";
	filterNames[CVFilterID::RENDER_REGION_OF_INTEREST] = "Render region of interest";
	filterNames[CVFilterID::REGION_OF_INTEREST_FILTER] = "Filter region of interest";
	filterNames[CVFilterID::LOAD_INITIAL_INPUT] = "Load initial input";
	filterNames[CVFilterID::OPTICAL_FLOW_FILTER] = "Optical flow filter";
	filterNames[CVFilterID::CONTRAST_BRIGHTNESS] = "Contrast brightness";
	filterNames[CVFilterID::SHORT_TO_UNSIGNED_BYTE] = "Short to unsigned byte";
	filterNames[CVFilterID::RANDOM_IMAGE] = "Random image";
	filterNames[CVFilterID::BIT_FILTER] = "Bit filter";
	filterNames[CVFilterID::HORIZONTAL_DEWARP] = "Horizontal dewarp";

	////////////////////////////////////////////////////////////////////
	// Data filters
	////////////////////////////////////////////////////////////////////
	filterNames[CVFilterID::FIND_CONTOURS] = "Find contours";
	filterNames[CVFilterID::FIND_CONVEX_HULLS] = "Find convex hulls";
	filterNames[CVFilterID::SHI_TOMASI_CORNERS] = "ShiTomasi Corners / Good features to track";
	filterNames[CVFilterID::HOUGH_CIRCLES] = "Hough circles";
	filterNames[CVFilterID::HOUGH_LINES] = "Hough lines";
	filterNames[CVFilterID::FIND_QUADS] = "Find quads";
	filterNames[CVFilterID::MAX_BOX_PERSISTANCE] = "Max box persistance";
//	filterNames[CVFilterID::HAND_DETECTOR] = "CVHand detector";
	filterNames[CVFilterID::QUAD_ASPECT_RATIO_CONSTRAINT] = "Quad Aspect Ratio Constraint";
	filterNames[CVFilterID::HAND_PERSISTENCE] = "Hand persistence";
	filterNames[CVFilterID::HAND_DETECTOR] = "CVHand detector (defects)";
	filterNames[CVFilterID::CALC_CONVEX_HULL] = "Calc Convex Hull";
	filterNames[CVFilterID::CALC_CONVEXITY_DEFECTS] = "Calc Convexity defects";
	filterNames[CVFilterID::FILTER_LINES_BY_ANGLE] = "Filter lines by angle";
	filterNames[CVFilterID::MERGE_LINES] = "Merge lines";
	filterNames[CVFilterID::APPROXIMATE_POLYGONS] = "Approximate polygons";
	filterNames[CVFilterID::FINGER_ACTION] = "Finger action-state filter";
	filterNames[CVFilterID::CONTOUR_CLASSIFICATION] = "Contour classifier";
	filterNames[CVFilterID::FINGER_EXTRACTION] = "Finger extraction";
	filterNames[CVFilterID::HAND_EMULATOR] = "Hand emulator";
	filterNames[CVFilterID::FINGER_IDENTIFICATION] = "Finger identification filter";
	filterNames[CVFilterID::CONTOUR_SEGMENT_CLASSIFIER] = "Contour segment classifier";
	filterNames[CVFilterID::CONTOUR_SEGMENTER] = "Contour segmenter";
	filterNames[CVFilterID::OPTICAL_FLOW_FARNEBACK] = "Optical flow - Farneback";
	filterNames[CVFilterID::OPTICAL_FLOW_LUCAS_KANADE] = "Optical flow - Lucas-Kanade";
	filterNames[CVFilterID::OPTICAL_FLOW_SIMPLE_FLOW] = "Optical flow - Simple flow";
	filterNames[CVFilterID::OPTICAL_FLOW_DUAL_TV_L1] = "Optical flow - Dual TV L1";
	filterNames[CVFilterID::GENERATE_OPTICAL_FLOW_FIELD] = "Generate Optical flow field";
	filterNames[CVFilterID::APPROXIMATE_HAND_VEOCITY_BY_OPTICAL_FLOW] = "Approx. Hand vel. by Optical flow";
	filterNames[CVFilterID::FILTER_HANDS_BY_MOVEMENT] =  "Filter hands by movement";
	filterNames[CVFilterID::PRINCIPAL_COMPONENT_ANALYSIS] = "PCA - Principal Component Analysis";
	filterNames[CVFilterID::POINT_CLOUDS] = "Point clouds";
	filterNames[CVFilterID::POINT_CLOUD_POSITIONAL_FILTER] = "Point cloud positional filter";
	filterNames[CVFilterID::HANDS_FROM_POINT_CLOUDS] = "Hands from point clouds";
	filterNames[CVFilterID::LINE_PERSISTENCE] = "Line persistence";
	filterNames[CVFilterID::TEMPLATE_MATCHER] = "Template matcher";
	filterNames[CVFilterID::SWIPE_GESTURE] = "Swipe gesture";

	////////////////////////////////////////////////////////////////////
	// Render filters
	////////////////////////////////////////////////////////////////////
	filterNames[CVFilterID::VIDEO_WRITER] = "Video writer";
	filterNames[CVFilterID::IMAGE_GALLERY_HAND] = "Image gallery, hand";
	filterNames[CVFilterID::MOVIE_PROJECTOR] = "Movie projector, polygon";
	filterNames[CVFilterID::MUSIC_PLAYER] = "Music player, hand-controlled";
	filterNames[CVFilterID::CONTOUR_PARTICLES] = "Contour particles";
	filterNames[CVFilterID::FINGER_PARTICLES] = "Finger particles";
	filterNames[CVFilterID::HAND_PARTICLES] = "Hand particles";
	filterNames[CVFilterID::MODEL_VIEWER] = "Model viewer";
	filterNames[CVFilterID::PONG] = "Pong";
	filterNames[CVFilterID::BREAKOUT] = "Breakout";
	filterNames[CVFilterID::SPACE_SHOOTER] = "Space shooter";
	filterNames[CVFilterID::MENU_INTERACTION] = "Menu interaction";
	filterNames[CVFilterID::OPTICAL_FLOW_PARTICLES] = "Optical flow (Particles)";
	filterNames[CVFilterID::SPRITE_ANIMATION] = "Sprite Animation";
	filterNames[CVFilterID::INTERACTIVE_PLATE] = "Interactive plate";
	filterNames[CVFilterID::PIANO] = "Piano";
	filterNames[CVFilterID::INTERACTIVE_SPHERE] = "Interactive sphere";

	// Own data filters <- Old stuff. Place in either Data or image above.
	filterNames[CVFilterID::LINE_GATHER] = "Line gather";
	filterNames[CVFilterID::MAX_BOUNDING_BOX] = "Max bounding box";
	filterNames[CVFilterID::TO_CHAR] = "Convert to single unsigned byte";
}

CVFilter * CreateFilterByID(int id)
{
	switch (id)
	{
		// Image filters
		case CVFilterID::CANNY_EDGE: return new CVCannyEdgeFilter();
		case CVFilterID::GAUSSIAN_BLUR: return new CVGaussianBlurFilter();
		case CVFilterID::GREYSCALE: return new CVGreyscaleFilter();
		case CVFilterID::SCALE_UP: return new CVScaleUpFilter();
		case CVFilterID::SCALE_DOWN: return new CVScaleDownFilter();
		case CVFilterID::HARRIS_CORNER: return new CVHarrisCornerFilter();
		case CVFilterID::INVERT: return new CVInvertFilter();
		case CVFilterID::THRESHOLD: return new CVThresholdFilter();
		case CVFilterID::ABS: return new CVAbsFilter();
		case CVFilterID::SATURATION_FILTER: return new CVSaturationFilter();
		case CVFilterID::REMOVE_BACKGROUND: return new CVRemoveBackgroundFilter();
		case CVFilterID::ERODE: return new CVErode();
		case CVFilterID::DILATE: return new CVDilate();
		case CVFilterID::EXTRACT_CHANNELS: return new CVExtractChannels();
		case CVFilterID::HUE_FILTER: return new CVHueFilter();
		case CVFilterID::VALUE_FILTER: return new CVValueFilter();
		case CVFilterID::FILL_THREE: return new CVFill3();
		case CVFilterID::ADAPTIVE_THRESHOLD: return new CVAdaptiveThreshold();
		case CVFilterID::PER_PIXEL_ADAPTIVE_THRESHOLD: return new CVPerPixelAdaptiveThreshold();
		case CVFilterID::RENDER_REGION_OF_INTEREST: return new CVRenderRegionOfInterest();
		case CVFilterID::REGION_OF_INTEREST_FILTER: return new CVRegionOfInterestFilter();
		case CVFilterID::LOAD_INITIAL_INPUT: return new CVLoadInitialInput();
		case CVFilterID::OPTICAL_FLOW_FILTER: return new CVOpticalFlowFilter();
		case CVFilterID::CONTRAST_BRIGHTNESS: return new CVContrastBrightnessFilter();
		case CVFilterID::SHORT_TO_UNSIGNED_BYTE: return new CVShortToUnsignedByte();
		case CVFilterID::RANDOM_IMAGE: return new CVRandomImage();
		case CVFilterID::BIT_FILTER: return new CVBitFilter();
		case CVFilterID::HORIZONTAL_DEWARP: return new CVHorizontalDewarp();

		// Data filters
		case CVFilterID::FIND_CONTOURS: return new CVFindContours();
		case CVFilterID::SHI_TOMASI_CORNERS: return new CVShiTomasiCorners();
		case CVFilterID::HOUGH_CIRCLES: return new CVHoughCircles();
		case CVFilterID::HOUGH_LINES: return new CVHoughLines();
		case CVFilterID::FIND_QUADS: return new CVFindQuads();
		case CVFilterID::MAX_BOX_PERSISTANCE: return new CVMaxBoxPersistance();
//		case CVFilterID::HAND_DETECTOR: return new CVHandDetector();
		case CVFilterID::QUAD_ASPECT_RATIO_CONSTRAINT: return new CVQuadAspectRatioConstraint();
		case CVFilterID::HAND_PERSISTENCE: return new CVHandPersistance();
		case CVFilterID::HAND_DETECTOR: return new CVHandDetector();
		case CVFilterID::CALC_CONVEX_HULL: return new CVConvexHull();
		case CVFilterID::CALC_CONVEXITY_DEFECTS: return new CVConvexityDefects();
		case CVFilterID::FILTER_LINES_BY_ANGLE: return new CVFilterLinesByAngle();
		case CVFilterID::MERGE_LINES: return new CVMergeLines();
		case CVFilterID::APPROXIMATE_POLYGONS: return new CVApproxPolygons();
		case CVFilterID::FINGER_ACTION: return new CVFingerActionFilter();
		case CVFilterID::CONTOUR_CLASSIFICATION: return new CVContourClassification();
		case CVFilterID::FINGER_EXTRACTION: return new CVFingerExtractionFilter();
		case CVFilterID::HAND_EMULATOR: return new CVHandEmulator();
		case CVFilterID::FINGER_IDENTIFICATION: return new CVFingerIdentificationFilter();
		case CVFilterID::CONTOUR_SEGMENT_CLASSIFIER: return new CVContourSegmentClassifier();
		case CVFilterID::CONTOUR_SEGMENTER: return new CVContourSegmenter();
		case CVFilterID::OPTICAL_FLOW_FARNEBACK: return new CVOpticalFlowFarneback();
		case CVFilterID::OPTICAL_FLOW_LUCAS_KANADE: return new CVOpticalFlowLucasKanade();
		case CVFilterID::OPTICAL_FLOW_SIMPLE_FLOW: return new CVOpticalFlowSimpleFlow();
		case CVFilterID::OPTICAL_FLOW_DUAL_TV_L1: return new CVOpticalFlowDualTVL1();
		case CVFilterID::GENERATE_OPTICAL_FLOW_FIELD: return new CVGenerateOpticalFlowField();
		case CVFilterID::APPROXIMATE_HAND_VEOCITY_BY_OPTICAL_FLOW: return new CVApproximateHandVelocityByOpticalFlow();
		case CVFilterID::FILTER_HANDS_BY_MOVEMENT: return new CVFilterHandsByMovement();
		case CVFilterID::PRINCIPAL_COMPONENT_ANALYSIS: return new CVPCA();
		case CVFilterID::POINT_CLOUDS: return new CVPointClouds();
		case CVFilterID::POINT_CLOUD_POSITIONAL_FILTER: return new CVPointCloudPositionalFilter();
		case CVFilterID::HANDS_FROM_POINT_CLOUDS: return new CVHandsFromPointClouds();
		case CVFilterID::LINE_PERSISTENCE: return new CVLinePersistence();
		case CVFilterID::TEMPLATE_MATCHER: return new CVTemplateMatcher();
		case CVFilterID::SWIPE_GESTURE: return new CVSwipeGesture();

		// Render-filters
		case CVFilterID::VIDEO_WRITER: return new CVVideoWriter();
		case CVFilterID::IMAGE_GALLERY_HAND: return new CVImageGalleryHand();
		case CVFilterID::MOVIE_PROJECTOR: return new CVMovieProjector();
		case CVFilterID::MUSIC_PLAYER: return new CVMusicPlayer();
		case CVFilterID::CONTOUR_PARTICLES: return new CVContourParticles();
		case CVFilterID::FINGER_PARTICLES: return new CVFingerParticles();
		case CVFilterID::HAND_PARTICLES: return new CVHandParticles();
		case CVFilterID::MODEL_VIEWER: return new CVModelViewer();
		case CVFilterID::PONG: return new CVPong();
		case CVFilterID::BREAKOUT: return new CVBreakout();
		case CVFilterID::SPACE_SHOOTER: return new CVSpaceShooter();
		case CVFilterID::MENU_INTERACTION: return new CVMenuInteraction();
		case CVFilterID::OPTICAL_FLOW_PARTICLES: return new CVOFParticles();
		case CVFilterID::SPRITE_ANIMATION: return new CVSpriteAnimation();
		case CVFilterID::INTERACTIVE_PLATE: return new CVInteractivePlate();
		case CVFilterID::PIANO: return new CVPiano();
		case CVFilterID::INTERACTIVE_SPHERE: return new CVInteractiveSphere();

		// Own <- Old stuff. Place in either Data or image above.
	//	case CVFilterID::LINE_GATHER: return new CVLineGatherFilter();
//		case CVFilterID::MAX_BOUNDING_BOX: return new CVMaxBoundingBox();
		case CVFilterID::TO_CHAR: return new CVToChar();	
		default:
		{
		//	std::cout<<"\nCreateFilterByID: CV Filter ID "<<id<<" not recognized. No filter created.";
			break;
		}
	}
	// And return it!
	return NULL;	
}

/// Returns sample filter of target type. These are static and should not be used directly.
CVFilter * GetSampleFilter(int id)
{
	if (!sampleFilters[id])
		sampleFilters[id] = CreateFilterByID(id);
	return sampleFilters[id];
}

void DeleteSampleFilters()
{
	for (int i = 0; i < CVFilterID::MAX_FILTERS; ++i)
	{
		CVFilter * filter = sampleFilters[i];
		if (filter)
			delete filter;
		sampleFilters[i] = NULL;
	}
}


// Must specify name when creating! o.o
CVFilter::CVFilter(int id)
	: id(id)
{
	name = filterNames[id];
	enabled	= true;
	type = -1;
	processingTime = renderTime = 0;
	previousFilter = NULL;
}

// Virtual destructor for proper deallocatoin when sub-classing.
CVFilter::~CVFilter()
{
	settings.ClearAndDelete();
}

/// Called upon adding the filter to the active pipeline.
void CVFilter::OnAdd()
{
	// Do nothing by default.
}


// Should be called when deleting a filter while the application is running. Removes things as necessary.
void CVFilter::OnDelete()
{
	// Do nothing by default?
}


/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
void CVFilter::SetEnabled(bool newEnabledState)
{
	enabled = newEnabledState;
}

/// Build-in messaging system. Used for advanced applications to communicate with the game engine properly.
void CVFilter::ProcessMessage(Message * message)
{
}


#define CV_FILTER_VERSION_1 1 // Initial version
#define CV_FILTER_VERSION_2 2 // Added save/load of the enabled-boolean so that the state of the pipeline is exactly the same as when saving it!
#define CURRENT_VERSION CV_FILTER_VERSION_2

/// Save all settings specific to this filter.
bool CVFilter::WriteTo(std::fstream & file)
{
	int version = CURRENT_VERSION;
	file.write((char*)&version, sizeof(int));
	name.WriteTo(file);

	/// CV_FILTER_VERSION_2
	file.write((char*)&enabled, sizeof(bool));

	/// Save amount of settings.
	int numSettings = settings.Size();
	file.write((char*)&numSettings, sizeof(int));
	for (int i = 0; i < settings.Size(); ++i)
	{
		CVFilterSetting * setting = settings[i];
		setting->WriteTo(file);
	}
	return true;
}

bool CVFilter::ReadFrom(std::fstream & file)
{
	int version;
	file.read((char*)&version, sizeof(int));
	assert(version >= CV_FILTER_VERSION_1 &&
		version <= CURRENT_VERSION);
	name.ReadFrom(file);

	/// Added enable in v.2
	if (version >= CV_FILTER_VERSION_2)
	{
		file.read((char*)&enabled, sizeof(bool));
		/// If below this version, enabled will just be true by default.
	}


	int numSettings = settings.Size();
	file.read((char*)&numSettings, sizeof(int));
	/// Use a temporary setting in order to load settings.
	CVFilterSetting setting;
	for (int i = 0; i < numSettings; ++i)
	{
		bool ok = setting.ReadFrom(file);
		if (!ok)
			continue;
		// Compare it's name with our current settings.
		CVFilterSetting * currSetting = GetSetting(setting.name);
		// If we are currently using such given setting, copy over it's value.
		if (currSetting){
			currSetting->fValue = setting.fValue;
			currSetting->iValue = setting.iValue;
			currSetting->sValue = setting.sValue;
			currSetting->bValue = setting.bValue;
			currSetting->vec2iData = setting.vec2iData;
			currSetting->vec2fData = setting.vec2fData;
			currSetting->vec3fData = setting.vec3fData;
			currSetting->vec4fData = setting.vec4fData;
			
			// Flag it as being in a new state.
			currSetting->settingChanged = true;
		}
	}
	return true;
}

int CVFilter::Process(CVPipeline * pipe)
{
	std::cout<<"\nDefault filter, does nothing.";
	return 0;
}

/** Main painting function. This will be called on the last processed filter in order to get a renderable result.
	Separated from main processing since painting can take an unnecessary long amount of time to complete.
*/
void CVFilter::Paint(CVPipeline * pipe)
{
	std::cout<<"\nDefault filter: Paint. Consider subclassing it to get proper output.";
}

/// Fetch specific setting.
CVFilterSetting * CVFilter::GetSetting(String byName)
{
	for (int i = 0; i < settings.Size(); ++i)
	{
		CVFilterSetting * setting = settings[i];
		if (setting->name == byName)
			return setting;
	}
	return NULL;
}
	

/// Construction helper function. Used by pipeline's Save/Load.
CVFilter * CreateFilterByName(String filterName)
{
	/// Type to create.
	int id = 0;
	for (int i = 0; i < CVFilter::filterNames.Size(); ++i)
	{
		String iName = CVFilter::filterNames[i];
		if (iName == filterName)
		{
			id = i;
			break;
		}
	}
	return CreateFilterByID(id);	
}

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"

#include "UI/UIInputs.h"
#include "UI/UIButtons.h"


/// Creates UI for editing this filter. Assumes an element named FilterEditor is present which will contain all created elements.
void CVFilter::UpdateEditUI(UserInterface * inUI)
{
		/// If the edit filter is the same as the current one (meaning UI has already been loaded), don't do any more.
//	if (currentEditFilter == filter)
	//	return;
	Graphics.QueueMessage(new GMClearUI("FilterEditor", inUI));

	/// Get all parameters of the selected filter and add them to the edit-ui!
	UILabel * nameLabel = new UILabel();
	nameLabel->text = this->name;
	nameLabel->sizeRatioY = 0.1f;
	Graphics.QueueMessage(new GMAddUI(nameLabel, "FilterEditor", inUI));

	for (int i = 0; i < settings.Size(); ++i)
	{
		CVFilterSetting * setting = settings[i];
		UIElement * settingUI = NULL;
		switch(setting->type)
		{
			case CVSettingType::VECTOR_2I:
			{
				UIVectorInput * vecIn = new UIVectorInput(2, setting->name, "SetVector:"+setting->name);
				vecIn->CreateChildren();
				vecIn->dataType = UIVectorInput::INTEGERS;
				vecIn->SetValue2i(setting->GetVec2i());
				settingUI = vecIn;
				break;
			}
			case CVSettingType::VECTOR_2F:
			{
				UIVectorInput * vecIn = new UIVectorInput(2, setting->name, "SetVector:"+setting->name);
				vecIn->CreateChildren();
				vecIn->SetValue2f(setting->GetVec2f());
				settingUI = vecIn;
				break;
			}
			case CVSettingType::VECTOR_3F:
			{
				UIVectorInput * vecIn = new UIVectorInput(3, setting->name, "SetVector:"+setting->name);
				vecIn->CreateChildren();
				vecIn->SetValue3f(setting->GetVec3f());
				settingUI = vecIn;
				break;
			}
			case CVSettingType::VECTOR_4F:
			{
				UIVectorInput * vecIn = new UIVectorInput(4, setting->name, "SetVector:"+setting->name);
				vecIn->CreateChildren();
				vecIn->SetValue4f(setting->GetVec4f());
				settingUI = vecIn;
				break;
			}
			case CVSettingType::STRING: 
			{
				UIStringInput * stringInput = new UIStringInput(setting->name, "SetString:"+setting->name);
				stringInput->CreateChildren();
				stringInput->input->text = setting->GetString();
				settingUI = stringInput;
				break;
			}
			case CVSettingType::FLOAT:
			{
				UIFloatInput * floatInput = new UIFloatInput(setting->name, "SetFloat:"+setting->name);
				floatInput->CreateChildren();
				floatInput->SetValue(setting->GetFloat());
				settingUI = floatInput;
				break;
			}
			case CVSettingType::INT:
			{
				UIIntegerInput * intInput = new UIIntegerInput(setting->name, "SetInteger:"+setting->name);
				intInput->CreateChildren();
				intInput->SetValue(setting->GetInt());
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
				boolInput->activationMessage = "SetBool:"+setting->name;
				boolInput->toggled = setting->GetBool();
				settingUI = boolInput;
				break;
			}
			default:
				assert(false && "Implement");
		}
		if (settingUI)
		{
			settingUI->sizeRatioY = 0.1f;
			Graphics.QueueMessage(new GMAddUI(settingUI, "FilterEditor", inUI));
		}
	}
	
	// If a render-filter, add toggles for rendering the output into the editor or not.
	if (type == CVFilterType::RENDER_FILTER)
	{
		CVRenderFilter * rf = (CVRenderFilter*) this;
		UIButton * button = new UIButton("ToggleRenderOntoEditor");
		button->sizeRatioY = 0.1f;
		Graphics.QueueMessage(new GMAddUI(button, "FilterEditor", inUI));
	}

	if (about.Length())
	{
		// Add about text at the end.
		UILabel * aboutLabel = new UILabel();
		// One row per.. each newline?
		aboutLabel->sizeRatioY = 0.05f * (1 + about.Count('\n'));
		aboutLabel->text = about;
		Graphics.QueueMessage(new GMAddUI(aboutLabel, "FilterEditor", inUI));
	}
	// Always create the status..
	UILabel * statusLabel = new UILabel();
	statusLabel->name = name+"Status";
	statusLabel->text = this->status;
	statusLabel->sizeRatioY = 0.1f;
	Graphics.QueueMessage(new GMAddUI(statusLabel, "FilterEditor", inUI));

	/// Add a back-button for those not wanting to right-click or Esc?
	UIButton * back = new UIButton("Back");
	back->sizeRatioY = 0.1f;
	Graphics.QueueMessage(new GMAddUI(back, "FilterEditor", inUI));
}


