#pragma once

#include <string>

#include <audiopolicy.h>

namespace event
{

class AudioSessionEventNotifier : public IAudioSessionEvents
{
private:
    LONG _refCtr;
    std::string _application;

    ~AudioSessionEventNotifier() = default;

public:
    AudioSessionEventNotifier(const std::string& applicationName);
    AudioSessionEventNotifier(std::string&& applicationName);

    // Inherited via IAudioSessionEvents
    virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    virtual ULONG __stdcall AddRef(void) override;
    virtual ULONG __stdcall Release(void) override;
    virtual HRESULT __stdcall OnDisplayNameChanged(LPCWSTR NewDisplayName, LPCGUID EventContext) override;
    virtual HRESULT __stdcall OnIconPathChanged(LPCWSTR NewIconPath, LPCGUID EventContext) override;
    virtual HRESULT __stdcall OnSimpleVolumeChanged(float NewVolume, BOOL NewMute, LPCGUID EventContext) override;
    virtual HRESULT __stdcall OnChannelVolumeChanged(DWORD ChannelCount, float NewChannelVolumeArray[], DWORD ChangedChannel, LPCGUID EventContext) override;
    virtual HRESULT __stdcall OnGroupingParamChanged(LPCGUID NewGroupingParam, LPCGUID EventContext) override;
    virtual HRESULT __stdcall OnStateChanged(AudioSessionState NewState) override;
    virtual HRESULT __stdcall OnSessionDisconnected(AudioSessionDisconnectReason DisconnectReason) override;
};

}
