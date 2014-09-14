/// Emil Hedemalm
/// 2014-08-21
/// Class for handling Drag an drop events.

#ifndef DRAG_AND_DROP_H
#define DRAG_AND_DROP_H

#include "OS/OS.h"
#include "String/AEString.h"

#ifdef WINDOWS

#include <Windows.h>
#include <OleIdl.h>
#include <shellapi.h>

#ifndef _COM_Outptr_
#define _COM_Outptr_ __RPC__deref_out
#endif

class DragAndDrop : public IDropTarget
{
public:
	DragAndDrop();
	// Required interface fucntions
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms682521%28v=vs.85%29.aspx
	virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject);
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms691379%28v=vs.85%29.aspx
    virtual ULONG STDMETHODCALLTYPE AddRef( void);
    virtual ULONG STDMETHODCALLTYPE Release( void);

	// Actual drag n drop stuff below.

	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms680106%28v=vs.85%29.aspx
	virtual HRESULT STDMETHODCALLTYPE DragEnter( 
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect);
        
	virtual HRESULT STDMETHODCALLTYPE DragOver( 
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect);
        
	virtual HRESULT STDMETHODCALLTYPE DragLeave( void);
        
	virtual HRESULT STDMETHODCALLTYPE Drop( 
		/* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
		/* [in] */ DWORD grfKeyState,
		/* [in] */ POINTL pt,
		/* [out][in] */ __RPC__inout DWORD *pdwEffect);
   
private:

	/** Fetches active dropping object. 
		May be called from any of the funtions to further deduce what possible effects a drop could have on this location.
	*/	
	int GetObject(IDataObject * pDataObj);

	/// Checks if the point has something we can drop data onto.
	bool Droppable(POINTL pt);
	/// Updates flag depending on the point and content (which was known since the first call to...
	void SetPdwEffect(POINTL pt, __RPC__inout DWORD *pdwEffect);


	/// o-o
	unsigned int refCount;
};

List<String> GetFilesFromHDrop(HDROP hDrop);


#endif // WINDOWS

#endif // DRAG_AND_DROP_H
