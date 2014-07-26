// Emil Hedemalm
// 2013-07-03 Linuxifying!

#include "../../DataObject.h"
#include "OS/Sleep.h"
#include "Player/PlayerManager.h"
#include "Graphics/Messages/GraphicsMessages.h"

#include "Graphics/GraphicsManager.h"
#include "Input/InputManager.h"
#include "StateManager.h"
#include "Physics/PhysicsManager.h"
#include "Graphics/FrameStatistics.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Multimedia/MultimediaManager.h"
#include "Application/Application.h"
#include "Window/WindowManager.h"
#include "Viewport.h"

#include "Message/FileEvent.h"
#include "Message/MessageManager.h"


enum generalActions{
	NULL_ACTION,

	// Debug-renders.
	TOGGLE_RENDER_LIGHTS,
	TOGGLE_RENDER_PHYSICS,
	TOGGLE_RENDER_WAYPOINTS,

	CYCLE_RENDER_PIPELINE,
	CYCLE_RENDER_PIPELINE_BACK,

	RECORD_VIDEO,
	PRINT_SCREENSHOT,

	RELOAD_MODELS, // Reloads all models.
	RELOAD_UI,
	RECOMPILE_SHADERS,
	GO_TO_EDITOR,
	GO_TO_MAIN_MENU,

    PRINT_FRAME_TIME,
	PRINT_PLAYER_INPUT_DEVICES,
	LIST_TEXTURES,
	LIST_MODELS,
	LIST_CAMERAS,
	PRINT_UI_TREE,

	PRINT_TO_FILE, // Prints active video screen or entire game screen-shot as PNG to file.

	OPEN_LIGHTING_EDITOR, // Common for the rendering pipeline in general.
	CLOSE_WINDOW,	// When pressing CTRL+W, like when closing tabs. Closes external windows or queries shutdown.
	QUIT_APPLICATION,
	TOGGLE_FULL_SCREEN,
	COPY,
	PASTE,	// General pasting!
	INTERPRET_CONSOLE_COMMAND,


};

/// Windows
#ifdef WINDOWS
#define UNICODE
#include <Windows.h>
#include <shellapi.h>
//PLABELBOX pbox;
LPTSTR	lptstr;
LPTSTR  lptstrCopy;
HGLOBAL hglb;
HGLOBAL hglbCopy;		// Global copy-something
int ich1, ich2, cch;

/// Linux
#elif defined LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // contains visual information masks and CVisualInfo structure
#include <X11/Xatom.h>
extern Window window;
extern Display * display;
#endif



