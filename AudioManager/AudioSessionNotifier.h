#pragma once

#include <functional>

#include <audiopolicy.h>

namespace event
{

class AudioSessionNotifier : public IAudioSessionNotification
{

private:
    LONG _refCtr;
    std::function<void(IAudioSessionControl2*)> _callback;

    ~AudioSessionNotifier() = default;

public:
    AudioSessionNotifier(const std::function<void(IAudioSessionControl2*)>& callback);
    AudioSessionNotifier(std::function<void(IAudioSessionControl2*)>&& callback);

    // Inherited via IAudioSessionNotification
    virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    virtual ULONG __stdcall AddRef(void) override;
    virtual ULONG __stdcall Release(void) override;
    virtual HRESULT __stdcall OnSessionCreated(IAudioSessionControl* NewSession) override;
};

}
