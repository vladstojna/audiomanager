#include "SessionVolumeMute.h"
#include "ActionError.h"
#include "../Macros.h"

#include <Audioclient.h>

#include <iostream>

#include <fmt/format.h>

action::SessionVolumeMute::SessionVolumeMute(const std::string& application, IAudioSessionControl2* session) :
    SessionVolumeControl(application, session)
{
}

void action::SessionVolumeMute::Execute(ExecValue value)
{
    BOOL muted;
    HRESULT res = GetSimpleAudioVolume()->GetMute(&muted);
    if (FAILED(res))
        throw ActionError(fmt::format("Failed to get session mute state (error code {})", res));
    res = GetSimpleAudioVolume()->SetMute(!muted, nullptr);
    if (FAILED(res))
        throw ActionError(fmt::format("Failed to set session mute state (error code {})", res));
    DBG(std::cout << GetApplication() << " -> " << (muted ? "unmuted\n" : "muted\n"));
}

void action::SessionVolumeMute::Print(std::ostream& os) const
{
    os << "action=VolumeMute, ";
    SessionVolumeControl::Print(os);
}
