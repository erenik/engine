/// Emil Hedemalm
/// 2014-04-29
/// Render-filters, for visual effects or writing to file. Some use the native graphics system to provide 3D-rendering(?)

#include "CVRenderFilters.h"
#include "CVPipeline.h"
#include "String/StringUtil.h"
#include "File/FileUtil.h"


// See CVFilterTypes.h for IDs
CVRenderFilter::CVRenderFilter(int filterID) 
	: CVFilter(filterID)
{
	type = CVFilterType::RENDER_FILTER;
}
int CVRenderFilter::Process(CVPipeline * pipe)
{
	std::cout<<"\nSubclass?";
	return -1;
}
// Should be overloaded? If you paint to the output-texture?
void CVRenderFilter::Paint(CVPipeline * pipe)
{
	
}


CVVideoWriter::CVVideoWriter()
	: CVRenderFilter(CVFilterID::VIDEO_WRITER)
{
	//CVFilterSetting * outputFile, * duration, * startButton;
	outputFile = new CVFilterSetting();
	outputFile->sValue = "video";
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
	// Used for both start- and stop, so..
	bool startStopQueued = startButton->bValue;
	startButton->bValue = false;
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
		// Add it into output folder..
		String fullPath = "output/"+outputFile->sValue+".avi";
		videoWriter.open(fullPath.c_str(), CV_FOURCC('M', 'P', 'E', 'G'), encodingFps->fValue, cv::Size(pipe->output.cols, pipe->output.rows));
		frame = 0;
		if (!videoWriter.isOpened())
		{
			status = errorString = "Unable to open target file: "+outputFile->sValue+", reverting to saving image sequences instead.";
			targetFolder = "output/"+outputFile->sValue+"/";
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


#include "Maps/MapManager.h"
#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"

CVImageGalleryHand::CVImageGalleryHand()
	: CVRenderFilter(CVFilterID::IMAGE_GALLERY_HAND)
{
	directory = new CVFilterSetting("Directory");
	directory->sValue = "img";
	directory->type = CVSettingType::STRING;
	settings.Add(directory);
	
	minimumTimeBetweenSwitches = new CVFilterSetting("Minimum time between", 1000);
	settings.Add(minimumTimeBetweenSwitches);

	currentDirectory = "";
	galleryEntity = NULL;
	lastSwap = 0;
	fingersLastFrame = 0;
}
int CVImageGalleryHand::Process(CVPipeline * pipe)
{
	// New directory? Load it.
	if (currentDirectory != directory->sValue)
	{
		files.Clear();
		currentDirectory = directory->sValue;
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
		galleryEntity = MapMan.CreateEntity(ModelMan.GetModel("Sprite"), TexMan.GetTexture("White"));
	}
	// Move it depending on where the hand is.
	if (pipe->hands.Size())
	{
		Hand hand = pipe->hands[0];
		// The relative scale that is rendered should probably be saved somewhere.. like in the pipeline?
		Vector3f position = Vector3f(hand.center.x, hand.center.y, 3);
		// Offset the position with half of the width and height, since the rendered entity sprite is centered to 0,0,0
		position.x -= pipe->initialInput->cols * 0.5f;
		position.y = pipe->initialInput->rows * 0.5f - position.y;

		position *= 0.01f;

		// Works!
//		position = Vector3f(0,0,3);
		Physics.QueueMessage(new PMSetEntity(POSITION, galleryEntity, position));  
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
			Hand & hand = pipe->hands[0];
			bool swap = false;
			// If 5 again, reset stuff, so we can "click" again.
			if (hand.fingers.Size() == 5)
			{
				
			}
			// Click only if went from 5 to 4?
			else if (hand.fingers.Size() == 4 && fingersLastFrame == 5)
			{
				swap = true;
				lastSwap = 0;
				timeSinceLastSwap = minimumTimeBetweenSwitches->iValue + 1;
			}
			else if (hand.fingers.Size() == 0)
			{
				swap = true;
			}
			if (swap && timeSinceLastSwap > minimumTimeBetweenSwitches->iValue)
			{
				currentImage++;
				if (currentImage >= files.Size())
					currentImage = 0;
				String source = currentDirectory + "/" + files[currentImage];
				Graphics.QueueMessage(new GMSetEntityTexture(galleryEntity, DIFFUSE_MAP, source));
				lastSwap = now;
			}
			fingersLastFrame = hand.fingers.Size();
		}
		
	}

	return CVReturnType::RENDER;
}

#include "Graphics/Messages/GraphicsMessages.h"

/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
void CVImageGalleryHand::SetEnabled(bool state)
{
	enabled = state;
	Graphics.QueueMessage(new GMSetEntityb(galleryEntity, VISIBILITY, state));
}


#include "Multimedia/MultimediaManager.h"

CVMovieProjector::CVMovieProjector()
	: CVRenderFilter(CVFilterID::MOVIE_PROJECTOR)
{
	movieEntity = NULL;
	movieStream = NULL;
}
CVMovieProjector::~CVMovieProjector()
{
/*	if (movieEntity)
		MapMan.DeleteEntity(movieEntity);
	if (movieStream)
	{
		movieStream->Pause();
	}
	*/
}

int CVMovieProjector::Process(CVPipeline * pipe)
{
	// Have an entity? If not create it.
	if (!movieEntity)
	{
		movieEntity = MapMan.CreateEntity(ModelMan.GetModel("Sprite"), TexMan.GetTexture("White"));
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
		// Offset the position with half of the width and height, since the rendered entity sprite is centered to 0,0,0
		position.x -= pipe->initialInput->cols * 0.5f;
		position.y = pipe->initialInput->rows * 0.5f - position.y;

		position *= 0.01f;

		// Works!
//		position = Vector3f(0,0,3);
		Physics.QueueMessage(new PMSetEntity(POSITION, movieEntity, position));  
	}
	/*
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
			Hand & hand = pipe->hands[0];
			bool swap = false;
			// If 5 again, reset stuff, so we can "click" again.
			if (hand.fingers.Size() == 5)
			{
				
			}
			// Click only if went from 5 to 4?
			else if (hand.fingers.Size() == 4 && fingersLastFrame == 5)
			{
				swap = true;
				lastSwap = 0;
				timeSinceLastSwap = minimumTimeBetweenSwitches->iValue + 1;
			}
			else if (hand.fingers.Size() == 0)
			{
				swap = true;
			}
			if (swap && timeSinceLastSwap > minimumTimeBetweenSwitches->iValue)
			{
				currentImage++;
				if (currentImage >= files.Size())
					currentImage = 0;
				String source = currentDirectory + "/" + files[currentImage];
				Graphics.QueueMessage(new GMSetEntityTexture(galleryEntity, DIFFUSE_MAP, source));
				lastSwap = now;
			}
			fingersLastFrame = hand.fingers.Size();
		}
		
	}
	*/
	return CVReturnType::RENDER;
}
	
/// For reacting to when enabling/disabling a filter. Needed for e.g. Render-filters. Not required to subclass.
void CVMovieProjector::SetEnabled(bool state)
{
	enabled = state;
	Graphics.QueueMessage(new GMSetEntityb(movieEntity, VISIBILITY, state));
}

