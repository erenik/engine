// Emil Hedemalm
// 2013-07-04

#include "XProc.h"

#ifdef USE_X11

#include "Input/InputManager.h"
#include "Graphics/GraphicsManager.h"
#include "Message/MessageManager.h"

#include "AppWindow.h"
#include "AppWindowManager.h"
#undef Time

#include <GL/glew.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>  // contains visual information masks and CVisualInfo structure
#include <GL/glx.h>     // connect X server with OpenGL

// Keyboard encoding: xlib.pdf page 253
#include <X11/keysymdef.h>  // Whole symbol list
#include <X11/keysym.h>     // Less comprehensive list (Latin1-4 + greek + misc.)

#include <iostream>

/// Program start-up variables!
extern    XEvent                  event;
extern    GLXContext              context; // OpenGL context
extern    Display*                xDisplay; // connection to X server
extern    XVisualInfo*            xVisualInfo;
extern    Window                  window;
extern    XSetWindowAttributes    window_attributes;
extern    Colormap                colormap;
extern    bool                    swapBuffers;

extern XEvent event;

int GetCharFromXK(int xKey){
    // XKeys are supposed to be mapped to ASCII for the first 200ish!
    if (xKey >= 32 && xKey <= 126)
        return xKey;
    else if (xKey >= 128 && xKey <= 254)
        return xKey;
   /* if (xKey >= XK_A && xKey < XK_Z)
        return xKey;
    else if (xKey >= XK_a && xKey < XK_z)
        return xKey;
    switch(xKey){
        case XK_Return: case XK_Linefeed: return '\n';
        case XK_comma: return ',';
        case XK_period: return '.';
        case XK_minus: return '-';
        case XK_plus: return '+';
    }
    */
    return 0;
}

/// Returns KeyCode depending on the X-Key provided by X11
int GetKeyCodeFromXK(int xk){
    // A - Z
    if (xk >= XK_A && xk < XK_Z)
        return KEY::A + xk - XK_A;
    else if (xk >= XK_a && xk < XK_z)
        return KEY::a + xk - XK_a;
    // 0 - 9
    if (xk >= XK_0 && xk < XK_9)
        return KEY::ZERO + xk - XK_0;
    // F1 - F11
    if (xk >= XK_F1 && xk < XK_F11)
        return KEY::F1 + xk - XK_F1;
    // Special keys
	switch(xk){
	    case XK_BackSpace: return KEY::BACKSPACE;
	    case XK_Tab:    return KEY::TAB;
	    case XK_Linefeed: return KEY::ENTER; // Doesn't seem to be used
//	    case XK_Clear:  return KEY::CLEAR; // Clear?
	    case XK_Return: return KEY::ENTER;
	    case XK_Pause: return KEY::PAUSE_BREAK;
	    case XK_Scroll_Lock: return KEY::SCROLL_LOCK;
	    case XK_Sys_Req: return 0;
	    case XK_Escape: return KEY::ESCAPE;
	    case XK_Delete: return KEY::DELETE_KEY;
        case XK_Home: return KEY::HOME;
        case XK_Left: return KEY::LEFT;
        case XK_Right: return KEY::RIGHT;
        case XK_Up: return KEY::UP;
        case XK_Down: return KEY::DOWN;
        case XK_Page_Up: return KEY::PG_UP;
        case XK_Page_Down: return KEY::PG_DOWN;
        case XK_End: return KEY::END;
        // Tested until here..- o-o

        case XK_plus: return KEY::PLUS;
        case XK_minus: return KEY::MINUS;
        case XK_comma: return KEY::COMMA;
        case XK_period: return KEY::PERIOD;

        case XK_Shift_L: case XK_Shift_R: return KEY::SHIFT;
        case XK_Control_L: case XK_Control_R: return KEY::CTRL;
        case XK_Caps_Lock: return KEY::CAPS_LOCK;
        case XK_Alt_L: case XK_Alt_R: return KEY::ALT;
        case XK_space: return KEY::SPACE;

	}
    std::cout<<"\nUndefined Xkey code "<<xk;
	return 0;
}

