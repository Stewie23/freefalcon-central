#ifndef FREEFALCON_ATLBASE_H_
#define FREEFALCON_ATLBASE_H_

#include <assert.h>
#include <windows.h>
#include <ocidl.h>
#include <oleauto.h>

#ifndef ATLASSERT
#define ATLASSERT(expr) assert(expr)
#endif

struct _ATL_OBJMAP_ENTRY
{
    const void *clsid;
    void *factory;
};

#define BEGIN_OBJECT_MAP(name) _ATL_OBJMAP_ENTRY name[] = {
#define OBJECT_ENTRY(clsid, className) { &(clsid), 0 },
#define END_OBJECT_MAP() { 0, 0 } };

class CComModule
{
public:
    HRESULT Init(_ATL_OBJMAP_ENTRY *, HINSTANCE)
    {
        return S_OK;
    }

    void Term()
    {
    }
};

class CComMultiThreadModel
{
};

template<class ThreadModel>
class CComObjectRootEx
{
public:
    CComObjectRootEx() : refCount_(0)
    {
    }

    STDMETHOD(QueryInterface)(REFIID, void **ppvObject)
    {
        if (ppvObject)
            *ppvObject = 0;

        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)()
    {
        return InterlockedIncrement(&refCount_);
    }

    STDMETHOD_(ULONG, Release)()
    {
        LONG refs = InterlockedDecrement(&refCount_);
        return refs < 0 ? 0 : (ULONG)refs;
    }

private:
    LONG refCount_;
};

#define BEGIN_COM_MAP(className)
#define COM_INTERFACE_ENTRY(interfaceName)
#define END_COM_MAP()

template<class Base>
class CComObject : public Base
{
};

inline HRESULT AtlWaitWithMessageLoop(HANDLE hEvent)
{
    for (;;)
    {
        DWORD result = MsgWaitForMultipleObjects(1, &hEvent, FALSE, INFINITE, QS_ALLINPUT);

        if (result == WAIT_OBJECT_0)
            return S_OK;

        if (result != WAIT_OBJECT_0 + 1)
            return HRESULT_FROM_WIN32(GetLastError());

        MSG msg;
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

inline HRESULT AtlAdvise(IUnknown *pUnkCP, IUnknown *pUnk, const IID &iid, DWORD *pdw)
{
    if (!pUnkCP || !pUnk || !pdw)
        return E_POINTER;

    IConnectionPointContainer *container = 0;
    HRESULT hr = pUnkCP->QueryInterface(IID_IConnectionPointContainer, (void **)&container);
    if (FAILED(hr))
        return hr;

    IConnectionPoint *connectionPoint = 0;
    hr = container->FindConnectionPoint(iid, &connectionPoint);
    container->Release();

    if (FAILED(hr))
        return hr;

    hr = connectionPoint->Advise(pUnk, pdw);
    connectionPoint->Release();
    return hr;
}

inline HRESULT AtlUnadvise(IUnknown *pUnkCP, const IID &iid, DWORD dw)
{
    if (!pUnkCP)
        return E_POINTER;

    IConnectionPointContainer *container = 0;
    HRESULT hr = pUnkCP->QueryInterface(IID_IConnectionPointContainer, (void **)&container);
    if (FAILED(hr))
        return hr;

    IConnectionPoint *connectionPoint = 0;
    hr = container->FindConnectionPoint(iid, &connectionPoint);
    container->Release();

    if (FAILED(hr))
        return hr;

    hr = connectionPoint->Unadvise(dw);
    connectionPoint->Release();
    return hr;
}

class CComBSTR
{
public:
    CComBSTR() : value_(0)
    {
    }

    CComBSTR(const char *value) : value_(0)
    {
        assign(value, value ? lstrlenA(value) : 0);
    }

    CComBSTR(int length, const char *value) : value_(0)
    {
        assign(value, length);
    }

    ~CComBSTR()
    {
        SysFreeString(value_);
    }

    operator BSTR() const
    {
        return value_;
    }

private:
    void assign(const char *value, int length)
    {
        if (!value)
        {
            value_ = SysAllocStringLen(0, 0);
            return;
        }

        int wideLength = MultiByteToWideChar(CP_ACP, 0, value, length, 0, 0);
        value_ = SysAllocStringLen(0, wideLength);
        if (value_)
        {
            MultiByteToWideChar(CP_ACP, 0, value, length, value_, wideLength);
        }
    }

    BSTR value_;
};

template<class T, const IID *piid = 0>
class CComQIPtr
{
public:
    CComQIPtr() : ptr_(0)
    {
    }

    CComQIPtr(T *ptr) : ptr_(ptr)
    {
        if (ptr_) ptr_->AddRef();
    }

    CComQIPtr(const CComQIPtr& other) : ptr_(other.ptr_)
    {
        if (ptr_) ptr_->AddRef();
    }

    ~CComQIPtr()
    {
        if (ptr_) ptr_->Release();
    }

    CComQIPtr& operator=(T *ptr)
    {
        if (ptr) ptr->AddRef();
        if (ptr_) ptr_->Release();
        ptr_ = ptr;
        return *this;
    }

    operator T*() const
    {
        return ptr_;
    }

    T* operator->() const
    {
        return ptr_;
    }

    T** operator&()
    {
        return &ptr_;
    }

private:
    T *ptr_;
};

#endif
