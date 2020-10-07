#include "SessionVolumeControl.h"
#include "ActionError.h"
#include "../AudioSessionEventNotifier.h"
#include "../Helpers.h"

#include <iostream>
#include <stdexcept>

#include <fmt/format.h>

action::SessionVolumeControl::SessionVolumeControl(const std::string& application, IAudioSessionControl2* session) :
    _application(application),
    _session(session),
    _audioVolume(nullptr),
#if defined(NDEBUG)
    _sessionEvents(nullptr)
#else
    _sessionEvents(new event::AudioSessionEventNotifier(application))
#endif
{
    if (_session == nullptr)
    {
        throw std::invalid_argument("audio session is nullptr");
    }

    _session->AddRef();

    HRESULT res = _session->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&_audioVolume);
    if (FAILED(res))
        goto cleanup;

#if !defined(NDEBUG)
    res = _session->RegisterAudioSessionNotification(_sessionEvents);
    if (FAILED(res))
        goto cleanup;
#endif

cleanup:
    if (FAILED(res))
    {
    #if !defined(NDEBUG)
        helpers::SafeRelease(&_sessionEvents);
    #endif
        helpers::SafeRelease(&_audioVolume);
        helpers::SafeRelease(&_session);
        throw std::runtime_error("Failed to initialize session volume controls");
    }
}

action::SessionVolumeControl::~SessionVolumeControl()
{
#if !defined(NDEBUG)
    if (_session != nullptr && _sessionEvents != nullptr)
    {
        _session->UnregisterAudioSessionNotification(_sessionEvents);
    }
#endif
    helpers::SafeRelease(&_sessionEvents);
    helpers::SafeRelease(&_audioVolume);
    helpers::SafeRelease(&_session);
}

bool action::SessionVolumeControl::Expired() const
{
    AudioSessionState state;
    HRESULT res = GetAudioSessionControl()->GetState(&state);
    if (FAILED(res))
        throw ActionError(fmt::format("Unable to obtain audio session state (error code {})", res));
    return state == AudioSessionState::AudioSessionStateExpired;
}

void action::SessionVolumeControl::Print(std::ostream& os) const
{
    DWORD pid;
    BOOL mute;
    FLOAT masterVol;
    AudioSessionState state;

    HRESULT res = GetAudioSessionControl()->GetProcessId(&pid);
    if (FAILED(res))
        throw ActionError(fmt::format("Failed to get process id (error code {})", res));
    res = GetAudioSessionControl()->GetState(&state);
    if (FAILED(res))
        throw ActionError(fmt::format("Failed to get session state (error code {})", res));
    res = GetSimpleAudioVolume()->GetMasterVolume(&masterVol);
    if (FAILED(res))
        throw ActionError(fmt::format("Failed to get session volume (error code {})", res));
    res = GetSimpleAudioVolume()->GetMute(&mute);
    if (FAILED(res))
        throw ActionError(fmt::format("Failed to get session mute state (error code {})", res));

    os << "app=" << GetApplication() << ", pid=" << pid << ", state=" << helpers::SessionStateToText(state) << ", volume=" << masterVol << ", mute=" << mute;
}
