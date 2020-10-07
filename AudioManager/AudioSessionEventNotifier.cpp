#include "AudioSessionEventNotifier.h"
#include "Helpers.h"
#include <iostream>

event::AudioSessionEventNotifier::AudioSessionEventNotifier(const std::string& applicationName) :
    _refCtr(1),
    _application(applicationName)
{
    // empty
}

event::AudioSessionEventNotifier::AudioSessionEventNotifier(std::string&& applicationName) :
    _refCtr(1),
    _application(std::move(applicationName))
{
    // empty
}

HRESULT __stdcall event::AudioSessionEventNotifier::QueryInterface(REFIID riid, void** ppvInterface)
{
    if (IID_IUnknown == riid)
    {
        AddRef();
        *ppvInterface = (IUnknown*)this;
    }
    else if (__uuidof(IAudioSessionEvents) == riid)
    {
        AddRef();
        *ppvInterface = (IAudioSessionEvents*)this;
    }
    else
    {
        *ppvInterface = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}

ULONG __stdcall event::AudioSessionEventNotifier::AddRef(void)
{
    return InterlockedIncrement(&_refCtr);
}

ULONG __stdcall event::AudioSessionEventNotifier::Release(void)
{
    ULONG ulRef = InterlockedDecrement(&_refCtr);
    if (0 == ulRef)
    {
        delete this;
    }
    return ulRef;
}

HRESULT __stdcall event::AudioSessionEventNotifier::OnDisplayNameChanged(LPCWSTR NewDisplayName, LPCGUID EventContext)
{
    return S_OK;
}

HRESULT __stdcall event::AudioSessionEventNotifier::OnIconPathChanged(LPCWSTR NewIconPath, LPCGUID EventContext)
{
    return S_OK;
}

HRESULT __stdcall event::AudioSessionEventNotifier::OnSimpleVolumeChanged(float NewVolume, BOOL NewMute, LPCGUID EventContext)
{
    return S_OK;
}

HRESULT __stdcall event::AudioSessionEventNotifier::OnChannelVolumeChanged(DWORD ChannelCount, float NewChannelVolumeArray[], DWORD ChangedChannel, LPCGUID EventContext)
{
    return S_OK;
}

HRESULT __stdcall event::AudioSessionEventNotifier::OnGroupingParamChanged(LPCGUID NewGroupingParam, LPCGUID EventContext)
{
    return S_OK;
}

HRESULT __stdcall event::AudioSessionEventNotifier::OnStateChanged(AudioSessionState NewState)
{
    printf("Audio session state of '%s' -> %s\n", _application.c_str(), helpers::SessionStateToText(NewState).c_str());
    return S_OK;
}

HRESULT __stdcall event::AudioSessionEventNotifier::OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason)
{
    return S_OK;
}