/// Creates bindings that are used for debugging purposes only
void CreateDefaultGeneralBindings()
{

	InputMapping * mapping = &Input.general;

	
	int ctrl = KEY::CTRL;

	mapping->CreateBinding(CLOSE_WINDOW, ctrl, KEY::W);
	mapping->CreateBinding(OPEN_LIGHTING_EDITOR, ctrl, KEY::O, KEY::L);
	mapping->CreateBinding(TOGGLE_RENDER_PHYSICS, ctrl, KEY::R, KEY::P);
	mapping->CreateBinding(TOGGLE_RENDER_LIGHTS, ctrl, KEY::R, KEY::L);
	mapping->CreateBinding(TOGGLE_RENDER_WAYPOINTS, ctrl, KEY::R, KEY::W);
	mapping->CreateBinding(CYCLE_RENDER_PIPELINE, ctrl, KEY::R, KEY::PLUS);
	mapping->CreateBinding(CYCLE_RENDER_PIPELINE_BACK, ctrl, KEY::R, KEY::MINUS);

	Input.general.CreateBinding(RECORD_VIDEO, KEY::CTRL, KEY::R, KEY::V);
	Input.general.CreateBinding(PRINT_SCREENSHOT, KEY::PRINT_SCREEN);
    Input.general.CreateBinding(PRINT_FRAME_TIME, KEY::CTRL, KEY::T);

	Input.general.CreateBinding(QUIT_APPLICATION, KEY::ALT, KEY::F4);
	//Input.general.CreateBinding(QUIT_APPLICATION, KEY::ALT, KEY::F4);
	Input.general.CreateBinding(TOGGLE_FULL_SCREEN, KEY::ALT, KEY::ENTER);
	Input.general.CreateBinding(PASTE, KEY::CTRL, KEY::V);
	Input.general.CreateBinding(COPY, KEY::CTRL, KEY::C);
	Input.general.CreateBinding(GO_TO_MAIN_MENU, KEY::CTRL, KEY::G, KEY::M);
	
	Input.general.CreateBinding(PRINT_TO_FILE, KEY::CTRL, KEY::P, KEY::F);

	Input.general.CreateBinding(RELOAD_MODELS, KEY::CTRL, KEY::R, KEY::M, "CTRL+R+M : Reload models");
	Input.general.CreateBinding(RELOAD_UI, KEY::CTRL, KEY::R, KEY::U);
	Input.general.CreateBinding(RECOMPILE_SHADERS, KEY::CTRL, KEY::R, KEY::S);
	Input.general.CreateBinding(GO_TO_EDITOR, KEY::CTRL, KEY::G, KEY::E);

	Input.general.CreateBinding(PRINT_PLAYER_INPUT_DEVICES, KEY::L, KEY::I);
    Input.general.CreateBinding(LIST_TEXTURES, KEY::L, KEY::T);
    Input.general.CreateBinding(LIST_CAMERAS, KEY::L, KEY::C);
    Binding * binding = Input.general.CreateBinding(LIST_MODELS, KEY::L, KEY::M);
	Input.general.SetBlockingKeys(binding, KEY::CTRL);
    Input.general.CreateBinding(PRINT_UI_TREE, KEY::L, KEY::U);
};


