// Emil Hedemalm
// 2013-07-03
// For interpretting Win32 message-loop messages.

#include "WndProc.h"

#ifdef WINDOWS

#include <process.h>

#include "Globals.h"

#include "Input/InputManager.h"
#include "Graphics/GraphicsManager.h"
#include "StateManager.h"
#include "Message/MessageManager.h"
#include "OS/WindowManager.h"

/// Returns KeyCode depending on the Virtual Key provided by Win32
int GetKeyCodeFromVK(int wParam){
	switch(wParam){
	case VK_TAB: return KEY::TAB;
	case VK_OEM_COMMA: return KEY::COMMA;
	case VK_OEM_PERIOD: return KEY::PUNCTUATION;
	case VK_OEM_PLUS: return KEY::PLUS;		/// Standard plus/minus, not NUMPAD! o-o
	case VK_OEM_MINUS: return KEY::MINUS;
	case VK_MENU: return KEY::ALT;
	case VK_RETURN: return KEY::ENTER;
	case VK_CONTROL: return KEY::CTRL;
	case VK_SHIFT: return KEY::SHIFT;
	case VK_DOWN: return KEY::DOWN;
	case VK_UP: return KEY::UP;
	case VK_RIGHT: return KEY::RIGHT;
	case VK_LEFT: return KEY::LEFT;
	case VK_BACK: return KEY::BACKSPACE;
	case VK_ESCAPE: return KEY::ESCAPE;
	case VK_SPACE: return KEY::SPACE;
	case VK_PRIOR: return KEY::PG_UP;
	case VK_NEXT: return KEY::PG_DOWN;
	case VK_END: return KEY::END;
	case VK_HOME: return KEY::HOME;
	case VK_DELETE: return KEY::DELETE_KEY;
	case VK_PAUSE: return KEY::PAUSE_BREAK;
	};
	if (wParam >= 0x70 && wParam <= 0x7B)
		return KEY::F1 + wParam - 0x70;
	if (wParam >= 0x41 && wParam <= 0x5A)
		return KEY::A + wParam - 0x41;	// 0x41 = 65  0x51 = 81

	if (wParam >= 48 && wParam <= 57)
		return KEY::ZERO + wParam - 48;
	return 0;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR greeting[] = _T("Hello, World!");

/*	int * p;
	p = new int[5000];
	delete[] p;
*/
    switch (message)
    {
	case WM_INPUT:
		return 0;
	case WM_INPUT_DEVICE_CHANGE:
		return 0;
	case WM_COMMAND:	// Sent when the user selects a command item from a menu, when a control sends a notification message to its parent window, or when an accelerator keystroke is translated.
		return 0;	// If an application processes this message, it should return zero.
	case WM_INITMENUPOPUP:	// Sent when a drop-down menu or submenu is about to become active. This allows an application to modify the menu before it is displayed, without changing the entire menu.
		return 0;	// If an application processes this message, it should return zero.
	case WM_DROPFILES:{
		// Drag'n'drop!
		// http://msdn.microsoft.com/en-us/library/windows/desktop/bb774303%28v=vs.85%29.aspx
		HDROP hDrop = (HDROP) wParam;
		
		int numFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, NULL);
		std::cout<<"\nWM_DROPFILES: "<<numFiles<<" files dropped onto window.";
		List<String> files;
		for (int i = 0; i < numFiles; ++i){
			wchar_t file[1024];
			DragQueryFile(hDrop, i, file, 1024);
			String fileName = file;
			/// Replace backslashes with regular slashes.
			fileName.Replace('\\', '/');
			/// Try and make it relative straight away?
			FilePath path(fileName);
			fileName =  path.RelativePath();
			files.Add(fileName);
		}
		// When finishing up!
		DragFinish(hDrop);
		/// Call UI to handle it.
		bool handled = Input.HandleDADFiles(files);
		/// Call stateManager to handle Drag and Drop files varying on active context
		if (!handled)
			StateMan.HandleDADFiles(files);
		return 0;	// An application should return zero if it processes this message.
	}
	case WM_ACTIVATE: {
		/// Just got active
		if (wParam != WA_INACTIVE){
			WindowMan.SetFocus(true);
			std::cout<<"\nWindow received focus";
		}
		/// Got inactivated
		else {
			WindowMan.SetFocus(false);
			std::cout<<"\nWindow lost focus";
		}
		return 0;				  
	}
	case WM_KILLFOCUS:{	// Sent to a window immediately before it loses the keyboard focus.
//		std::cout<<"\nWM_KILLFOCUS message received. wParam: "<<wParam<<" lParam: "<<lParam;
//		std::cout<<"\nResetting input flags!";
		Input.ClearInputFlags();
		return 0;// An application should return zero if it processes this message.
	}
	case WM_SETFOCUS: {	// Sent to a window after it has gained the keyboard focus.
//		std::cout<<"\nWM_SETFOCUS message received. wParam: "<<wParam<<" lParam: "<<lParam;
//		std::cout<<"\nResetting input flags!";
		Input.ClearInputFlags();
		return 0;	// An application should return zero if it processes this message.
	}
	case WM_NCCALCSIZE: {	// Sent when the size and position of a window's client area must be calculated. By processing this message, an application can control the content of the window's client area when the size or position of the window changes.
		if (wParam) {
			// If wParam is TRUE, it specifies that the application should indicate
			// which part of the client area contains valid information. The system
			// copies the valid information to the specified area within the new client area.

			NCCALCSIZE_PARAMS * calcParams = (NCCALCSIZE_PARAMS*)lParam;
			// Adjust so we don't have a frame with transparent
			calcParams->rgrc[0].top -= 1;
			calcParams->rgrc[0].bottom += 1;
			calcParams->rgrc[0].left -= 1;
			calcParams->rgrc[0].right += 1;
			/// If full-screen, disable the additional parameters to get truly full-screen ^^
			if (calcParams->lppos->cx == Graphics.ScreenWidth() && calcParams->lppos->cy == Graphics.ScreenHeight())
				return WVR_ALIGNTOP | WVR_ALIGNLEFT;
		}
		if (wParam == false){
			/// If wParam is FALSE, lParam points to a RECT structure. On entry, the
			// structure contains the proposed window rectangle for the window. On exit,
			// the structure should contain the screen coordinates of the corresponding window client area.
			RECT * proposedWRect = (RECT*)lParam;
			proposedWRect->left += 2;
			proposedWRect->bottom += 2;
			return 0;
		}
		break;
	}
	case WM_SIZE: {		// Sent to a window after its size has changed.
		switch(wParam){
		case SIZE_MAXHIDE:	// Message is sent to all pop-up windows when some other window is maximized.
			break;
		case SIZE_MAXIMIZED: // Message is sent to all pop-up windows when some other window is maximized.
			break;
		case SIZE_MAXSHOW:	// Message is sent to all pop-up windows when some other window has been restored to its former size.
			break;
		case SIZE_MINIMIZED:// The window has been minimized.
			break;
		case SIZE_RESTORED: // The window has been resized, but neither the SIZE_MINIMIZED nor SIZE_MAXIMIZED value applies.
			break;
		default:
			break;
		}
		DWORD width = (lParam & 0xFFFF) + 2;
		DWORD height = (lParam >> 16) + 2;

		if (!Graphics.SetResolution(width, height))
			break;

		// If an application processes this message, it should return zero.
		return 0;
		break;
	}
	case WM_CHAR: {	// Posted to the window with the keyboard focus when a WM_KEYDOWN message is translated by the TranslateMessage function. // The WM_CHAR message contains the character code of the key that was pressed.
#ifdef LOG_KEYS
		std::cout<<"\nWM_CHAR received: keyCode: "<<wParam<<" ascii: "<<(char)wParam;
#endif
		Input.Char(wParam);
		return 0;	// An application should return zero if it processes this message.
		break;
	}
	case WM_SYSCOMMAND: {
		switch(wParam){
		case SC_CLOSE:		// Closes the window.

			break;
		case SC_HOTKEY:		// Activates the window associated with the application-specified hot key. The lParam parameter identifies the window to activate.
			break;
		case SC_KEYMENU:	// Retrieves the window menu as a result of a keystroke. For more information, see the Remarks section.
		//	std::cout<<"\nSC_KEYMENU (Alt) received.";
		//	Input.KeyDown(KEY::ALT);
			return 0;
			break;
		case SC_MOVE:		// Moves the window.
			break;
		};
		break;
	};
	case WM_SYSKEYDOWN:{
		/// If first occurence, flag it as first occurence
		int downBefore = lParam>>30;
#ifdef LOG_KEYS
		std::cout<<"\nWM_SYSKEYDOWN received: keyCode: "<<wParam<<" ascii: "<<(char)wParam;
#endif
		int keyCode = 0;
		keyCode = GetKeyCodeFromVK(wParam);
		// Pass keycode to input manager if applicable
		if (keyCode)
			Input.KeyDown(keyCode, downBefore > 0);
		return 0;	// An application should return zero if it processes this message.
		break;
	}
	case WM_SYSKEYUP:{
#ifdef LOG_KEYS
		std::cout<<"\nWM_SYSKEYUP received: keyCode: "<<wParam<<" ascii: "<<(char)wParam;
#endif
		int keyCode = 0;
		keyCode = GetKeyCodeFromVK(wParam);
		// Pass keycode to input manager if applicable
		if (keyCode)
			Input.KeyUp(keyCode);
		return 0;	// An application should return zero if it processes this message.
		break;
	}
	case WM_KEYDOWN: {
		/// If first occurence, flag it as first occurence
		int downBefore = lParam>>30;
#ifdef LOG_KEYS
		std::cout<<"\nWM_KEYDOWN received: keyCode: "<<wParam<<" ascii: "<<(char)wParam;
#endif
		int keyCode = 0;
		keyCode = GetKeyCodeFromVK(wParam);
		// Pass keycode to input manager if applicable
		if (keyCode)
			Input.KeyDown(keyCode, downBefore > 0);
		return 0;	// An application should return zero if it processes this message.
		break;
	}
	case WM_KEYUP: {
#ifdef LOG_KEYS
		std::cout<<"\nWM_KEYUP received: keyCode: "<<wParam<<" ascii: "<<(char)wParam;
#endif
		int keyCode = 0;
		keyCode = GetKeyCodeFromVK(wParam);
		// Pass keycode to input manager if applicable
		if (keyCode)
			Input.KeyUp(keyCode);
		return 0;	// An application should return zero if it processes this message.
		break;
	}

#define XtoWindow(x) (int)x
#define YtoWindow(x) (int)(Graphics.height - x)

	case WM_LBUTTONDOWN: {
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		Input.MouseClick(true, XtoWindow(xPos), YtoWindow(yPos));
		return 0; // If an application processes this message, it should return zero.
		break;
	}
	case WM_RBUTTONDOWN: {
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		Input.MouseRightClick(true, XtoWindow(xPos), YtoWindow(yPos));
		return 0; // If an application processes this message, it should return zero.
		break;
	}
	case WM_LBUTTONUP: {
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		Input.MouseClick(false, XtoWindow(xPos), YtoWindow(yPos));
		return 0; // If an application processes this message, it should return zero.
		break;
	}
	case WM_RBUTTONUP: {
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		Input.MouseRightClick(false, XtoWindow(xPos), YtoWindow(yPos));
		return 0; // If an application processes this message, it should return zero.
		break;
	}
	/// Posted when the user releases the left mouse button while the cursor is within the nonclient area of a window. 
	/// This message is posted to the window that contains the cursor. If a window has captured the mouse, this message is not posted.
	case WM_NCLBUTTONUP: {
		std::cout<<"\nWM_NCLBUTTONUP message received.";
		return 0;					 
	}
	case WM_MOUSEMOVE: {
		// Disable movements for it?
		if (Input.MouseLocked()){
		//	BlockInput(true);
			INPUT mouseInput;
			mouseInput.type = INPUT_MOUSE;
			MOUSEINPUT * mi = &mouseInput.mi;
			mi->dx = (LONG) (Graphics.ScreenWidth() * 0.5f * 20);
			mi->dy = (LONG) (Graphics.ScreenHeight() * 0.5f * 40);
			mi->mouseData = NULL;
			mi->dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE_NOCOALESCE;
			mi->time = 0;
			mi->dwExtraInfo = NULL;
			SendInput(1, &mouseInput, sizeof(INPUT));
			return 0;
		}
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		Input.MouseMove(XtoWindow(xPos), YtoWindow(yPos));
		return 0; // If an application processes this message, it should return zero.
		break;
	}
	case WM_MOUSEWHEEL: {
		//fwKeys = GET_KEYSTATE_WPARAM(wParam);
		float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		Input.MouseWheel(zDelta);
		return 0; // If an application processes this message, it should return zero.
		break;
	}
	case WM_PRINT: case WM_PRINTCLIENT:
	//	return 0;
		break; //return 0;
	case WM_SYNCPAINT:	// Message to synchronize painting while avoiding linking independent GUI threads.
	//	return 0;		// Return zero if processed.
		break;
	/// When queried to re-render because of other windows in front.
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms648055%28v=vs.85%29.aspx
	case WM_ERASEBKGND:
		Graphics.QueryRender();
		return 0;
		break;
	// http://msdn.microsoft.com/en-us/library/windows/desktop/dd145212%28v=vs.85%29.aspx
	case WM_NCPAINT:
	/// Regular paint message.
    case WM_PAINT:
		Graphics.QueryRender();
        break;
	// Sent to a window to query it's destruction. Ref: http://msdn.microsoft.com/en-us/library/windows/desktop/ms632617%28v=vs.85%29.aspx
	case WM_CLOSE:
		MesMan.QueueMessages("Query(QuitApplication)");
		return 0;
	// Sent when the window is already being destroyed: Ref: http://msdn.microsoft.com/en-us/library/windows/desktop/ms632620%28v=vs.85%29.aspx
    case WM_DESTROY:
		std::cout<<"\nWM_DESTROY received";
	//	assert(false);
		// Bring up a dialogue if the user is sure that it wants to quit! :)
	//	MesMan.QueueMessages("Query(QuitApplication)");
	//	PostQuitMessage(0);
		return 0;
        break;
	case WM_QUIT: {
		std::cout<<"\nWM_QUIT received";
		assert(false);
		// Nothing should be processing if we arrive here!
		int b = 3;
		b += 3;
		if (b == 2)
			b += message;
		std::cout<<"\n\nQuit message recieved!";
		break;
	}
    default:
       // return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }
	// Do default window processing if the above functions didn't already process the message and return a value!
	return DefWindowProc(hWnd, message, wParam, lParam);

    return 0;
}

#endif // WINDOWS

