/// Emil Hedemalm
/// 2014-04-29
/// Render-filters, for visual effects or writing to file. Some use the native graphics system to provide 3D-rendering(?)

#include "CVRenderFilters.h"

#include "CV/CVPipeline.h"
#include "CV/Data/CVHand.h"

#include "String/StringUtil.h"
#include "File/FileUtil.h"
#include "Direction.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Messages/GMAnimate.h"
#include "Graphics/GraphicsProperty.h"
#include "Graphics/Particles/Sparks.h"
#include "Graphics/Messages/GMParticles.h"
#include "Graphics/Camera/Camera.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"


//#include "Texture.h"

// See CVFilterTypes.h for IDs
CVRenderFilter::CVRenderFilter(int filterID) 
	: CVFilter(filterID)
{
	type = CVFilterType::RENDER_FILTER;
	renderOntoEditor = true;
}

CVRenderFilter::~CVRenderFilter()
{

}

int CVRenderFilter::Process(CVPipeline * pipe)
{
	std::cout<<"\nSubclass?";
	return -1;
}
// Should be overloaded? If you paint to the output-texture?
void CVRenderFilter::Paint(CVPipeline * pipe)
{
	// Default, just copy the input to output, assume the Process is playing with entities
	pipe->initialInput.copyTo(pipe->output);
}

/// By default, calls GetEntities and sets their visibility according to the given state.
void CVRenderFilter::SetEnabled(bool value)
{
	CVFilter::SetEnabled(value);
	List<Entity*> entities = GetEntities();
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		if (!entity)
			continue;
		Graphics.QueueMessage(new GMSetEntityb(entity, GT_VISIBILITY, value));
	}
}



CVVideoWriter::CVVideoWriter()
	: CVRenderFilter(CVFilterID::VIDEO_WRITER)
{
	//CVFilterSetting * outputFile, * duration, * startButton;
	outputFile = new CVFilterSetting();
	outputFile->SetString("video");
	outputFile->type = CVSettingType::STRING;
	outputFile->name = "Output file";
	startButton = new CVFilterSetting("Start/Stop recording");
	encodingFps = new CVFilterSetting("Encoding FPS", 15.f);

	settings.Add(outputFile);
	settings.Add(startButton);
	settings.Add(encodingFps);

	List<String> lines;
	lines += "All files are saved into the /output/ folder.";
	lines += "If duration is positive, it will record target duration,\nif not click the start button again to stop recording.";
	about = MergeLines(lines, '\n');
	writing = false;
}

int CVVideoWriter::Process(CVPipeline * pipe)
{
	// Make sure the last tested filter paints first!
	if (pipe->filterToPaint)
		pipe->filterToPaint->Paint(pipe);

	// Used for both start- and stop, so..
	bool startStopQueued = startButton->GetBool();
	startButton->SetBool(false);
	// Check if currently writing, if so write moar.
	if (writing)
	{
		++frame;
		// Image-sequence
		if (imageSequence)
		{
			String fullPath = targetFolder+String::ToString(frame)+".png";
			cv::imwrite(fullPath.c_str(), pipe->output);
			status = "Writing frame "+String::ToString(frame);
			// Check if flagged to stop writing, if so stop.
			if (startStopQueued)
			{
				writing = false;
				status = "Writing stopped.";
			}
		}
		// Video
		else {
			videoWriter.write(pipe->output);
			status = "Writing frame "+String::ToString(frame);
			// Check if flagged to stop writing, if so stop.
			if (startStopQueued)
			{
				videoWriter.release();
				writing = false;
				status = "Writing stopped.";
			}
		}
		return CVReturnType::VIDEO;
	}
	// If not writing, do we want to start writing? If so re-create the video-writer object-thingy
	else if (startStopQueued)
	{
		String outputFileStr = outputFile->GetString();
		// Add it into output folder..
		String fullPath = "output/"+outputFileStr+".avi";
		videoWriter.open(fullPath.c_str(), CV_FOURCC('M', 'P', 'E', 'G'), encodingFps->GetFloat(), cv::Size(pipe->output.cols, pipe->output.rows));
		frame = 0;
		if (!videoWriter.isOpened())
		{
			status = errorString = "Unable to open target file: "+outputFileStr+", reverting to saving image sequences instead.";
			targetFolder = "output/"+outputFileStr+"/";
			CreateFolder(targetFolder);
			writing = true;
			imageSequence = true;
			return -1;
		}
		else	
		{
			// Opened file, start writing.
			imageSequence = false;
			writing = true;
			status = "File opened.";
			return CVReturnType::VIDEO;
		}
	}
	return 0;
}

