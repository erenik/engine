/// Emil Hedemalm
/// 2014-08-21
/// Class for handling Drag an drop events.

#include "OS/OS.h"

#ifdef WINDOWS

#include "DragAndDrop.h"
#include <iostream>
#include <cassert>
#include "String/AEString.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"

#include "Window/WindowManager.h"

#include "Input/InputManager.h"

DragAndDrop::DragAndDrop()
{
	refCount = 1;
}

// Required interface fucntions
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms682521%28v=vs.85%29.aspx
HRESULT STDMETHODCALLTYPE DragAndDrop::QueryInterface( 
    /* [in] */ REFIID riid,
	/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject)
{
	int id = riid.Data1;
	// Check if interface is supported.
	switch(id)
	{
		default:
			*ppvObject = NULL;
			return E_NOINTERFACE;
		case 46:
			break;
	}
	return S_OK;
}

// http://msdn.microsoft.com/en-us/library/windows/desktop/ms691379%28v=vs.85%29.aspx
ULONG STDMETHODCALLTYPE DragAndDrop::AddRef( void)
{
	++refCount;
	return refCount;
}
ULONG STDMETHODCALLTYPE DragAndDrop::Release( void)
{
	--refCount;
	if (refCount <= 0)
	{
		delete this;
		return 0;
	}
	return refCount;
}

// http://msdn.microsoft.com/en-us/library/windows/desktop/ms680106%28v=vs.85%29.aspx
HRESULT STDMETHODCALLTYPE DragAndDrop::DragEnter( 
	/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
	/* [in] */ DWORD grfKeyState,
	/* [in] */ POINTL pt,
	/* [out][in] */ __RPC__inout DWORD *pdwEffect)
{


	/// Check if the application supports dropping at the location.
	SetPdwEffect(pt, pdwEffect);
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE DragAndDrop::DragOver( 
	/* [in] */ DWORD grfKeyState,
	/* [in] */ POINTL pt,
	/* [out][in] */ __RPC__inout DWORD *pdwEffect)
{
	/// Check if the application supports dropping at the location.
	SetPdwEffect(pt, pdwEffect);

	/// Fetch window co-ordinates from screen co-ordinates, yo..
	Window * hoverWindow = HoverWindow();
	if (hoverWindow)
	{
		/// Create own mouse-move effects to see that it works.
		Vector2i windowCoords = hoverWindow->GetWindowCoordsFromScreenCoords(Vector2i(pt.x, pt.y));
		InputMan.MouseMove(HoverWindow(), windowCoords);
	}	

	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE DragAndDrop::DragLeave( void)
{
	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE DragAndDrop::Drop( 
	/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
	/* [in] */ DWORD grfKeyState,
	/* [in] */ POINTL pt,
	/* [out][in] */ __RPC__inout DWORD *pdwEffect)
{
	/// Check if the application supports dropping at the location.
	SetPdwEffect(pt, pdwEffect);
	
	/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms678431%28v=vs.85%29.aspx
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms682177%28v=vs.85%29.aspx
	FORMATETC format;
	format.cfFormat = CF_TEXT;
	format.ptd = NULL;
	format.dwAspect = DVASPECT::DVASPECT_CONTENT;
	format.lindex = -1;
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms691227%28v=vs.85%29.aspx
	// Use global memory for the transfer? Or accept all types?
	format.tymed = TYMED::TYMED_HGLOBAL | TYMED::TYMED_FILE | TYMED::TYMED_ISTREAM |
		TYMED::TYMED_GDI | TYMED::TYMED_ISTORAGE; 

	STGMEDIUM medium;
	
	int result = pDataObj->GetData(&format, &medium);
	assert(result == S_OK);
	if (result != S_OK)
		return S_FALSE;

	// Fetch the data
	switch(medium.tymed)
	{
		case TYMED::TYMED_HGLOBAL:
		{
			std::cout<<"\nGet the stuffs D:";
			// ask the IDataObject for some CF_TEXT data, stored as a HGLOBAL
			// We need to lock the HGLOBAL handle because we can't
			// be sure if this is GMEM_FIXED (i.e. normal heap) data or not
			char *data = (char*) GlobalLock(medium.hGlobal);
 
			/// Fetch window co-ordinates from screen co-ordinates, yo..
			Window * hoverWindow = HoverWindow();
			Vector2i windowCoords = hoverWindow->GetWindowCoordsFromScreenCoords(Vector2i(pt.x, pt.y));

			String text = data;
			std::cout<<"\nReceived text: "<<text; 
			// Set the text of target UI element or whatever was there...
			DragAndDropMessage * dadm = new DragAndDropMessage(windowCoords, text);
			MesMan.QueueMessage(dadm);

			// cleanup
			GlobalUnlock(medium.hGlobal);
			break;
		}
	}
	// Do we have to de-allocate it?
	if (medium.pUnkForRelease == 0)
	{
		ReleaseStgMedium(&medium);
	}

	return S_OK;
}


/// Checks if the point has something we can drop data onto.
bool DragAndDrop::Droppable(POINTL pt)
{
	// Cursor co-ordinates...!
	std::cout<<"\nCoords: x"<<pt.x<<" y"<<pt.y;
	return true;
}

void DragAndDrop::SetPdwEffect(POINTL pt, __RPC__inout DWORD *pdwEffect)
{
	if (Droppable(pt))
		*pdwEffect = DROPEFFECT_COPY;
	else
		*pdwEffect = DROPEFFECT_NONE;
}

#endif
