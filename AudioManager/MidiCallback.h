#pragma once

#include <mutex>

#include <combaseapi.h>
#include <mmeapi.h>

namespace manager
{
    class AudioManager;
}

namespace midi
{

    struct CallbackData
    {
        manager::AudioManager& manager;
        mutable std::mutex mutex;
        CallbackData(manager::AudioManager& m);
    };

    void __stdcall MidiInCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

    void __stdcall MidiInInfoCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

}
