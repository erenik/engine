/// Emil Hedemalm
/// 2015-01-17
/// Utility functions for OS-specific tasks.

#include "OSUtil.h"
#include "Message/FileEvent.h"
#include "Message/MessageManager.h"

/// Declaring stuff before the actual functions.
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
#endif


#ifdef WINDOWS
List<String> GetFilesFromHDrop(HDROP hDrop)
{
	List<String> files;
	if (hDrop == NULL) {
		int error = GetLastError();
		std::cout<<"\nERROR: GetClipboardData failed: "<<error;
		if (error == 1418)
			std::cout<<": Clipboard not open";
		return files;
	}
	const int MAX_FILES = 10;
	wchar_t filename[MAX_FILES][MAX_PATH];
	for (int i = 0; i < MAX_PATH; ++i)
	{
		filename[0][i] = 0;
	}
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
	/// Go through and see if we should do anything with any of the files!
	for (int i = 0; i < result && i < MAX_FILES; ++i)
	{
		/// Check file-ending, deal with appropriately
		memset(fileSuffix, 0, sizeof(wchar_t) * 10);
		String file = filename[i];
		files.Add(file);
	}
	return files;
}
#endif


#include "Window/DragAndDrop.h"
#include "Window/AppWindowManager.h"

void OSUtil::Copy()
{
#ifdef WINDOWS
	AppWindow * AppWindow = WindowMan.GetCurrentlyActiveWindow();
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
}

void OSUtil::Paste()
{
#ifdef LINUX
    std::cout<<"\nPasting!";
    int result;
//    XInternAtom();
    result = XSetSelectionOwner(display, XA_PRIMARY, AppWindow, CurrentTime);
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
    XConvertSelection(display, XA_PRIMARY, XA_STRING, 7030, AppWindow, CurrentTime);
    if (result == BadAtom){
        std::cout<<"\nBadAtom!";
    }
    else if (result == BadWindow){
        std::cout<<"\nBadWindow!";
    }

/*          result = XSetSelectionOwner(display, XA_SECONDARY, AppWindow, CurrentTime);
    if (result == BadAtom){
        std::cout<<"\nBadAtom!";
    }
    else if (result == BadWindow){
        std::cout<<"\nBadWindow!";
    }
    result = XSetSelectionOwner(display, XA_CUT_BUFFER0, AppWindow, CurrentTime);
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

	AppWindow * AppWindow = WindowMan.GetCurrentlyActiveWindow();
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
			
			List<String> files = GetFilesFromHDrop(hDrop);

			// Close clipboard before we begin any further processing!
			assert(CloseClipboard());


			assert(files.Size());
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
					// AppWindow.
					String text = lpstr;
					std::cout<<"\nPaste text intercepted: "<<text;
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
}


#ifdef WINDOWS
#include "Windows.h"
#include "Shlobj.h"
#elif defined LINUX
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif


String OSUtil::GetHomeDirectory()
{
#ifdef WINDOWS
	// Get foldeeeer
	String folderDir = "";
	WCHAR homePathBuf[MAX_PATH];
	HRESULT hr = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL,
								 SHGFP_TYPE_CURRENT, homePathBuf);
#elif defined LINUX	
	/// http://linux.die.net/man/3/getpwuid_r
	struct passwd * passwordEntryFile;
	passwordEntryFile = getpwuid(getuid());
	const char * homePathBuf = passwordEntryFile->pw_dir;
#endif
	return String(homePathBuf);
}