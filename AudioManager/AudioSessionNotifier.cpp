#include "AudioSessionNotifier.h"
#include "Helpers.h"

event::AudioSessionNotifier::AudioSessionNotifier(const std::function<void(IAudioSessionControl2*)>& callback) :
    _refCtr(1),
    _callback(callback)
{
    // empty
}

event::AudioSessionNotifier::AudioSessionNotifier(std::function<void(IAudioSessionControl2*)>&& callback) :
    _refCtr(1),
    _callback(std::move(callback))
{
    // empty
}

HRESULT __stdcall event::AudioSessionNotifier::QueryInterface(REFIID riid, void** ppvInterface)
{
    if (IID_IUnknown == riid)
    {
        AddRef();
        *ppvInterface = (IUnknown*)this;
    }
    else if (__uuidof(IAudioSessionNotification) == riid)
    {
        AddRef();
        *ppvInterface = (IAudioSessionNotification*)this;
    }
    else
    {
        *ppvInterface = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}

ULONG __stdcall event::AudioSessionNotifier::AddRef(void)
{
    return InterlockedIncrement(&_refCtr);
}

ULONG __stdcall event::AudioSessionNotifier::Release(void)
{
    ULONG ulRef = InterlockedDecrement(&_refCtr);
    if (0 == ulRef)
    {
        delete this;
    }
    return ulRef;
}

HRESULT __stdcall event::AudioSessionNotifier::OnSessionCreated(IAudioSessionControl* NewSession)
{
    if (NewSession)
    {
        IAudioSessionControl2* control2;
        HRESULT result = NewSession->QueryInterface(__uuidof(IAudioSessionControl2), (LPVOID*)&control2);
        if (SUCCEEDED(result))
            _callback(control2);
        helpers::SafeRelease(&control2);
        return result;
    }
    return E_POINTER;
}