List<Entity*> CVVideoWriter::GetEntities()
{
	return List<Entity*>();
}


CVImageGalleryHand::CVImageGalleryHand()
	: CVRenderFilter(CVFilterID::IMAGE_GALLERY_HAND)
{
	directory = new CVFilterSetting("Directory");
	directory->SetString("img");
	directory->type = CVSettingType::STRING;
	settings.Add(directory);
	
	minimumTimeBetweenSwitches = new CVFilterSetting("Minimum time between", 1000);
	settings.Add(minimumTimeBetweenSwitches);

	currentDirectory = "";
	galleryEntity = NULL;
	lastSwap = 0;
	fingersLastFrame = 0;
	tex = NULL;
}

void CVImageGalleryHand::Paint(CVPipeline * pipe)
{
	pipe->initialInput.copyTo(pipe->output);
	RenderHands(pipe);
}

// Should be called when deleting a filter while the application is running. Removes things as necessary.
void CVImageGalleryHand::OnDelete()
{
	MapMan.DeleteEntity(galleryEntity);
	galleryEntity = NULL;
}

int CVImageGalleryHand::Process(CVPipeline * pipe)
{
	// New directory? Load it.
	String dir = directory->GetString();
	if (currentDirectory != dir)
	{
		files.Clear();
		currentDirectory = dir;
		GetFilesInDirectory(currentDirectory, files);
		currentImage = -1;
		// Skip those not .png?
		for (int i = 0; i < files.Size(); ++i)
		{
			String fileName = files[i];
			if (!fileName.Contains(".png"))
			{
				files.RemoveIndex(i);
				--i;
			}
		}
		this->status = String::ToString(files.Size())+" files found.";
	}
	if (files.Size() <= 0){
		errorString = "No files in directory";
		return -1;
	}

	// Have an entity? If not create it.
	if (!galleryEntity)
	{
		galleryEntity = MapMan.CreateEntity("ImageGallery", ModelMan.GetModel("Sprite"), TexMan.GetTexture("White"));
	}
	// Move it depending on where the hand is.
	if (pipe->hands.Size())
	{
		CVHand hand = pipe->hands[0];
		// The relative scale that is rendered should probably be saved somewhere.. like in the pipeline?
		Vector3f position = Vector3f(hand.center.x, hand.center.y, 3);
		Vector3f worldPos = pipe->InputSpaceToWorldSpace(position);
		// Offset the position with half of the width and height, since the rendered entity sprite is centered to 0,0,0
	//	position.x -= pipe->initialInput->cols * 0.5f;
	//	position.y = pipe->initialInput->rows * 0.5f - position.y;

		// Works!
//		position = Vector3f(0,0,3);

		// Convert to projection space (-halfwidth, halfwidth), similar to GLs [-1,1]
		Physics.QueueMessage(new PMSetEntity(galleryEntity, PT_POSITION, worldPos));  
		UpdateScale(hand);
	}
	if (files.Size())
	{
		// Load image if not already done so.
		if (currentImage == -1)
		{
			currentImage = 0;
			Graphics.QueueMessage(new GMSetEntityTexture(galleryEntity, DIFFUSE_MAP, files[currentImage]));
		}
		// Check for fingersss, if only 4, get next?
		if (pipe->hands.Size())
		{
			long long now = Timer::GetCurrentTimeMs();
			long long  timeSinceLastSwap = now - lastSwap;
			CVHand & hand = pipe->hands[0];
			bool swap = false;
			int direction = Direction::FORWARD;
	
			
			// New finger-state filter data! :D
			if (pipe->fingerStates.Size() >= 1)
			{
				FingerState & currentState = pipe->fingerStates.Last();
				FingerState & previousState = pipe->fingerStates[pipe->fingerStates.Size() - 2];
//				std::cout<<"\nTime in state: "<<currentState.duration<<" processed: "<<currentState.processed;
#define MIN_DURATION 10
				if (currentState.fingers == 5 && !currentState.processed && currentState.duration > MIN_DURATION)
				{
					currentState.processed = true;
				}
				// Check number of fingers! If.. 4? switch song
				if (currentState.fingers == 4 && !currentState.processed && currentState.duration > MIN_DURATION)
				{
					// Check the preceding state, was it 5?
					if (previousState.fingers == 5)
						swap = true;
					currentState.processed = true;
				}
				else if (currentState.fingers == 3 && !currentState.processed && currentState.duration > MIN_DURATION)
				{
					// Check the preceding state, was it 5?
					if (previousState.fingers == 4)
					{
						swap = true;
						direction = Direction::BACKWARD;
					}
					currentState.processed = true;				
				}
				else if (currentState.fingers == 0)
				{
					swap = true;
				}
			}
			// Old method checking just number of fingers before the smoother
			else 
			{
				// If 5 again, reset stuff, so we can "click" again.
				if (hand.fingers.Size() == 5)
				{
					// std::cout<<"\nClick reset";	
				}
				// Click only if went from 5 to 4?
				else if (hand.fingers.Size() == 4)
				{
					std::cout<<"\nFingers 4, last frame: "<<fingersLastFrame;
					if (fingersLastFrame == 5)
					{
						swap = true;
					//	timeSinceLastSwap = minimumTimeBetweenSwitches->GetInt() + 10000;
						std::cout<<"\n\"Click\"!";
					}
				
				}
				// Click from 4 to 3 too?!
				else if (hand.fingers.Size() == 3)
				{
					if (fingersLastFrame == 4)
					{
						swap = true;
						direction = Direction::BACKWARD;
						std::cout<<"\nBackwards! o-o";
					}
				}
				else if (hand.fingers.Size() == 0)
				{
					swap = true;
			//		std::cout<<"\nAutoplayin";
				}
			}	
			
			// If a swap was queried, do it?
			if (swap || tex == NULL)
			{
				if (timeSinceLastSwap > minimumTimeBetweenSwitches->GetInt())
				{
					if (direction == Direction::FORWARD)
					{
						++currentImage;
						if (currentImage >= files.Size())
							currentImage = 0;
					}
					else if (direction == Direction::BACKWARD)
					{
						--currentImage;
						if (currentImage < 0)
							currentImage = files.Size() - 1;
					}
					String source = currentDirectory + "/" + files[currentImage];

					SetTexture(source);
					UpdateScale(hand);
				}
				else {
		//			std::cout<<"\nSwap not deemed ready yet. Minimum time hasn't passed.";
				}
			}
			fingersLastFrame = hand.fingers.Size();
		}
		
	}

	return CVReturnType::RENDER;
}


