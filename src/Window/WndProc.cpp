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
#include "Window/AppWindowManager.h"
#include "Application/Application.h"
#include "Message/WindowMessage.h"

/// Returns KeyCode depending on the Virtual Key provided by Win32
int GetKeyCodeFromVK(int wParam)
{
	switch(wParam)
	{
		case VK_VOLUME_DOWN: return 0;
		case VK_VOLUME_UP: return 0;
		case VK_TAB: return KEY::TAB;
		case VK_OEM_COMMA: return KEY::COMMA;
		case VK_OEM_PERIOD: return KEY::PUNCTUATION;
		case VK_OEM_PLUS: return KEY::PLUS;		/// Standard plus/minus, not NUMPAD! o-o
		case VK_OEM_MINUS: return KEY::MINUS;
		// From numpad!
		case VK_ADD: return KEY::PLUS;
		case VK_SUBTRACT: return KEY::MINUS;
		case VK_MULTIPLY: return KEY::MULTIPLY;
		case VK_DECIMAL: return KEY::PERIOD;
		case VK_NUMPAD0: return KEY::ZERO;
		case VK_NUMPAD1: return KEY::ONE;
		case VK_NUMPAD2: return KEY::TWO;
		case VK_NUMPAD3: return KEY::THREE;
		case VK_NUMPAD4: return KEY::FOUR;
		case VK_NUMPAD5: return KEY::FIVE;
		case VK_NUMPAD6: return KEY::SIX;
		case VK_NUMPAD7: return KEY::SEVEN;
		case VK_NUMPAD8: return KEY::EIGHT;
		case VK_NUMPAD9: return KEY::NINE;		
		// CTRL, Shift, ALT
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
		case VK_PRINT:
		case VK_SNAPSHOT: return KEY::PRINT_SCREEN;
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
//  PURPOSE:  Processes messages for the main AppWindow.
//
//  WM_PAINT    - Paint the main AppWindow
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

	AppWindow * window = WindowMan.GetWindowByHWND(hWnd);

    switch (message)
    {
	case WM_MOVE: 
	{
		int xPos = (int)(short) LOWORD(lParam);   // horizontal position 
		int yPos = (int)(short) HIWORD(lParam);   // vertical position 
		Vector2i currPos(xPos, yPos);	
		if (window->IsMain())
		{
			/// Update all other windows which have relative positions?
			Vector2i prevPos = window->previousPosition;
			Vector2i diff = currPos - prevPos;
			WindowMan.UpdateChildWindows();
		}
		window->previousPosition = window->position;
		window->position = currPos;
		return 0;
	}
	case WM_DEADCHAR:
		return 0;
	case WM_SYSDEADCHAR:
		return 0;
	case WM_INPUT:
		return 0;
	case WM_INPUT_DEVICE_CHANGE:
		return 0;
	case WM_COMMAND:	// Sent when the user selects a command item from a menu, when a control sends a notification message to its parent AppWindow, or when an accelerator keystroke is translated.
		return 0;	// If an application processes this message, it should return zero.
	case WM_INITMENUPOPUP:	// Sent when a drop-down menu or submenu is about to become active. This allows an application to modify the menu before it is displayed, without changing the entire menu.
		return 0;	// If an application processes this message, it should return zero.
	case WM_DROPFILES:{
		// Drag'n'drop!
		// http://msdn.microsoft.com/en-us/library/windows/desktop/bb774303%28v=vs.85%29.aspx
		HDROP hDrop = (HDROP) wParam;
		
		int numFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, NULL);
		std::cout<<"\nWM_DROPFILES: "<<numFiles<<" files dropped onto AppWindow.";
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
		bool handled = InputMan.HandleDADFiles(files);
		/// Call stateManager to handle Drag and Drop files varying on active context
		if (!handled)
			StateMan.HandleDADFiles(files);
		return 0;	// An application should return zero if it processes this message.
	}
	case WM_ACTIVATE: {
		/// Just got active
		if (wParam != WA_INACTIVE)
		{
			window->inFocus = true;
		}
		/// Got inactivated
		else {
			window->inFocus = false;
		}
		return 0;				  
	}
	case WM_KILLFOCUS:{	// Sent to a window immediately before it loses the keyboard focus.
//		std::cout<<"\nWM_KILLFOCUS message received. wParam: "<<wParam<<" lParam: "<<lParam;
//		std::cout<<"\nResetting input flags!";
		InputMan.ClearInputFlags();
		return 0;// An application should return zero if it processes this message.
	}
	case WM_SETFOCUS: {	// Sent to a window after it has gained the keyboard focus.
//		std::cout<<"\nWM_SETFOCUS message received. wParam: "<<wParam<<" lParam: "<<lParam;
//		std::cout<<"\nResetting input flags!";
		InputMan.ClearInputFlags();
		return 0;	// An application should return zero if it processes this message.
	}
	case WM_NCCALCSIZE: {	// Sent when the size and position of a AppWindow's client area must be calculated. By processing this message, an application can control the content of the AppWindow's client area when the size or position of the window changes.
		if (wParam) {
			// If wParam is TRUE, it specifies that the application should indicate
			// which part of the client area contains valid information. The system
			// copies the valid information to the specified area within the new client area.
			//NCCALCSIZE_PARAMS * calcParams = (NCCALCSIZE_PARAMS*)lParam;
			//PWINDOWPOS windowPos = calcParams->lppos;
			//Vector2i requestedSize = Vector2i(windowPos->cx, windowPos->cy);
			//Vector2i newSize = Vector2i::Maximum(requestedSize, window->minimumSize);
			//RECT * rgrc = calcParams->rgrc;
			//RECT newRect = calcParams->rgrc[0];
			//// 1 - previous window rect
			//// 2 - previous client area rect.
			//int paddingLeft = rgrc[2].left - rgrc[1].left;
			//int paddingTop = rgrc[2].top - rgrc[1].top;
			//int paddingRight = rgrc[1].right - rgrc[2].right;
			//int paddingBottom = rgrc[1].bottom - rgrc[2].bottom;
			//calcParams->rgrc[0] = RECT{ newRect.left + paddingLeft, newRect.top + paddingTop, newRect.left + newSize.x - paddingRight, newRect.top + newSize.y - paddingBottom };
			//// https://docs.microsoft.com/sv-se/windows/win32/api/winuser/ns-winuser-nccalcsize_params?redirectedfrom=MSDN
			//return WVR_REDRAW;
			/*
			/// If full-screen, disable the additional parameters to get truly full-screen ^^
			if (calcParams->lppos->cx == Graphics.ScreenWidth() && calcParams->lppos->cy == Graphics.ScreenHeight())
			{
			}	
			// Adjust so we don't have a frame with transparency if small.
			else {
				int mod = 0;
				calcParams->rgrc[0].top -= mod;
				calcParams->rgrc[0].bottom += mod;
				calcParams->rgrc[0].left -= mod;
				calcParams->rgrc[0].right += mod;	
		//		return 0;
		//		return WVR_VALIDRECTS | WVR_REDRAW;
			}
			*/
		}
		if (wParam == false){
			/// If wParam is FALSE, lParam points to a RECT structure. On entry, the
			// structure contains the proposed window rectangle for the AppWindow. On exit,
			// the structure should contain the screen coordinates of the corresponding window client area.
			RECT * proposedWRect = (RECT*)lParam;
		//	proposedWRect->left += 2;
		//	proposedWRect->bottom += 2;
		//	return 0;
		}
		break;
	}
	case WM_SHOWWINDOW:	
	{
		window->visible = wParam;
		return 0;
		break;
	}
	case WM_SIZING:
	{
		RECT * rect = (RECT*)lParam;
		Vector2i requestedSize = Vector2i(rect->right - rect->left, rect->bottom - rect->top);
		Vector2i updatedSize = Vector2i::Maximum(window->minimumSize, requestedSize);
		rect->right = rect->left + updatedSize.x;
		rect->bottom = rect->top + updatedSize.y;
		return TRUE;
	}
	case WM_SIZE: 
	{		// Sent to a window after its size has changed.
		switch(wParam){
		case SIZE_MAXHIDE:	// Message is sent to all pop-up windows when some other window is maximized.
			break;
		case SIZE_MAXIMIZED: // Message is sent to all pop-up windows when some other window is maximized.
			break;
		case SIZE_MAXSHOW:	// Message is sent to all pop-up windows when some other window has been restored to its former size.
			break;
		case SIZE_MINIMIZED:
			// The window has been minimized. Just return.
			window->visible = false;
			return 0;
			break;
		case SIZE_RESTORED: // The window has been resized, but neither the SIZE_MINIMIZED nor SIZE_MAXIMIZED value applies.
//			std::cout<<"RESTORE";
			window->visible = true;
			break;
		default:
			break;
		}
		DWORD width = (lParam & 0xFFFF);
		DWORD height = (lParam >> 16);

		window->SetClientAreaSize(Vector2i(width, height));
//		std::cout<<"\nWindow "<<window->name<<" new size: "<<width<<"x"<<height;
		// Recalculate OS window size.
		window->osWindowSize = window->OSWindowSize();

		window->OnSizeUpdated();

		// If an application processes this message, it should return zero.
		return 0;
		break;
	}
	case WM_CHAR: {	// Posted to the window with the keyboard focus when a WM_KEYDOWN message is translated by the TranslateMessage function. // The WM_CHAR message contains the character code of the key that was pressed.
#ifdef LOG_KEYS
		std::cout<<"\nWM_CHAR received: keyCode: "<<wParam<<" ascii: "<<(char)wParam;
#endif
		InputMan.Char(window, wParam);
		return 0;	// An application should return zero if it processes this message.
		break;
	}
	case WM_SYSCOMMAND: {
		switch(wParam){
		case SC_CLOSE:		// Closes the AppWindow.

			break;
		case SC_HOTKEY:		// Activates the window associated with the application-specified hot key. The lParam parameter identifies the AppWindow to activate.
			break;
		case SC_KEYMENU:	// Retrieves the AppWindow menu as a result of a keystroke. For more information, see the Remarks section.
		//	std::cout<<"\nSC_KEYMENU (Alt) received.";
		//	InputMan.KeyDown(KEY::ALT);
			return 0;
			break;
		case SC_MOVE:		// Moves the AppWindow.
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
		{
			InputMan.KeyDown(window, keyCode, downBefore > 0);
		}
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
		{
			InputMan.KeyUp(window, keyCode);
		}
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
		{
			InputMan.KeyDown(window, keyCode, downBefore > 0);
		}
		return 0;	// An application should return zero if it processes this message.
		break;
	}
	case WM_KEYUP: {
#ifdef LOG_KEYS
		std::cout<<"\nWM_KEYUP received: keyCode: "<<wParam<<" ascii: "<<(char)wParam;
#endif
		int keyCode = 0;
		keyCode = GetKeyCodeFromVK(wParam);
		// Special cases?
		switch(keyCode)
		{
		case KEY::PRINT_SCREEN:
			InputMan.KeyDown(window, keyCode, false);
			break;
		}
		// Pass keycode to input manager if applicable
		if (keyCode)
			InputMan.KeyUp(window, keyCode);
		return 0;	// An application should return zero if it processes this message.
		break;
	}

#define XtoWindow(coord) (int)coord
#define YtoWindow(coord) (int)(window->clientAreaSize[1] - coord)

	case WM_LBUTTONDOWN: 
	{
		SetCapture(hWnd);
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		InputMan.MouseClick(window, true, XtoWindow(xPos), YtoWindow(yPos));
		return 0; // If an application processes this message, it should return zero.
		break;
	}
	case WM_RBUTTONDOWN: 
	{
		SetCapture(hWnd);
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		InputMan.MouseRightClick(window, true, XtoWindow(xPos), YtoWindow(yPos));
		return 0; // If an application processes this message, it should return zero.
		break;
	}
	case WM_LBUTTONUP: 
	{
		ReleaseCapture();
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		InputMan.MouseClick(window, false, XtoWindow(xPos), YtoWindow(yPos));
		return 0; // If an application processes this message, it should return zero.
		break;
	}
	case WM_RBUTTONUP: 
	{
		ReleaseCapture();
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		InputMan.MouseRightClick(window, false, XtoWindow(xPos), YtoWindow(yPos));
		return 0; // If an application processes this message, it should return zero.
		break;
	}
	/// Posted when the user releases the left mouse button while the cursor is within the nonclient area of a AppWindow. 
	/// This message is posted to the AppWindow that contains the cursor. If a window has captured the mouse, this message is not posted.
	case WM_NCLBUTTONUP: {
		std::cout<<"\nWM_NCLBUTTONUP message received.";
		return 0;					 
	}
	case WM_MOUSEMOVE: 
	{
		// Disable movements for it?
		if (InputMan.MouseLocked()){
			assert(false && "Fix");

		////	BlockInput(true);
		//	INPUT mouseInput;
		//	mouseInputMan.type = INPUT_MOUSE;
		//	MOUSEINPUT * mi = &mouseInputMan.mi;
		//	mi->dx = (LONG) (Graphics.ScreenWidth() * 0.5f * 20);
		//	mi->dy = (LONG) (Graphics.ScreenHeight() * 0.5f * 40);
		//	mi->mouseData = NULL;
		//	mi->dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE_NOCOALESCE;
		//	mi->time = 0;
		//	mi->dwExtraInfo = NULL;
		//	SendInput(1, &mouseInput, sizeof(INPUT));
		//	
			return 0;
		}
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
//		std::cout<<"\nMouse move: "<<xPos<<" "<<yPos;
		InputMan.MouseMove(window, Vector2i(XtoWindow(xPos), YtoWindow(yPos)));
		WindowMan.hoverWindow = window;
		return 0; // If an application processes this message, it should return zero.
		break;
	}
	case WM_MOUSEWHEEL: {
		//fwKeys = GET_KEYSTATE_WPARAM(wParam);
		float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
	//	std::cout<<" zDelta pre: "<<zDelta;
		zDelta /= WHEEL_DELTA;
		zDelta /= 10.f;
	//	std::cout<<" post: "<<zDelta;
		InputMan.MouseWheel(window, zDelta);
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
		/// Hide or destroy the window, depends on what kind of window it is and application settings.
		window->Close();
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

