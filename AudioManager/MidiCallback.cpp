#include "MidiCallback.h"
#include "Midi.h"
#include "Helpers.h"
#include "AudioManager.h"
#include "action/ActionError.h"

#include <iostream>
#include <functional>

#include <fmt/format.h>

midi::CallbackData::CallbackData(manager::AudioManager& m) :
    manager(m),
    mutex()
{
}


const std::function<void(DWORD_PTR, DWORD_PTR)> handleNormal = [](DWORD_PTR dwInstance, DWORD_PTR paramData)
{
    midi::CallbackData* cbData = reinterpret_cast<midi::CallbackData*>(dwInstance);
    std::scoped_lock lock(cbData->mutex);
    midi::MidiData status = static_cast<midi::MidiData>(paramData & 0xff);
    midi::MidiData controller = static_cast<midi::MidiData>((paramData & 0xff00) >> 8);
    midi::MidiData value = static_cast<midi::MidiData>((paramData & 0xff0000) >> 16);
    try
    {
        cbData->manager.ExecuteAction(midi::MidiMessage(status, controller, value));
    }
    catch (const action::ActionError& e)
    {
        std::cerr << fmt::format("{}: {}\n", helpers::Timestamp(), e.what());
    }
};

const std::function<void(DWORD_PTR, DWORD_PTR)> handleInfo = [](DWORD_PTR dwInstance, DWORD_PTR paramData)
{
    std::cout << fmt::format("status={0} (0x{0:x}) second={1} (0x{1:x}) third={2} (0x{2:x})\n",
        paramData & 0xff, (paramData & 0xff00) >> 8, (paramData & 0xff0000) >> 16);
};


void GeneralDataCallback(const std::function<void(DWORD_PTR, DWORD_PTR)>& func, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1)
{
    switch (wMsg)
    {
    case MIM_DATA:
        func(dwInstance, dwParam1);
        break;
    case MIM_OPEN:
        std::cout << "MIDI input device opened\n";
        break;
    case MIM_CLOSE:
        std::cout << "MIDI input device closed\n";
        break;
    case MIM_ERROR:
        std::cout << "Invalid MIDI message received\n";
        break;
    case MIM_LONGERROR:
        std::cout << "Invalid/incomplete MIDI system-exclusive message received\n";
        break;
    case MIM_LONGDATA:
        std::cout << "MIDI system-exclusive message received\n";
        break;
    default:
        std::cout << "Other MIDI message received\n";
    }
}


void __stdcall midi::MidiInCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    GeneralDataCallback(handleNormal, wMsg, dwInstance, dwParam1);
}

void __stdcall midi::MidiInInfoCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    GeneralDataCallback(handleInfo, wMsg, dwInstance, dwParam1);
}