List<Entity*> CVImageGalleryHand::GetEntities()
{
	return galleryEntity;
}

void CVImageGalleryHand::SetTexture(String source)
{
	tex = TexMan.GetTexture(source);
	Graphics.QueueMessage(new GMSetEntityTexture(galleryEntity, DIFFUSE_MAP, tex));
	lastSwap = Timer::GetCurrentTimeMs();
}

void CVImageGalleryHand::UpdateScale(CVHand & hand)
{
	// Scale?
	if (tex){
		float s = hand.size.x * 0.005f * hand.size.y * 0.0025f;
		s *= 100.f;
		float max = tex->width > tex->height? tex->width : tex->height;
		Vector3f scale = Vector3f(s * tex->width / max, s * tex->height / max, s);
		Physics.QueueMessage(new PMSetEntity(galleryEntity, PT_SET_SCALE, scale));
	}
}


#include "Graphics/Messages/GraphicsMessages.h"

/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
void CVImageGalleryHand::SetEnabled(bool state)
{
	CVRenderFilter::SetEnabled(state);
//	enabled = state;
//	Graphics.QueueMessage(new GMSetEntityb(galleryEntity, GT_VISIBILITY, state));
}


#include "Multimedia/MultimediaManager.h"

CVMovieProjector::CVMovieProjector()
	: CVRenderFilter(CVFilterID::MOVIE_PROJECTOR)
{
	movieEntity = NULL;
	movieStream = NULL;
	framesToSmooth = new CVFilterSetting("Frames to smooth", 10);
	settings.Add(framesToSmooth);
}
CVMovieProjector::~CVMovieProjector()
{
}

