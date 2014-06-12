// Emil Hedemalm
// 2013-07-03

#ifndef DATAOBJECT_H
#define DATAOBJECT_H

/// Copy-paste objects o-o;

#include "OS/OS.h"

#ifdef WINDOWS

/*
#include <objidl.h>

/// A Win32 Class for carrying data related to the application.
class DataObject : public IDataObject {
public:
	DataObject();
	HRESULT STDMETHODCALLTYPE QueryInterface(const IID &, void **);
	ULONG	STDMETHODCALLTYPE AddRef(void);
	ULONG	STDMETHODCALLTYPE Release(void);
	HRESULT STDMETHODCALLTYPE GetData(FORMATETC *,STGMEDIUM *);
	HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC *,STGMEDIUM *);
	HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC *);
	HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC *,FORMATETC *);
	HRESULT STDMETHODCALLTYPE SetData(FORMATETC *,STGMEDIUM *,BOOL);
	HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD,IEnumFORMATETC **);
	HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC *,DWORD,IAdviseSink *,DWORD *);
	HRESULT STDMETHODCALLTYPE DUnadvise(DWORD);
	HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA **);
private:
	void * data;
};



class DropTarget : public IDropTarget{
public:
	DropTarget();

};
*/

#endif // WINDOWS

#endif
