/// Emil Hedemalm
/// 2014-05-08
/// Screen-capturing utility.

#include "Graphics/GraphicsManager.h"
#include "GraphicsState.h"
#include "Texture.h"
#include "String/AEString.h"
#include "File/FileUtil.h"
#include "TextureManager.h"
#include "Window/AppWindow.h"
#include "System/Memory.h"

void GraphicsManager::RenderCapture()
{
	AppWindow * window = graphicsState->activeWindow;
	Vector2i windowSize = window->WorkingArea();
	if (window->saveScreenshot)
	{
		// Grab frame! o.o
		Texture * frame = window->frame;
		if (!frame)
			window->frames = frame = TexMan.New();
		frame->SetFormat(Texture::RGBA);
//		frame->bpp = 4; // 4 bytes per pixel, RGBA
		frame->Resize(windowSize);
		glReadPixels(0, 0, windowSize[0], windowSize[1], GL_RGBA, GL_UNSIGNED_BYTE, frame->data);
		// Flip it.
	//	frame.FlipXY();
	//	frame->FlipY();
		
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
		frame->Save(dirPath+"/"+String::ToString(++graphicsState->screenshotsTaken)+".png", true);
		window->saveScreenshot = false;
	}

	// If not interested in video...
	if (!window->recordVideo && !window->isRecording && window->frames.Size() == 0)
		return;

	// When starting,
	if (window->recordVideo == true && !window->isRecording)
	{
		window->frames.ClearAndDelete();
		window->isRecording = true;
		// Frame time should be savable into the texture objects.
		window->captureStart = Time::Now();
		
	}
	// When recording.
	bool grabFrame = false;
	if (window->isRecording)
	{
		Time now = Time::Now();
		// See if enough time has passed. Is a new frame due?
		int millisecondsPassed = (now - window->captureStart).Milliseconds();
		// Min milliseconds per frame.
		if (millisecondsPassed > window->frames.Size() * 50)
			grabFrame = true;
	}
	if (grabFrame)
	{
		// Grab frame! o.o
		Texture * frame = NULL;
		frame = TexMan.New();
		frame->name = window->name + "_"+String::ToString(window->frames.Size()); 
		frame->SetFormat(Texture::RGBA);
//		frame->bpp = 4; // 4 bytes per pixel, RGBA
		// Resize/allocate the frame. 
		if (!frame->Resize(windowSize))
		{
			// If it fails, skip this loop and stop recording.
			window->recordVideo = false;
			window->isRecording = false;
			return;
		}
		glReadPixels(0, 0, windowSize[0], windowSize[1], GL_RGBA, GL_UNSIGNED_BYTE, frame->data);
		// Flip it. Why..?
//		frame->FlipY();
		// Add it to list of frames to save.
		window->frames.Add(frame);
		// Check amount of free memory available.
		int freeMBs = AvailableMemoryMB();
		// If we exceed a pre-defined limit, which should be relative to the amount of free memory, stop recording.
		if (freeMBs < 50)
		{
			std::cout<<"\nMemory limit reached, halting video capture procedure.";
			window->isRecording = false;
		}
		if (window->frames.Size() % 100 == 0)
			std::cout<<"\n"<<window->frames.Size()<<" frames recorded";
	}
	static String videoDirPath = "output/video/";
	/// If stopping.
	if (window->isRecording && !window->recordVideo)
	{
		// Start saving all textures to file-system.
		window->isRecording = false;
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
	if (!window->isRecording && window->frames.Size())
	{
		// Fetch next frame, and save it.
		Texture * frame = window->frames[0];
		std::cout<<"\nSaving recorded frames. "<<window->frames.Size()<<" frames remaining.";
		// Replace any slashes with some other character, or it will fail.
		frame->name.Replace('/', '-');
		frame->Save(videoDirPath+"/"+frame->name, true);
		window->frames.RemoveItem(frame);
		TexMan.DeleteTexture(frame);
	}

	// Render thingy if schmingy
	if (window->isRecording)
	{
		// Render a "recording" symbol somewhere?
		ShadeMan.SetActiveShader(0);
	
		// Fill the polygons!
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Reset projection matrix for this
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
	
		glOrtho(0, windowSize[0], 0, windowSize[1], 1, 10);
		// Reset modelmatrix too
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		float z = -1.01f;		
		glTranslatef(0,0,z);
		Matrix4d modelView = graphicsState->viewMatrixD * graphicsState->modelMatrixD;
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
		graphicsState->currentTexture = texture;
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

	

		int size = windowSize[0] * 0.05f;
		int offset = size * 0.5f;
		// Specifies how the red, green, blue and alpha source blending factors are computed
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunc(GL_ONE, GL_ONE);
		glDisable( GL_LIGHTING );
		glEnable(GL_BLEND);
		glBegin(GL_QUADS);
			glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
			glTexCoord2f(0.0f, 0.0f);			glVertex3f(offset,	windowSize[1] - offset,	z);
			glTexCoord2f(1.0f, 0.0f);			glVertex3f(GLfloat(offset+size),	windowSize[1] - offset,	z);
			glTexCoord2f(1.0f, 1.0f);			glVertex3f(GLfloat(offset+size),	GLfloat(windowSize[1] - offset - size),		z);
			glTexCoord2f(0.0f, 1.0f);			glVertex3f(offset,	GLfloat(windowSize[1] - offset - size),		z);
		glEnd();
		error = glGetError();
		if (error != GL_NO_ERROR){
		//	PRINT_ERROR
		}
		glDisable(GL_TEXTURE_2D);
		// Load projection matrix again
		glLoadMatrixd(graphicsState->projectionMatrixD.getPointer());

		// Enable disabled stuffs.
		glEnable(GL_DEPTH_TEST);	
	}

}