int CVMovieProjector::Process(CVPipeline * pipe)
{
	// Have an entity? If not create it.
	if (!movieEntity)
	{
		movieEntity = MapMan.CreateEntity("Movie", ModelMan.GetModel("Sprite"), TexMan.GetTexture("White"));
	}
	if (!movieStream)
	{
		movieStream = MultimediaMan.Play("video/big_buck_bunny_480p_stereo.ogg");
		if (movieStream)
		{
			Texture * tex = movieStream->GetFrameTexture();
			Graphics.QueueMessage(new GMSetEntityTexture(movieEntity, DIFFUSE_MAP, tex));
		}
	}
	if (movieStream)
		status = String::ToString(movieStream->CurrentFrameTime());

	// Move it depending on where the hand is.
	std::vector<cv::Point> * bestPoly = NULL;
	float bestSize = 0;
	for (int i = 0; i < pipe->approximatedPolygons.size(); ++i)
	{
		std::vector<cv::Point> & poly = pipe->approximatedPolygons[i];
		float polySize = cv::contourArea(poly);
		if (poly.size() == 4){
			if (bestPoly == NULL){
				bestPoly = &poly;
				bestSize = polySize;
			}
			else if (polySize > bestSize)
			{
				bestPoly = &poly;
			}
		}
	}
	if (bestPoly && bestPoly->size())
	{
		Vector3f center;
		for (int i = 0; i < bestPoly->size(); ++i)
		{
			cv::Point point = (*bestPoly)[i];
			center += Vector3f(point.x, point.y, 3);
		}
		center *= 0.25f;
		// The relative scale that is rendered should probably be saved somewhere.. like in the pipeline?
		Vector3f position = Vector3f(center.x, center.y, 3);
		Vector3f worldPos = pipe->InputSpaceToWorldSpace(position);
		// Offset the position with half of the width and height, since the rendered entity sprite is centered to 0,0,0
//		position.x -= pipe->initialInput->cols * 0.5f;
//		position.y = pipe->initialInput->rows * 0.5f - position.y;

		// Works!
//		position = Vector3f(0,0,3);
		float frames = framesToSmooth->GetInt();
		averagePosition = averagePosition * (frames - 1) / frames + worldPos / frames;
		Physics.QueueMessage(new PMSetEntity(movieEntity, PT_POSITION, averagePosition));  

		// Set width/height ratio.
		Texture * tex = movieStream->GetFrameTexture();
		float max = tex->width > tex->height? tex->width : tex->height;
		Vector3f scale(tex->width / max, tex->height / max, 1);
		// Multiply scale with width or height or the polygon?
		cv::Rect boundingRect = cv::boundingRect(*bestPoly);
#define Maximum(a,b) ( (a > b) ? a : b)

		scale *= Maximum(boundingRect.width, boundingRect.height);
		averageScale = averageScale * (frames - 1) / frames + scale / frames;
		// Scale it further depending on the size of the contour of the hand?
		Physics.QueueMessage(new PMSetEntity(movieEntity, PT_SET_SCALE, averageScale));
	}
	return CVReturnType::RENDER;
}

// Should be called when deleting a filter while the application is running. Removes things as necessary.
void CVMovieProjector::OnDelete()
{
	movieStream->Pause();
	MapMan.DeleteEntity(movieEntity);
	movieEntity = NULL;
}

void CVMovieProjector::Paint(CVPipeline * pipe)
{
	pipe->initialInput.copyTo(pipe->output);
	RenderPolygons(pipe);
}
	

List<Entity*> CVMovieProjector::GetEntities()
{
	if (this->movieEntity)
		return movieEntity;
	return List<Entity*>();
}
	
/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
void CVMovieProjector::SetEnabled(bool state)
{
	CVRenderFilter::SetEnabled(state);
//	enabled = state;
//	Graphics.QueueMessage(new GMSetEntityb(movieEntity, GT_VISIBILITY, state));
}


#include "Audio/TrackManager.h"

CVMusicPlayer::CVMusicPlayer()
	: CVRenderFilter(CVFilterID::MUSIC_PLAYER)
{
	directory = new CVFilterSetting("Directory");
	directory->type = CVSettingType::STRING;
	directory->SetString("sound/bgm/SpaceRace");
	settings.Add(directory);
	audioTrack = NULL;
	currentTrack = 0;
	lastFinger = 0;
	lastFingerStart = lastFingerDuration = 0;	
	audioEntity = NULL;
}
CVMusicPlayer::~CVMusicPlayer()
{
}

// Should be called when deleting a filter while the application is running. Removes things as necessary.
void CVMusicPlayer::OnDelete()
{
	if (audioTrack)
		audioTrack->Stop();
	if (audioEntity)
		MapMan.DeleteEntity(audioEntity);
}

