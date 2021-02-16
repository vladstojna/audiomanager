#include "GlobalVolumeSet.h"
#include "ActionError.h"
#include "../Helpers.h"
#include "../Macros.h"
#include "../Midi.h"

#include <iostream>

#include <endpointvolume.h>
#include <mmdeviceapi.h>

#include <fmt/format.h>

action::GlobalVolumeSet::GlobalVolumeSet() :
    _endpointVolume(nullptr)
{
    IMMDeviceEnumerator* deviceEnumerator = nullptr;
    IMMDevice* defaultDevice = nullptr;

    HRESULT result = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator);
    if (FAILED(result))
        goto cleanup;

    result = deviceEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eConsole, &defaultDevice);
    if (FAILED(result))
        goto cleanup;

    result = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, nullptr, (void**)&_endpointVolume);
    if (FAILED(result))
        goto cleanup;

cleanup:
    if (FAILED(result))
    {
        helpers::SafeRelease(&deviceEnumerator);
        helpers::SafeRelease(&defaultDevice);
        throw std::runtime_error("Failed to initialize audio endpoint volume");
    }
}

action::GlobalVolumeSet::~GlobalVolumeSet()
{
    helpers::SafeRelease(&_endpointVolume);
}

void action::GlobalVolumeSet::Execute(ExecValue value)
{
    float volume = static_cast<float>(value) / midi::MidiValue::Max;
    HRESULT res = _endpointVolume->SetMasterVolumeLevelScalar(volume, nullptr);
    if (FAILED(res))
        throw ActionError(fmt::format("Failed to set master volume (error code {})", res));
    DBG(std::cout << "global volume -> " << static_cast<uint32_t>(100 * volume + 0.5) << "%\n");
}

void action::GlobalVolumeSet::Print(std::ostream& os) const
{
    float volume;
    BOOL mute;
    HRESULT res = _endpointVolume->GetMasterVolumeLevelScalar(&volume);
    if (FAILED(res))
        throw ActionError(fmt::format("Failed to get master volume (error code {})", res));
    res = _endpointVolume->GetMute(&mute);
    if (FAILED(res))
        throw ActionError(fmt::format("Failed to get master mute state (error code {})", res));
    os << "action=VolumeSet, app=global, volume=" << volume << ", mute=" << mute;
}
