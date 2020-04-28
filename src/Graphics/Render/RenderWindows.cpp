/// Emil Hedemalm
/// 2014-06-12
/// Contains the abstraction for rendering to multiple windows.

#include "Graphics/GraphicsManager.h"
#include "Window/AppWindowManager.h"
#include "GraphicsState.h"

#include "Window/WindowSystem.h"
#include "Graphics/FrameStatistics.h"

#ifdef USE_X11
	#include "Window/XWindowSystem.h"
	#include "Window/XIncludes.h"
	extern Display * xDisplay;
#endif

int framesSkipped = 0;

void GraphicsManager::RenderWindows()
{
	graphicsThreadDetails = "GraphicsManager::RenderWindows";
	List<AppWindow*> windows = WindowMan.GetWindows();

	int times = 0;

	for (int i = 0; i < windows.Size(); ++i)
	{
	renderWindowStart:
		AppWindow * window = windows[i];


		// Only render visible windows?
		if (!window->IsVisible())
		{
			continue;
		}

		bool ok = window->MakeGLContextCurrent();

		/// Tester functions: setting up a default projection and rendering test triangle...?
		//SetupViewProjectionGL(400, 400);
		//RenderTestTriangle();


		framesSkipped = 0;
		graphicsState.activeWindow = window;
		graphicsState.windowWidth = window->WorkingArea()[0];
		graphicsState.windowHeight = window->WorkingArea()[1];

		// Reset shader. Force AppWindow to explicitly set an own one, so that attributes are bound correctly.
		ShadeMan.SetActiveShader(&graphicsState, nullptr);

		// Render all that is needed
		RenderWindow();

		Timer swapBufferTimer;
		swapBufferTimer.Start();
		// Swap buffers to screen once we're finished.
#ifdef WINDOWS
		// SwapBuffers should preferably be called on a per-AppWindow basis?
		bool result = SwapBuffers(window->hdc);
		if (!result)
		{
			std::cout<<"\nError in SwapBuffers(window->hdc)";
		}
#elif defined USE_X11
		glXSwapBuffers(xDisplay, window->xWindowHandle);
#endif
		int64 swapBufferFrameTime = swapBufferTimer.GetMs();	
		FrameStats.swapBuffers += swapBufferTimer.GetMs();
		++times;
		if (times >= 2 || true)
		{
			times = 0;
			continue;
		}
		else
			goto renderWindowStart;
	}
	++framesSkipped;
	if (framesSkipped > 1000)
	{
		std::cout<<"\nWindow not visible. Rendering paused.";
		framesSkipped = 0;
	}
}
