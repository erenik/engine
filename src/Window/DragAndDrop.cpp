/// Emil Hedemalm
/// 2014-08-21
/// Class for handling Drag an drop events.

#include "OS/OSUtil.h"

#ifdef WINDOWS

#include "DragAndDrop.h"
#include <iostream>
#include <cassert>
#include "String/AEString.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"
#include "Message/FileEvent.h"

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

//	GetObject(pDataObj);

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
		uiMutex.Claim();
		InputMan.MouseMove(HoverWindow(), windowCoords);
		uiMutex.Release();
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
	
	return GetObject(pDataObj);
}

/** Fetches active dropping object. 
	May be called from any of the funtions to further deduce what possible effects a drop could have on this location.
*/
int DragAndDrop::GetObject(IDataObject * pDataObj)
{
	/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms678431%28v=vs.85%29.aspx
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms682177%28v=vs.85%29.aspx
	FORMATETC format;
	format.cfFormat = CF_TEXT;
	format.ptd = NULL;
	format.dwAspect = DVASPECT::DVASPECT_CONTENT;
	format.lindex = -1;
	format.tymed = TYMED::TYMED_HGLOBAL | TYMED::TYMED_FILE | TYMED::TYMED_ISTREAM |
		TYMED::TYMED_ISTORAGE | TYMED::TYMED_GDI | TYMED::TYMED_MFPICT |
		TYMED::TYMED_ENHMF; 
	
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms691227%28v=vs.85%29.aspx

	STGMEDIUM medium;

	int result = S_OK;
	for (int i = 1; i < CF_MAX; ++i)
	{
		format.cfFormat = i;
		result = pDataObj->GetData(&format, &medium);
		/// Break when a format worked out.
		if (result == S_OK)
			break;
	}
	std::cout<<"\nFORMAT: "<<format.cfFormat;
	/*
	format.tymed = TYMED::TYMED_HGLOBAL | TYMED::TYMED_FILE | TYMED::TYMED_ISTREAM |
		TYMED::TYMED_ISTORAGE | TYMED::TYMED_GDI | TYMED::TYMED_MFPICT |
		TYMED::TYMED_ENHMF; 
	 CF_ENHMETAFILE
	 */
	switch(result)
	{
		case DV_E_LINDEX:
			std::cout<<"\nThe value for lindex is not valid; currently, only -1 is supported.";
			return S_FALSE;
		case DV_E_FORMATETC:
			std::cout<<"\nThe value for pformatetcIn is not valid.";
			return S_FALSE;
		case DV_E_TYMED:
			std::cout<<"";
			return S_FALSE;
		case DV_E_DVASPECT:
			std::cout<<"\nASPECT errorr";
			return S_FALSE;
		default:
			std::cout<<"Other error";
			return S_FALSE;
		case S_OK:
			break;
	}
	if (result != S_OK)
	{
		return S_FALSE;
	}

	// Fetch the data
	switch(medium.tymed)
	{
		case TYMED::TYMED_HGLOBAL:
		{
			std::cout<<"\nGet the stuffs D:";
			
			/// Fetch window co-ordinates from screen co-ordinates, yo..
			Window * hoverWindow = HoverWindow();

			switch(format.cfFormat)
			{
				case CF_TEXT:
				{
					// ask the IDataObject for some CF_TEXT data, stored as a HGLOBAL
					// We need to lock the HGLOBAL handle because we can't
					// be sure if this is GMEM_FIXED (i.e. normal heap) data or not
					char * data = (char*) GlobalLock(medium.hGlobal);
 
		//			Vector2i windowCoords = hoverWindow->GetWindowCoordsFromScreenCoords(Vector2i(pt[0], pt[1]));
					String text = data;
					std::cout<<"\nReceived text: "<<text; 
					// Set the text of target UI element or whatever was there...
					//DragAndDropMessage * dadm = new DragAndDropMessage(windowCoords, text);
				//	MesMan.QueueMessage(dadm);
					break;
				}
				case CF_HDROP:
				{
					// ask the IDataObject for some CF_TEXT data, stored as a HGLOBAL
					// We need to lock the HGLOBAL handle because we can't
					// be sure if this is GMEM_FIXED (i.e. normal heap) data or not
					HDROP hDrop = (HDROP) GlobalLock(medium.hGlobal);
 
					List<String> files = GetFilesFromHDrop(hDrop);

					/// Send a message about it.
					FileEvent * fe = new FileEvent();
					fe->files = files;
					fe->msg = "PasteFiles";
					MesMan.QueueMessage(fe);
					break;
				}
			}

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