int CVMusicPlayer::Process(CVPipeline * pipe)
{
	if (!audioEntity)
	{
		audioEntity = MapMan.CreateEntity("MusicPlayer", ModelMan.GetModel("Sprite"), TexMan.GetTexture("Alpha"));
		Graphics.QueueMessage(new GMSetEntityVec4f(audioEntity, GT_TEXT_COLOR, Vector4f(0,0,1,1)));
		Graphics.QueueMessage(new GMSetEntityf(audioEntity, GT_TEXT_SIZE_RATIO, 0.1f));
	//	Graphics.QueueMessage(new GMSetEntityb(audioEntity, GT_VISIBILITY, false));
	}

	// New directory? Load it.
	String dir = directory->GetString();
	if (currentDirectory != dir)
	{
		files.Clear();
		currentDirectory = dir;
		GetFilesInDirectory(currentDirectory, files);
		currentTrack = 0;
		// Skip those not .png?
		for (int i = 0; i < files.Size(); ++i)
		{
			String fileName = files[i];
			if (!fileName.Contains(".ogg"))
			{
				files.RemoveIndex(i);
				--i;
			}
		}
		this->status = String::ToString(files.Size())+" files found.";
	}
	if (files.Size() <= 0){
		errorString = "No files in directory";
		return -1;
	}

	// Have an entity? If not create it.
	if (!audioTrack)
	{
		if (files.Size() == 0)
			return -1;
		audioTrack = TrackMan.PlayTrack(currentDirectory + "/" + files[currentTrack]);
		if (!audioTrack)
			return -1;
		audioTrack->Loop(true);
	}
	// Check hand position
	if (pipe->hands.Size())
	{
		// 
		if (pipe->fingerStates.Size())
		{
			FingerState & lastState = pipe->fingerStates.Last();
			std::cout<<"\nTime in state: "<<lastState.duration<<" processed: "<<lastState.processed;
			if (lastState.fingers == 5 && !lastState.processed && lastState.duration > 2000)
			{
				if (audioTrack->IsPlaying())
				{
					audioTrack->Pause();
					std::cout<<"\n Pausing";
				}
				else 
				{
					audioTrack->Resume();
					std::cout<<"\n Resuming";
				}
				lastState.processed = true;
			}
			// Check number of fingers! If.. 4? switch song
			if (lastState.fingers == 4 && !lastState.processed && lastState.duration > 2000)
			{
				std::cout<<"\n Next";
				++currentTrack;
				if (currentTrack >= files.Size())
					currentTrack = 0;
				audioTrack = TrackMan.PlayTrack(currentDirectory + "/" + files[currentTrack]);
				lastState.processed = true;
			}
			else if (lastState.fingers == 3 && !lastState.processed && lastState.duration > 2000)
			{
				std::cout<<"\n Previous";
				--currentTrack;
				if (currentTrack < 0)
					currentTrack = files.Size() - 1;
				audioTrack = TrackMan.PlayTrack(currentDirectory + "/" + files[currentTrack]);
				lastState.processed = true;				
			}
			if (audioTrack)
				audioTrack->Loop(true);
		}
		CVHand & hand = pipe->hands[0];
		Vector3f handPos = hand.center;
		// Use hand position to set volume o-o
		// Compare with input height.
		float relative = handPos.y / pipe->initialInput.rows;
		relative = 1 - relative - 0.15f;
		relative *= 1.5f;
	//	std::cout<<"\nVolume set to: "<<relative;
		if (audioTrack)
			audioTrack->SetVolume(relative);

		// The relative scale that is rendered should probably be saved somewhere.. like in the pipeline?
		Vector3f position = Vector3f(handPos.x, handPos.y, 3);
		Vector3f worldPos = pipe->InputSpaceToWorldSpace(position);
		// Offset the position with half of the width and height, since the rendered entity sprite is centered to 0,0,0
	//	position.x -= pipe->initialInput->cols * 0.5f;
	//	position.y = pipe->initialInput->rows * 0.5f - position.y;

//		position *= 0.01f;

		Physics.QueueMessage(new PMSetEntity(audioEntity, PT_POSITION, worldPos));  
		
		// Update the text on le hand-music-audio-entity-thingy
		Graphics.QueueMessage(new GMSetEntitys(audioEntity, GT_TEXT, files[currentTrack]+"\nVolume: "+String::ToString(relative, 3)));

		float scale = hand.size.y * 2.0f * 0.1f;
		scale *= 1.f;
		Graphics.QueueMessage(new GMSetEntityf(audioEntity, GT_TEXT_SIZE_RATIO, scale));
		Vector4f textPosition = Vector4f(- hand.size.x * 0.5f, hand.size.y * 0.1f, 0, 0);
		Graphics.QueueMessage(new GMSetEntityVec4f(audioEntity, GT_TEXT_POSITION, textPosition));
		Graphics.QueueMessage(new GMSetEntityVec4f(audioEntity, GT_TEXT_COLOR, Vector4f(1,1,0,1)));
	}
	return CVReturnType::RENDER;
}
/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
void CVMusicPlayer::SetEnabled(bool state)
{
	CVRenderFilter::SetEnabled(state);
	// Pause the music?
//	enabled = state;
	if (!enabled)
	{
		audioTrack->Pause();
	}
	else
		audioTrack->Resume();
	// o-o
//	Graphics.QueueMessage(new GMSetEntityb(audioEntity, GT_VISIBILITY, enabled));
}

