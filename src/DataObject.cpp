// Emil Hedemalm
// 2013-07-03

#include "DataObject.h"

#ifdef WINDOWS

DataObject::DataObject() : IDataObject (){
	data = NULL;
};

HRESULT STDMETHODCALLTYPE DataObject::QueryInterface(const IID &, void **){
	return E_NOTIMPL;
};
ULONG	STDMETHODCALLTYPE DataObject::AddRef(void){
	return E_NOTIMPL;
};
ULONG	STDMETHODCALLTYPE DataObject::Release(void) {
	return E_NOTIMPL;
};
HRESULT STDMETHODCALLTYPE DataObject::GetData(FORMATETC *,STGMEDIUM *) {
	return E_NOTIMPL;
};
HRESULT STDMETHODCALLTYPE DataObject::GetDataHere(FORMATETC *,STGMEDIUM *) {
	return E_NOTIMPL;
};
HRESULT STDMETHODCALLTYPE DataObject::QueryGetData(FORMATETC *) {
	return E_NOTIMPL;
};
HRESULT STDMETHODCALLTYPE DataObject::GetCanonicalFormatEtc(FORMATETC *,FORMATETC *) {
	return E_NOTIMPL;
};
HRESULT STDMETHODCALLTYPE DataObject::SetData(FORMATETC *,STGMEDIUM *,BOOL) {
	return E_NOTIMPL;
};
HRESULT STDMETHODCALLTYPE DataObject::EnumFormatEtc(DWORD,IEnumFORMATETC **) {
	return E_NOTIMPL;
};
HRESULT STDMETHODCALLTYPE DataObject::DAdvise(FORMATETC *, DWORD, IAdviseSink *, DWORD *) {
	return E_NOTIMPL;
};
HRESULT STDMETHODCALLTYPE DataObject::DUnadvise(DWORD) {
	return E_NOTIMPL;
};
HRESULT STDMETHODCALLTYPE DataObject::EnumDAdvise(IEnumSTATDATA **) {
	return E_NOTIMPL;
};


#endif