void generalInputProcessor(int action, int inputDevice)
{
	Window * activeWindow = ActiveWindow();
	Viewport * mainViewport = activeWindow->MainViewport();
	switch(action)
	{
		case QUIT_APPLICATION:
			if (Application::queryOnQuit)
				MesMan.QueueMessages("Query(QuitApplication)");
			else
				MesMan.QueueMessages("QuitApplication");
			break;
		case CYCLE_RENDER_PIPELINE:
		{
			Graphics.QueueMessage(new GraphicsMessage(GM_CYCLE_RENDER_PIPELINE));	
			break;
		}
		case CYCLE_RENDER_PIPELINE_BACK:
		{
			Graphics.QueueMessage(new GraphicsMessage(GM_CYCLE_RENDER_PIPELINE_BACK));	
			break;
		}
		case CLOSE_WINDOW:
		{
			if (activeWindow->IsMain())
				MesMan.QueueMessages("Query(QuitApplication)");
			else
				activeWindow->Hide();	
			break;
		}
		case OPEN_LIGHTING_EDITOR:
		{
			/// This assumes only one lighting setup is used.
			Lighting * activeLighting = Graphics.ActiveLighting();
			activeLighting->OpenEditorWindow();
			break;
		}
		case TOGGLE_RENDER_LIGHTS:
			mainViewport->renderLights = !mainViewport->renderLights;
			break;
		case TOGGLE_RENDER_PHYSICS:
			mainViewport->renderPhysics = !mainViewport->renderPhysics;
			break;
		case TOGGLE_RENDER_WAYPOINTS:
			mainViewport->renderNavMesh = !mainViewport->renderNavMesh;
			break;
		case RECORD_VIDEO:
			Graphics.QueueMessage(new GMRecordVideo(activeWindow));
			break;
		case PRINT_SCREENSHOT:
			Graphics.QueueMessage(new GraphicsMessage(GM_PRINT_SCREENSHOT));
			break;
		case PRINT_TO_FILE:
			std::cout<<"\nPrint to file command issued.";
			MultimediaMan.SaveCurrentFramesToFile();
			break;
		case RELOAD_MODELS:
			std::cout<<"\nInput>>RELOAD_MODELS";
			Graphics.QueueMessage(new GraphicsMessage(GM_RELOAD_MODELS));
			break;
        case RELOAD_UI:
			std::cout<<"\nInput>>RELOAD_UI";
			Graphics.QueueMessage(new GraphicsMessage(GM_RELOAD_UI));
			break;
		case RECOMPILE_SHADERS:
			std::cout<<"\nInput>>RECOMPILE_SHADERS";
			Graphics.QueueMessage(new GraphicsMessage(GM_RECOMPILE_SHADERS));
			break;
		case GO_TO_EDITOR:
			std::cout<<"\nInput>>GO_TO_EDITOR";
			StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_EDITOR));
			break;
		case GO_TO_MAIN_MENU: {
			StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_MAIN_MENU));
			break;
		}
		case PRINT_FRAME_TIME:
		{
			std::cout<<"\nFrame time, total: "<<Graphics.FrameTime()<<" Average FPS: "<< 1000.0f / Graphics.FrameTime()
                <<" \n- MessageProcessing: "<<Graphics.graphicsMessageProcessingFrameTime
                <<" \n- Updates: "<<Graphics.graphicsUpdatingFrameTime
				<<" \n- Render: "<<Graphics.RenderFrameTime()
				<<" \n  - PreRender: "<<Graphics.preRenderFrameTime
				<<" \n  - PostRender(UI,grid): "<<Graphics.postViewportFrameTime
				<<" \n  - Swapbuffers: "<<Graphics.swapBufferFrameTime
                <<" \n  - Viewports: "<<Graphics.renderViewportsFrameTime;
			
			/*
            for (int i = 0; i < Graphics.GetViewports().Size() && i < 4; ++i)
                std::cout<<" \n    - v"<<i<<" : "<<Graphics.renderViewportFrameTime[i];

            std::cout<<" \n- Physics: "<<Graphics.PhysicsFrameTime()
				<<" \n  - Recalculating properties: "<<Physics.GetRecalculatingPropertiesFrameTime()
				<<" \n  - Movement: "<<Physics.GetMovementFrameTime()
				<<" \n  - Collision: "<<Physics.GetCollisionProcessingFrameTime()
				<<" \n    - PhysicsMeshCollisionChecks: "<<Physics.GetPhysicsMeshCollisionChecks()
				<<" \n  - Processing messages: "<<Physics.GetMessageProcessingFrameTime();
*/
			FrameStats.Print();

            break;
		}
		case PRINT_PLAYER_INPUT_DEVICES: {
			PlayerMan.PrintPlayerInputDevices();
			break;
		}
		case LIST_TEXTURES: {
            TexMan.ListTextures();
            break;
        }
        case LIST_MODELS: {
            ModelMan.ListObjects();
            break;
        }
        case LIST_CAMERAS: {
        	Graphics.ListCameras();
        	break;
        }
        case PRINT_UI_TREE:{
			if (StateMan.ActiveState()->GetUI())
				StateMan.ActiveState()->GetUI()->Print();
            break;
        }
		case COPY: {
#ifdef WINDOWS
			Window * window = WindowMan.GetCurrentlyActiveWindow();
			// Open the clipboard, and empty it.
			if (!OpenClipboard(window->hWnd)) {
				std::cout<<"Unable to open clipboard!";
				return;
			}
			EmptyClipboard();
			// Get a pointer to the structure for the selected label.
		//	pbox = (PLABELBOX) GetWindowLong(hwndSelected, 0);

			// If text is selected, copy it using the CF_TEXT format.
			if (true/*pbox->fEdit*/) {
			/*	if (pbox->ichSel == pbox->ichCaret)     // zero length
				{
					CloseClipboard();                   // selection
					return FALSE;
				} */
		 /*
				if (pbox->ichSel < pbox->ichCaret) {
					ich1 = pbox->ichSel;
					ich2 = pbox->ichCaret;
				}
				else {
					ich1 = pbox->ichCaret;
					ich2 = pbox->ichSel;
				}
				cch = ich2 - ich1;
		*/
				// Allocate a global memory Entity for the text.

		/*		const int MAX_SIZE = 1024;
				TCHAR text[MAX_SIZE];
				memset(text, 0, MAX_SIZE * sizeof(TCHAR));
				_tcscat(text, L"Test string!");
				int length = _tcslen(text);

				hglbCopy = GlobalAlloc(GMEM_MOVEABLE,
					length*7 * sizeof(TCHAR));
				if (hglbCopy == NULL) {
					CloseClipboard();
					std::cout<<"ERROR: Unable to allocate memory to global copy.";
					return;
				}
				// Lock the handle and copy the text to the buffer.
				lptstrCopy = (LPTSTR) GlobalLock(hglbCopy);
				memcpy(lptstrCopy, text,
					length * sizeof(TCHAR));
				lptstrCopy[length] = 0;    // null character
				GlobalUnlock(hglbCopy);

				// Place the handle on the clipboard.
				SetClipboardData(CF_TEXT, hglbCopy);
				*/
			}
			// If no text is selected, the label as a whole is copied.
		 /*
			else
			{
				// Save a copy of the selected label as a local memory
				// Entity. This copy is used to render data on request.
				// It is freed in response to the WM_DESTROYCLIPBOARD
				// message.

				pboxLocalClip = (PLABELBOX) LocalAlloc(
					LMEM_FIXED,
					sizeof(LABELBOX)
				);
				if (pboxLocalClip == NULL)
				{
					CloseClipboard();
					return FALSE;
				}
				memcpy(pboxLocalClip, pbox, sizeof(LABELBOX));
				pboxLocalClip->fSelected = FALSE;
				pboxLocalClip->fEdit = FALSE;

				// Place a registered clipboard format, the owner-display
				// format, and the CF_TEXT format on the clipboard using
				// delayed rendering.

				SetClipboardData(uLabelFormat, NULL);
				SetClipboardData(CF_OWNERDISPLAY, NULL);
				SetClipboardData(CF_TEXT, NULL);
			}
		 */
			// Close the clipboard.
			CloseClipboard();
#endif
			break;
		};
		case PASTE: {

#ifdef LINUX
            std::cout<<"\nPasting!";
            int result;
        //    XInternAtom();
            result = XSetSelectionOwner(display, XA_PRIMARY, window, CurrentTime);
            if (result == BadAtom){
                std::cout<<"\nBadAtom!";
            }
            else if (result == BadWindow){
                std::cout<<"\nBadWindow!";
            }

            int atom = XInternAtom(display, "Woshi", true);
            std::cout<<"\nAtom: "<<atom;

            int owner;
            result = XGetSelectionOwner(display, XA_PRIMARY);
            std::cout<<"\nSelection owner: "<<result;
            if (result == BadAtom){
                std::cout<<"\nBadAtom!";
            }
            else if (result == BadWindow){
                std::cout<<"\nBadWindow!";
            }
            else
                owner = result;

            std::cout<<"\nconvertSelection: ";
            XConvertSelection(display, XA_PRIMARY, XA_STRING, 7030, window, CurrentTime);
            if (result == BadAtom){
                std::cout<<"\nBadAtom!";
            }
            else if (result == BadWindow){
                std::cout<<"\nBadWindow!";
            }

  /*          result = XSetSelectionOwner(display, XA_SECONDARY, window, CurrentTime);
            if (result == BadAtom){
                std::cout<<"\nBadAtom!";
            }
            else if (result == BadWindow){
                std::cout<<"\nBadWindow!";
            }
            result = XSetSelectionOwner(display, XA_CUT_BUFFER0, window, CurrentTime);
            if (result == BadAtom){
                std::cout<<"\nBadAtom!";
            }
            else if (result == BadWindow){
                std::cout<<"\nBadWindow!";
            }
*/
            Atom a = XInternAtom(display, "", true);
#endif


#ifdef WINDOWS

			Window * window = WindowMan.GetCurrentlyActiveWindow();
		//	pbox = hwndSelected == NULL ? NULL :
		//		(PLABELBOX) GetWindowLong(hwndSelected, 0);

			// If the application is in edit mode,
			// get the clipboard text.
			if (true /*pbox != NULL && pbox->fEdit*/)
			{
				if (IsClipboardFormatAvailable(CF_BITMAP))
					std::cout<<"\nClipboardFormat CF_BITMAP available";
				if (IsClipboardFormatAvailable(CF_DIB))
					std::cout<<"\nClipboardFormat CF_DIB available";
				if (IsClipboardFormatAvailable(CF_DIF))
					std::cout<<"\nClipboardFormat CF_DIF available";
				if (IsClipboardFormatAvailable(CF_DSPENHMETAFILE))
					std::cout<<"\nClipboardFormat CF_DSPENHMETAFILE available";
				if (IsClipboardFormatAvailable(CF_DSPTEXT))
					std::cout<<"\nClipboardFormat CF_DSPTEXT available";
				/** A handle to type HDROP that identifies a list of files.
					An application can retrieve information about the files by passing
					the handle to the DragQueryFile function.
				*/
				if (IsClipboardFormatAvailable(CF_HDROP))
				{
					std::cout<<"\nClipboardFormat CF_HDROP available";
					if (!OpenClipboard(window->hWnd))
						return;

					HDROP hDrop = (HDROP) GetClipboardData(CF_HDROP);
					if (hDrop == NULL) {
						int error = GetLastError();
						std::cout<<"\nERROR: GetClipboardData failed: "<<error;
						if (error == 1418)
							std::cout<<": Clipboard not open";
						break;
					}
					const int MAX_FILES = 10;
					wchar_t filename[MAX_FILES][MAX_PATH];
					wchar_t fileSuffix[10];
					/// First extract amount of files available
					int result = DragQueryFileW(hDrop, 0xFFFFFFFF, filename[0], MAX_PATH);
					std::cout<<"\nINFO: Pasting from clipboard: 1 file(s):";
					for (int i = 0; i < result && i < MAX_FILES; ++i){
						int pathLength = DragQueryFileW(hDrop, i, filename[i], MAX_PATH);
#ifdef _UNICODE
						std::wcout<<"\n- "<<filename[i];
#else
						std::cout<<"\n- "<<filename[i];
#endif
					}
					// Close clipboard before we begin any further processing!
					assert(CloseClipboard());
					/// Go through and see if we should do anything with any of the files!
					List<String> files;
					for (int i = 0; i < result && i < MAX_FILES; ++i)
					{
						/// Check file-ending, deal with appropriately
						memset(fileSuffix, 0, sizeof(wchar_t) * 10);
						String file = filename[i];
						files.Add(file);
					}
					/// Send a message about it.
					FileEvent * fe = new FileEvent();
					fe->files = files;
					fe->msg = "PasteFiles";
					MesMan.QueueMessage(fe);
					return;
				}
				if (IsClipboardFormatAvailable(CF_UNICODETEXT))
				{
					std::cout<<"\nClipboardFormat CF_UNICODETEXT available";
					if (!OpenClipboard(window->hWnd))
						return;
					HANDLE h = GetClipboardData(CF_UNICODETEXT);
					// Close clipboard before we begin any further processing!
					assert(CloseClipboard());

			//		return;	
				}
				if (IsClipboardFormatAvailable(CF_WAVE))
					std::cout<<"\nClipboardFormat CF_WAVE available";
				if (IsClipboardFormatAvailable(CF_TIFF))
					std::cout<<"\nClipboardFormat CF_TIFF available";
				if (IsClipboardFormatAvailable(CF_TEXT))
				{
					if (!OpenClipboard(window->hWnd))
						return;
					hglb = GetClipboardData(CF_TEXT);
					if (hglb != NULL)
					{
						LPSTR lpstr = (LPSTR) GlobalLock(hglb);
						if (lpstr != NULL)	
						{
							// Call the application-defined ReplaceSelection
							// function to insert the text and repaint the
							// window.
							String text = lpstr;
							text.PrintData();
							PasteMessage * pasteMessage = new PasteMessage();
							pasteMessage->msg = "Paste:Text";
							pasteMessage->text = text;
							MesMan.QueueMessage(pasteMessage);
							//	ReplaceSelection(hwndSelected, pbox, lptstr);
							GlobalUnlock(hglb);
						}
					}
				
				}
				// Close clipboard before we begin any further processing!
				assert(CloseClipboard());
				return;
			}
#endif
			return;
		}
		case TOGGLE_FULL_SCREEN: 
		{
			Graphics.ToggleFullScreen(WindowMan.GetCurrentlyActiveWindow());
			/*
			std::cout<<"\nInput>>TOGGLE_FULL_SCREEN";
			// Check full-screen state
			if (Graphics.isFullScreen){
			    /// If was full-screen, go back to previous-size!
				if (Graphics.oldWidth < 200)
					Graphics.oldWidth = 200;
				if (Graphics.oldHeight < 200)
					Graphics.oldHeight = 200;
#ifdef WINDOWS
				extern DWORD windowStyle;
					/// Set window style
				windowStyle =
					WS_CAPTION |		// The window has a title bar (includes the WS_BORDER style).
					WS_MAXIMIZEBOX |	// The window has a maximize button. Cannot be combined with the WS_EX_CONTEXTHELP style. The WS_SYSMENU style must also be specified.
					WS_MINIMIZEBOX |	// The window has a minimize button. Cannot be combined with the WS_EX_CONTEXTHELP style. The WS_SYSMENU style must also be specified.
					WS_OVERLAPPED |		// The window is an overlapped window. An overlapped window has a title bar and a border. Same as the WS_TILED style.
					WS_SIZEBOX |		// The window has a sizing border. Same as the WS_THICKFRAME style.
					WS_SYSMENU |		// The window has a window menu on its title bar. The WS_CAPTION style must also be specified.
					WS_THICKFRAME |		// The window has a sizing border. Same as the WS_SIZEBOX style.
					WS_SYSMENU | WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE | // Have no idea..
					0;

				/// If not resizable, de-flag it
				extern bool notResizable;
				if (notResizable)
					windowStyle &= ~WS_SIZEBOX;

				SetWindowLongPtr(hWnd, GWL_STYLE,
					windowStyle);
				MoveWindow(hWnd,
					Graphics.ScreenWidth()/2 - Graphics.oldWidth/2,
					Graphics.ScreenHeight()/2 - Graphics.oldHeight/2,
					Graphics.oldWidth + 12, Graphics.oldHeight + 32, true);
#elif defined LINUX
                XResizeWindow(display, window, Graphics.oldWidth, Graphics.oldHeight);
#endif
                Graphics.isFullScreen = false;
			}
			else {
				/// Save away old sizes
				Graphics.oldWidth = Graphics.Width();
				Graphics.oldHeight = Graphics.Height();
#ifdef WINDOWS
				// Sets full-screen style if specified
				SetWindowLongPtr(hWnd, GWL_STYLE,
					WS_SYSMENU |
					WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);
				MoveWindow(hWnd, 0, 0, Graphics.ScreenWidth(), Graphics.ScreenHeight(), true);
#elif defined LINUX
                XResizeWindow(display, window, Graphics.ScreenWidth(), Graphics.ScreenHeight());
#endif
				Graphics.isFullScreen = true;
			}*/
			break;
		}
	}
}
