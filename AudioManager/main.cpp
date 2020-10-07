// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "action/ActionError.h"
#include "AudioManager.h"
#include "Helpers.h"
#include "Macros.h"
#include "MidiClient.h"

#include <fmt/format.h>

void Error(const std::string& msg)
{
    std::cerr << msg << std::endl;
    std::cout << "Press ENTER to exit" << std::endl;
    std::cin.get();
}

void HandleDataCallback(const midi::CallbackData* cbData, DWORD_PTR paramData)
{
    std::scoped_lock lock(cbData->mutex);
    midi::MidiData status = paramData & 0xff;
    midi::MidiData controller = (paramData & 0xff00) >> 8;
    midi::MidiData value = (paramData & 0xff0000) >> 16;
    try
    {
        cbData->manager.ExecuteAction(midi::MidiMessage(status, controller), value);
    }
    catch (const action::ActionError& e)
    {
        std::cerr << fmt::format("{}: {}\n", helpers::Timestamp(), e.what());
    }
}

void __stdcall MidiInCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    switch (wMsg)
    {
    case MIM_DATA:
        HandleDataCallback(reinterpret_cast<midi::CallbackData*>(dwInstance), dwParam1);
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

void __stdcall MidiInInfoCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    switch (wMsg)
    {
    case MIM_DATA:
        std::cout << fmt::format("status={0} (0x{0:x}) second={1} (0x{1:x}) third={2} (0x{2:x})\n",
            dwParam1 & 0xff, (dwParam1 & 0xff00) >> 8, (dwParam1 & 0xff0000) >> 16);
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

bool ReadInput(const manager::AudioManager& am)
{
    const char* helpMsg =
        "> e/enum: enumerate audio sessions\n"
        "> s/state: print manager state\n"
        "> r/reload: reload\n"
        "> h/help: print this message\n"
        "> ENTER: exit";
    std::cout << "\nReady\n" << helpMsg << "\n";
    std::string input;
    while (true)
    {
        std::cout << ">>> ";
        std::getline(std::cin, input);
        if (input.empty())
            return false;
        else if (input == "r" || input == "reload")
            return true;
        else if (input == "e" || input == "enum")
            am.EnumerateAudioSessions();
        else if (input == "s" || input == "state")
            std::cout << am << std::endl;
        else if (input == "h" || input == "help")
            std::cout << helpMsg << "\n";
        else
            std::cerr << "unknown command\n";
    }
}

void InfoMode(const std::string& deviceName)
{
    midi::Client client(deviceName, MidiInInfoCallback, nullptr);
    std::cout << "Press ENTER to exit\n";
    std::cin.get();
}

void NormalMode(const std::string& configPath, const std::string& deviceName)
{
    bool readResult = true;
    while (readResult)
    {
        manager::AudioManager manager(configPath);
        midi::CallbackData callbackData(manager);
        midi::Client client(deviceName, MidiInCallback, &callbackData);
        readResult = ReadInput(manager);
    }
}

int main(int argc, char* argv[])
{
    HRESULT result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(result))
    {
        Error(fmt::format("{}: Error initializing COM (error code {})", helpers::Timestamp(), result));
        return 1;
    }
    std::cout << fmt::format("{}: CoInitializeEx\n", helpers::Timestamp());

    const bool infoMode = argc > 1 && (std::strcmp(argv[1], "i") == 0 || std::strcmp(argv[1], "info") == 0);
    const std::string configPath = argc > 1 ? argv[1] : "profile.xml";
    const std::string deviceName = argc > 2 ? argv[2] : "nanoKONTROL2";

    if (infoMode)
        std::cout << fmt::format("{}: Launched in info mode\n", helpers::Timestamp());
    else
        std::cout << fmt::format("{}: Profile = {}\n", helpers::Timestamp(), configPath);
    std::cout << fmt::format("{}: MIDI device = {}\n", helpers::Timestamp(), deviceName);

    try
    {
        infoMode ? InfoMode(deviceName) : NormalMode(configPath, deviceName);
    }
    catch (const std::exception& e)
    {
        CoUninitialize();
        std::cout << fmt::format("{}: CoUninitialize\n", helpers::Timestamp());
        Error(fmt::format("{}: {}", helpers::Timestamp(), e.what()));
        return 1;
    }

    CoUninitialize();
    std::cout << fmt::format("{}: CoUninitialize\n", helpers::Timestamp());
}
