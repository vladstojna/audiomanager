#include "MidiClient.h"
#include "Helpers.h"

#include <iostream>

#include <fmt/format.h>

// helper functions

std::string MidiErrorToString(MMRESULT res) noexcept
{
    char errorMsg[MAXERRORLENGTH];
    MMRESULT convRes = midiInGetErrorTextA(res, errorMsg, MAXERRORLENGTH);
    if (MMSYSERR_NOERROR != convRes)
    {
        return fmt::format("Error getting error message (error code {})", convRes);
    }
    return std::string(errorMsg);
}

std::vector<std::string> GetMidiDeviceNames(UINT midiDeviceCount)
{
    MIDIINCAPS caps;
    std::vector<std::string> devices;
    devices.reserve(midiDeviceCount);
    for (UINT i = 0; i < midiDeviceCount; ++i)
    {
        MMRESULT res = midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));
        if (MMSYSERR_NOERROR != res)
        {
            throw midi::ClientException(
                fmt::format("Error getting MIDI device capabilities: {} (error code {})",
                    MidiErrorToString(res), res));
        }
        devices.push_back(helpers::Utf16ToUtf8(caps.szPname));
        std::cout << i << " : name = " << devices[i] << "\n";
    }
    return devices;
}


midi::CallbackData::CallbackData(manager::AudioManager& m) :
    manager(m),
    mutex()
{
}


midi::ClientException::ClientException(const std::string& msg) :
    std::runtime_error(msg)
{
}

midi::ClientException::ClientException(const char* msg) :
    std::runtime_error(msg)
{
}


midi::Client::Client(const std::string& deviceName, InCallback callback, CallbackData* data) :
    _midiDevice(nullptr)
{
    unsigned int midiDeviceNum = midiInGetNumDevs();
    if (midiDeviceNum == 0)
        throw ClientException("No MIDI devices found");

    std::cout << "Found " << midiDeviceNum << " MIDI devices\n";

    std::vector<std::string> devices = GetMidiDeviceNames(midiDeviceNum);

    DWORD midiPort = 0;
    for (size_t i = 0; i < devices.size(); i++)
    {
        if (devices[i].find(deviceName) != std::string::npos)
        {
            midiPort = i;
            break;
        }
    }

    MMRESULT res = midiInOpen(&_midiDevice, midiPort, reinterpret_cast<DWORD_PTR>(callback), reinterpret_cast<DWORD_PTR>(data), CALLBACK_FUNCTION);
    if (res != MMSYSERR_NOERROR)
        throw ClientException(fmt::format("midiInOpen: {}", MidiErrorToString(res)));

    std::cout << "Opened input port for \"" << devices[midiPort] << "\"\n";

    res = midiInStart(_midiDevice);
    if (res != MMSYSERR_NOERROR)
    {
        res = midiInClose(_midiDevice);
        if (res != MMSYSERR_NOERROR)
            std::cerr << fmt::format("midiInClose: {}", MidiErrorToString(res)) << std::endl;
        throw ClientException(fmt::format("midiInStart: {}", MidiErrorToString(res)));
    }
}

midi::Client::~Client()
{
    MMRESULT res = midiInStop(_midiDevice);
    if (res != MMSYSERR_NOERROR)
        std::cerr << fmt::format("midiInStop: {}", MidiErrorToString(res)) << std::endl;

    res = midiInClose(_midiDevice);
    if (res != MMSYSERR_NOERROR)
        std::cerr << fmt::format("midiInClose: {}", MidiErrorToString(res)) << std::endl;
}
