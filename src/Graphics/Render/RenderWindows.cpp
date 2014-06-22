/// Emil Hedemalm
/// 2014-06-12
/// Contains the abstraction for rendering to multiple windows.

#include "Graphics/GraphicsManager.h"
#include "Window/WindowManager.h"
#include "GraphicsState.h"

void GraphicsManager::RenderWindows()
{

	List<Window*> windows = WindowMan.GetWindows();
	for (int i = 0; i < windows.Size(); ++i)
	{
		Window * window = windows[i];
		// Only render visible windows?
		if (!window->IsVisible())
			continue;
		bool ok = window->MakeGLContextCurrent();
		graphicsState.activeWindow = window;
		graphicsState.windowWidth = window->WorkingArea().x;
		graphicsState.windowHeight = window->WorkingArea().y;

		// Render all that is needed
		RenderWindow();

		Timer swapBufferTimer;
		swapBufferTimer.Start();
		// Swap buffers to screen once we're finished.
#ifdef WINDOWS
		// SwapBuffers should preferably be called on a per-window basis?
		bool result = SwapBuffers(window->hdc);
		if (!result)
		{
			std::cout<<"\nError in SwapBuffers(window->hdc)";
		}
#elif defined USE_X11
		glXSwapBuffers(display, window);
#endif
		int64 swapBufferFrameTime = swapBufferTimer.GetMs();	
	//	std::cout<<"\nSwapBufferTime: "<<swapBufferFrameTime;
	}
}