void CVMusicPlayer::Paint(CVPipeline * pipe)
{
	pipe->initialInput.copyTo(pipe->output);
	RenderHands(pipe);
}


List<Entity*> CVMusicPlayer::GetEntities()
{
	return audioEntity;
}



CVModelViewer::CVModelViewer()
	: CVRenderFilter(CVFilterID::MODEL_VIEWER)
{
	modelEntity = NULL;
	String defaultModel = "obj/Bunny.obj";
	model = new CVFilterSetting("Model", defaultModel);
	rotationSpeed = new CVFilterSetting("Rotation speed", Vector3f());
	resetPosition = new CVFilterSetting("Reset position");
	manualRotation = new CVFilterSetting("Manual rotation", false);
	playSkeletalAnimation = new CVFilterSetting("Play skeletal animation");
	animateUsingShaders = new CVFilterSetting("Animate using shaders", true);
	pauseAnimation = new CVFilterSetting("Pause animation", false);
	settings.Add(7, 
		model, rotationSpeed, resetPosition, 
		manualRotation, playSkeletalAnimation, animateUsingShaders,
		pauseAnimation);
}
int CVModelViewer::Process(CVPipeline * pipe)
{
	int returnType = CVOutputType::RENDER;
	if (!modelEntity)
	{
		modelEntity = MapMan.CreateEntity("ModelViewer", ModelMan.GetModel(model->GetString()), TexMan.GetTexture("Gray"));
		Physics.QueueMessage(new PMSetEntity(modelEntity, PT_SET_SCALE, 50.f));
		// Ignore collisions
		Physics.QueueMessage(new PMSetEntity(modelEntity, PT_COLLISIONS_ENABLED, false));
		Physics.QueueMessage(new PMSetEntity(modelEntity, PT_GRAVITY_MULTIPLIER, 0.f));
		Physics.QueueMessage(new PMSetEntity(modelEntity, PT_PHYSICS_TYPE, PhysicsType::DYNAMIC));
		
	}
	// Adjust position.
	if (model->HasChanged())
	{
		String newModelName = model->GetString();
		if (!newModelName.Length() == 0)
		{
			Model * newModel = ModelMan.GetModel(newModelName);
			Graphics.QueueMessage(new GMSetEntity(modelEntity, GT_MODEL, newModel));
		}
	}
	if (resetPosition->HasChanged())
	{
		Physics.QueueMessage(new PMSetEntity(modelEntity, PT_RESET_ROTATION));
		Physics.QueueMessage(new PMSetEntity(modelEntity, PT_CONSTANT_ROTATION_SPEED, Vector3f()));
	}
	if (playSkeletalAnimation->HasChanged())
	{
		Graphics.QueueMessage(new GMPlaySkeletalAnimation(modelEntity));
	}
	if (animateUsingShaders->HasChanged())
	{
		Graphics.QueueMessage(new GMSetEntityb(modelEntity, GT_ANIMATE_SKIN_USING_SHADERS, animateUsingShaders->GetBool()));
	}
	if (pauseAnimation->HasChanged())
	{
		Graphics.QueueMessage(new GMSetEntityb(modelEntity, GT_PAUSE_ANIMATIONS, pauseAnimation->GetBool()));
	}
	// Change speed as necessary.
	if (rotationSpeed->HasChanged())
	{
		Physics.QueueMessage(new PMSetEntity(modelEntity, PT_CONSTANT_ROTATION_SPEED, rotationSpeed->GetVec3f()));
	}

	// Fetch frustum.
	Camera * camera = Graphics.ActiveCamera();
	Frustum frustum = camera->GetFrustum();
	float centerZ = (frustum.fartherBottomRight.z - frustum.hitherBottomLeft.z) * 0.5f;
	float rotSpeed = 0.5f;


		
	if (!manualRotation->GetBool())
	{
		
		return returnType;
	}

	// Check for a visible hand. 
	if (pipe->hands.Size())
	{
		CVHand * hand = &pipe->hands[0];

		// Place model in the middle.
		Vector2f center = hand->center;
		center = pipe->InputSpaceToWorldSpace(center);
//		center.x -= pipe->initialInput->cols * 0.5;
//		center.y = pipe->initialInput->rows * 0.5 - center.y;
		// Move it back a bit?
		Physics.QueueMessage(new PMSetEntity(modelEntity, PT_SET_POSITION, Vector3f(center)));
	
		// Check finger count. Do something if a special finger count?
		int numFingers = hand->fingers.Size();
		switch(numFingers)
		{
			case 5:	
				SetRotationSpeed(Vector3f(0,0,0)); 
				break;
			case 4: 
				SetRotationSpeed(Vector3f(0,rotSpeed,0));	
				break;
			case 3:
				SetRotationSpeed(Vector3f(0,-rotSpeed, 0));
				break;
			case 2: 
				SetRotationSpeed(Vector3f(rotSpeed,0,0));	
				break;
			case 1:
				SetRotationSpeed(Vector3f(-rotSpeed, 0, 0));
				break;
		}
	}
	// No hand? Default rotation.
	else 
	{
		SetRotationSpeed(Vector3f(0,-rotSpeed, 0));
	}
	return returnType;
}

