// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "action/ActionError.h"
#include "AudioManager.h"
#include "Helpers.h"
#include "Macros.h"
#include "MidiClient.h"
#include "MidiCallback.h"

#include <fmt/format.h>

void Error(const std::string& msg)
{
    std::cerr << msg << std::endl;
    std::cout << "Press ENTER to exit" << std::endl;
    std::cin.get();
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
    midi::Client client(deviceName, midi::MidiInInfoCallback, nullptr);
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
        midi::Client client(deviceName, midi::MidiInCallback, &callbackData);
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