/// XWindow Script Processor, return NULL for basic actions, integer numbers for exit codes.
void * XProc()
{
    // Check which window it came from.
    XAnyEvent & anyEvent = (XAnyEvent&) event;
    int type = anyEvent.type;
    Window xWindow = anyEvent.window;
    AppWindow * appWindow;
    List<AppWindow*> windows = WindowMan.GetWindows();
    for (int i = 0; i < windows.Size(); ++i)
    {
        AppWindow * appWin = windows[i];
        if (appWin->xWindowHandle == xWindow)
        {
            appWindow = appWin;
            break;
        }
    }

    static float x, y;
    static int button, state;
    static int xKey, keyCode;
#define PRINT   std::cout
    switch(event.type)
    {

        // Events ref: http://tronche.com/gui/x/xlib/events/structures.html
    case GraphicsExpose:
    {
        std::cout<<"\nXGraphicsExpose";
        break;
    }
    case NoExpose:
        std::cout<<"\nXNoExpose";
        break;
    case VisibilityNotify:
        std::cout<<"\nXVisibilityNotify";
        break;
    case Expose: {
        std::cout<<"\nXExposeEvent";
        appWindow->visible = true;
        break;
    }
    /// This event is generated when a window is created. X.SubstructureNotifyMask must be selected on the parent of the new window to receive this event. 
    case CreateNotify: 
    {
        std::cout<<"\nXCreateNotify";
        appWindow->created = true; // Mark as created.
        break;
    }
    /// Sent after destroying my AppWindow? D:
    case DestroyNotify: {
        std::cout<<"\nXDestroyNotify";
        break;
    }
    case UnmapNotify:
        std::cout<<"\nXUnmapNotify";
        break;
    case MapNotify:
        std::cout<<"\nXMapNotify";
        break;
    case MapRequest:
        std::cout<<"\nXMapRequest";
        break;
    /// When the AppWindow loses/receives input-focus?
    // http://tronche.com/gui/x/xlib/events/input-focus/#XFocusChangeEvent
    case FocusIn: {
        std::cout<<"\nXFocusIn";
        break;
    }
    case FocusOut: {
        std::cout<<"\nXFocusOut";
        /// Clears flags for all input keys. Returns amount of keys that were in need of resetting.
        int p = Input.ClearInputFlags();
//        std::cout<<"\n "<<p<<" input flags cleared.";
        break;   
    }
    case ConfigureNotify: 
    {
        std::cout<<"\nXConfigureNotify received. New info received.";
        XConfigureEvent& e = (XConfigureEvent &) event;
        int x, y, width, height;
        width = e.width;
        height = e.height;
        appWindow->clientAreaSize = appWindow->osWindowSize = Vector2i(width, height);        
        return NULL;
        break;

    }

	case KeyPress: 
	{
        std::cout<<"\nXKeyPress";
		bool upperCase = Input.KeyPressed(KEY::SHIFT);
		xKey = (int) XLookupKeysym(&event.xkey, 0);
		//        std::cout << "\nXLookupKeysym "<<(int)xKey<<" "<<(char)xKey;
		keyCode = GetKeyCodeFromXK(xKey);
		if (keyCode)
		    Input.KeyDown(keyCode, false);
		/// Send down the Keysym straight away for chars..!
		int modifier = 0;
		if (Input.KeyPressed(KEY::SHIFT))
		    ++modifier;
		int xKey = (int) XLookupKeysym(&event.xkey, modifier);
		//     std::cout << "\nXLookupKeysym "<<(int)xKey<<" "<<(char)xKey;
		int c = GetCharFromXK(xKey);
		//     std::cout<<"\nGetCharFromXK: "<<c<<" "<<(char)c;
		if (c)
		    Input.Char(c);
		break;
	}
    case KeyRelease: 
    {
        std::cout<<"\nXKeyRelease";
        unsigned short is_retriggered = 0;
        if (!is_retriggered){
       //     std::cout << "\nKey Release: ";
            xKey = (int) XLookupKeysym(&event.xkey, 0);
        //    std::cout << "key " << xKey << " was released.";
            keyCode = GetKeyCodeFromXK(xKey);
            if (keyCode)
                Input.KeyUp(keyCode);
        }
        break;
    }
    case KeymapNotify:
    {
        std::cout<<"\nXKeymapNotify";
        break;
    }
    case MotionNotify: 
    {
        std::cout<<"\nXMotionNotify";
        x = event.xbutton.x;
#define Y_UP_0_AT_BOTTOM(iny) (appWindow->ClientAreaSize().y - iny)
        y = Y_UP_0_AT_BOTTOM(event.xbutton.y);
      //  PRINT << " " << event[0]button[0] << "," << event[0]button[1];
        Input.MouseMove(appWindow, Vector2i(x,y));
        break;
    }
    case ButtonPress:
    {
        std::cout<<"\nXButtonPress";
   //     std::cout << "ButtonPress: ";
        x = event.xbutton.x;
        y = Y_UP_0_AT_BOTTOM(event.xbutton.y);
        state = event.xbutton.state;
        button = event.xbutton.button;
   //     PRINT << event[0]button.button << " " << event[0]button[0] << "," << event[0]button[1] << ", "<< state <<std::endl;
        /// Left clickur
        if (button == Button1)
            Input.MouseClick(appWindow, true, x, y);
        /// Right clickur
        if (button == Button3)
            Input.MouseRightClick(appWindow, true, x, y);
        /// Scrollur
        if (button == Button4)
            Input.MouseWheel(appWindow, 1);
        else if (button == Button5)
            Input.MouseWheel(appWindow, -1);
        break;
    }
    case ButtonRelease:
        x = event.xbutton.x;
        y = Y_UP_0_AT_BOTTOM(event.xbutton.y);
        button = event.xbutton.button;
 //       std::cout << "ButtonRelease: ";
 //       PRINT << event[0]button.button << " " << event[0]button[0] << "," << event[0]button[1] << std::endl;
        if (button == Button1)
            Input.MouseClick(appWindow, false, x, y);
        if (button == Button3)
            Input.MouseRightClick(appWindow, false, x, y);
        break;
    case EnterNotify:
        std::cout << "\nXEnterNotify";
        break;
    case LeaveNotify:
        std::cout << "\nXLeaveNotify";
        break;
    // Special messages! 
    case ClientMessage:
    {
        std::cout<<"\nXClientMessage";
        Atom wm_protocol = XInternAtom (xDisplay, "WM_PROTOCOLS", False);
        Atom wm_delete_window = XInternAtom (xDisplay, "WM_DELETE_WINDOW", False);
        // In our case primarily requests to close the AppWindow! Ref: http://www.opengl.org/discussion_boards/showthread.php/157469-Properly-destroying-a-AppWindow
        if (event.xclient.message_type == wm_protocol)
        {
            std::cout<<"\nWindowManager protocol (wm_protocol)";
            // Delete window event?
            if ((Atom)event.xclient.data.l[0] == wm_delete_window)
            {
                /// Hide or destroy the window, depends on what kind of window it is and application settings.
                appWindow->Close();
            }
        }
        break;
    }
    case SelectionRequest:
        std::cout<<"\nXSelectionRequest";
        break;
    case SelectionNotify: 
    {
        std::cout<<"\nXSelectionNotify";

        XSelectionEvent & selectionEvent =(XSelectionEvent &)event;
        std::cout<<"\nSelectionNotify! :)";
        std::cout<<"send_event:"<<selectionEvent.send_event
            <<" requestor: "<<selectionEvent.requestor
            <<" selection: "<<selectionEvent.selection
            <<" target: "<<selectionEvent.target
            <<" property: "<<selectionEvent.property;
        break;
    }
    default:
        std::cout<<"\nUnknown/untracked XEvent received: "<<type;
    //    std::cout<<"\nNOTE: Undefined message received in XProc! typeID: "<<event.type;
        break;
    }
 //   PRINT << "\n";
 //   std::cout<<"Message "<<int(event.type)<<": ";
    return NULL;
}

#endif // LINUX
