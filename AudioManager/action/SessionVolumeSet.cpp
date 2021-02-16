#include "SessionVolumeSet.h"
#include "ActionError.h"
#include "../Macros.h"
#include "../Midi.h"

#include <Audioclient.h>

#include <iostream>

#include <fmt/format.h>

action::SessionVolumeSet::SessionVolumeSet(const std::string& application, IAudioSessionControl2* session) :
    SessionVolumeControl(application, session)
{
}

void action::SessionVolumeSet::Execute(ExecValue value)
{
    float volume = static_cast<float>(value) / midi::MidiValue::Max;
    HRESULT res = GetSimpleAudioVolume()->SetMasterVolume(volume, nullptr);
    if (FAILED(res))
        throw ActionError(fmt::format("Failed to set session volume (error code {})", res));
    DBG(std::cout << GetApplication() << " volume -> " << static_cast<uint32_t>(100 * volume + 0.5) << "%\n");
}

void action::SessionVolumeSet::Print(std::ostream& os) const
{
    os << "action=VolumeSet, ";
    SessionVolumeControl::Print(os);
}
