/// Emil Hedemalm
/// 2014-05-08
/// Screen-capturing utility.

#include "Graphics/GraphicsManager.h"
#include "GraphicsState.h"
#include "Texture.h"
#include "String/AEString.h"
#include "File/FileUtil.h"

void GraphicsManager::RenderCapture()
{
	if (graphicsState->promptScreenshot)
	{
		// Grab frame! o.o
		Texture frame;
		frame.bpp = 4; // 4 bytes per pixel, RGBA
		frame.Resize(Vector2i(width, height));
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, frame.data);
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
		frame.Save(dirPath+"/"+String::ToString(++graphicsState->screenshotsTaken)+".png", true);
		graphicsState->promptScreenshot = false;
	}
}