// Should be called when deleting a filter while the application is running. Removes things as necessary.
void CVModelViewer::OnDelete()
{
	if (modelEntity)
		MapMan.DeleteEntity(modelEntity);
	modelEntity = NULL;
}	

List<Entity*> CVModelViewer::GetEntities()
{
	return modelEntity;
}
/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
void CVModelViewer::SetEnabled(bool state)
{
	CVRenderFilter::SetEnabled(state);
}


void CVModelViewer::SetRotationSpeed(Vector3f speed)
{
	Physics.QueueMessage(new PMSetEntity(modelEntity, PT_CONSTANT_ROTATION_SPEED, speed));		
}


CVSpriteAnimation::CVSpriteAnimation()
	: CVRenderFilter(CVFilterID::SPRITE_ANIMATION)
{
	scale = new CVFilterSetting("Sprite scale", 50.f);

	settings.Add(scale);
	spriteEntity = NULL;
}



int CVSpriteAnimation::Process(CVPipeline * pipe)
{
	if (!spriteEntity)
	{
		spriteEntity = MapMan.CreateEntity("Sprite entity", ModelMan.GetModel("Sprite.obj"), TexMan.GetTexture("White"));
		Physics.QueueMessage(new PMUnregisterEntity(spriteEntity));
		// Assign it an animation?
		Graphics.QueueMessage(new GMSetEntitys(spriteEntity, GT_ANIMATION_SET, "anim/Horse"));
		Graphics.QueueMessage(new GMSetEntitys(spriteEntity, GT_ANIMATION, "Gallop"));
	}
	if (pipe->hands.Size())
	{
		Vector3f worldSpace = pipe->InputSpaceToWorldSpace(pipe->hands[0].center);
		float smoothingFactor = 0.15f;
		smoothedPosition = smoothedPosition * (1 - smoothingFactor) + worldSpace * smoothingFactor;
		Physics.QueueMessage(new PMSetEntity(spriteEntity, PT_POSITION, smoothedPosition));
	}
	if (scale->HasChanged())
	{
		Physics.QueueMessage(new PMSetEntity(spriteEntity, PT_SET_SCALE, scale->GetFloat()));
	}
	return CVReturnType::RENDER;
}


List<Entity*> CVSpriteAnimation::GetEntities()
{
	return spriteEntity;
}

// Should be called when deleting a filter while the application is running. Removes things as necessary.
void CVSpriteAnimation::OnDelete()
{
	if (spriteEntity)
		MapMan.DeleteEntity(spriteEntity);
	spriteEntity = NULL;
}
