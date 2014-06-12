/// Emil Hedemalm
/// 2014-05-08
/// Screen-capturing utility.

#include "Graphics/GraphicsManager.h"
#include "GraphicsState.h"
#include "Texture.h"
#include "String/AEString.h"
#include "File/FileUtil.h"
#include "TextureManager.h"
#include "Window/Window.h"

void GraphicsManager::RenderCapture()
{
	Vector2i windowSize = graphicsState.activeWindow->Size();
	if (graphicsState.promptScreenshot)
	{
		// Grab frame! o.o
		Texture frame;
		frame.bpp = 4; // 4 bytes per pixel, RGBA
		frame.Resize(windowSize);
		glReadPixels(0, 0, windowSize.x, windowSize.y, GL_RGBA, GL_UNSIGNED_BYTE, frame.data);
		// Flip it.
	//	frame.FlipXY();
		frame.FlipY();
		
		String dirPath = "output/screenshots";
		if (!PathExists(dirPath))
		{
			/// Builds a path of folders so that the given path can be used. Returns false if it fails to meet the path-required. NOTE: Only works with relative directories!
			bool foldersCreated = EnsureFoldersExistForPath(dirPath);
			if (!foldersCreated){
				std::cout<<"Unable to create folders for path: "<<dirPath;
				return;
			}
		}
		frame.Save(dirPath+"/"+String::ToString(++graphicsState.screenshotsTaken)+".png", true);
		graphicsState.promptScreenshot = false;
	}

	// If we are currently recording.
	static bool isRecording = false;
	static List<Texture*> frames;
	static int64 lastFrame;
	int timeBetweenFrames = 200;
	static int framesSaved = 0;
	static String videoDirPath = "output/video";
	int64 now = Timer::GetCurrentTimeMs();
	// When starting.
	if (!isRecording && graphicsState.recording)
	{
		frames.ClearAndDelete();
		isRecording = true;
		lastFrame = now;
		framesSaved = 0;
		graphicsState.framesRecorded = 0;
	}
	// If recording o-o
	if (graphicsState.recording && lastFrame + timeBetweenFrames < now)
	{
		// Grab frame! o.o
		Texture * frame = new Texture();
		frame->bpp = 4; // 4 bytes per pixel, RGBA
		frame->Resize(windowSize);
		glReadPixels(0, 0, windowSize.x, windowSize.y, GL_RGBA, GL_UNSIGNED_BYTE, frame->data);
		// Flip it.
		frame->FlipY();
		// Add it to list of frames to save.
		frames.Add(frame);
		// If we exceed a pre-defined limit, which should be relative to the amount of free memory, stop recording.
		if (frames.Size() > 800)
			graphicsState.recording = false;
		++graphicsState.framesRecorded;
		lastFrame = now;
		std::cout<<"\n"<<graphicsState.framesRecorded<<" frames recorded";
	}
	/// If stopping.
	if (isRecording && !graphicsState.recording)
	{
		// Save all textures to file-system!
		isRecording = false;
		
		if (!PathExists(videoDirPath))
		{
			/// Builds a path of folders so that the given path can be used. Returns false if it fails to meet the path-required. NOTE: Only works with relative directories!
			bool foldersCreated = EnsureFoldersExistForPath(videoDirPath);
			if (!foldersCreated){
				std::cout<<"Unable to create folders for path: "<<videoDirPath;
				return;
			}
		}
	}

	/// After recording, save one .png file each frame until done.
	if (!isRecording && framesSaved < frames.Size())
	{
		for (int i = framesSaved; i < frames.Size(); ++i)
		{
			Texture * frame = frames[i];
			frame->Save(videoDirPath+"/"+String::ToString(i)+".png", true);
			break;
		}
		++framesSaved;
		// When done, de-allocate frame data.
		if (framesSaved >= frames.Size())
		{
			frames.ClearAndDelete();
		}
	}

	if (isRecording)
	{
		// Render a "recording" symbol somewhere?
		glUseProgram(0);
	
		// Fill the polygons!
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Reset projection matrix for this
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
	
		glOrtho(0, windowSize.x, 0, windowSize.y, 1, 10);
		// Reset modelmatrix too
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		float z = -1.01f;		
		glTranslatef(0,0,z);
		Matrix4d modelView = graphicsState.viewMatrixD * graphicsState.modelMatrixD;
	//	glLoadMatrixd(modelView.getPointer());

		// Disable depth-testing in-case deferred rendering is enabled D:
		glDisable(GL_DEPTH_TEST);

		// Enable blending
		glEnable(GL_BLEND);	
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// Enable texturing
		glEnable(GL_TEXTURE_2D);
		Texture * texture = TexMan.GetTexture("Red");
		if (texture->glid == -1)
			texture->Bufferize();
		glBindTexture(GL_TEXTURE_2D, texture->glid);
		graphicsState.currentTexture = texture;
		// Buffer it again..
		int error = glGetError();
		if (error != GL_NO_ERROR){
		//	PRINT_ERROR
		}

		// Set texturing parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Disable lighting
		glDisable(GL_LIGHTING);
		error = glGetError();
		if (error != GL_NO_ERROR){
		//	PRINT_ERROR
		}
		glDisable(GL_COLOR_MATERIAL);
	///	glDisable(GL_COLOR);

	

		int size = windowSize.x * 0.05f;
		int offset = size * 0.5f;
		// Specifies how the red, green, blue and alpha source blending factors are computed
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable( GL_LIGHTING );
		glEnable(GL_BLEND);
		glBegin(GL_QUADS);
			glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
			glTexCoord2f(0.0f, 0.0f);			glVertex3f(offset,	windowSize.y - offset,	z);
			glTexCoord2f(1.0f, 0.0f);			glVertex3f(GLfloat(offset+size),	windowSize.y - offset,	z);
			glTexCoord2f(1.0f, 1.0f);			glVertex3f(GLfloat(offset+size),	GLfloat(windowSize.y - offset - size),		z);
			glTexCoord2f(0.0f, 1.0f);			glVertex3f(offset,	GLfloat(windowSize.y - offset - size),		z);
		glEnd();
		error = glGetError();
		if (error != GL_NO_ERROR){
		//	PRINT_ERROR
		}
		glDisable(GL_TEXTURE_2D);
		// Load projection matrix again
		glLoadMatrixd(graphicsState.projectionMatrixD.getPointer());

		// Enable disabled stuffs.
		glEnable(GL_DEPTH_TEST);	
	}

}