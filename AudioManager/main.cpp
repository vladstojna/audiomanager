// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "action/ActionError.h"
#include "AudioManager.h"
#include "Helpers.h"
#include "MidiClient.h"
#include "MidiCallback.h"

#include <fmt/format.h>

enum class ReadResult
{
    reload,
    info,
    exit
};

void Error(const std::string& msg)
{
    std::cerr << msg << std::endl;
    std::cout << "Press ENTER to exit" << std::endl;
    std::cin.get();
}

ReadResult ReadInput(const manager::AudioManager& am)
{
    const char* helpMsg =
        "> e/enum: enumerate audio sessions\n"
        "> s/state: print manager state\n"
        "> r/reload: reload\n"
        "> i/info: reload in info mode\n"
        "> h/help: print this message\n"
        "> ENTER: exit";
    std::cout << "\nReady\n" << helpMsg << "\n";
    std::string input;
    while (true)
    {
        std::cout << ">>> ";
        std::getline(std::cin, input);
        if (input.empty())
            return ReadResult::exit;
        else if (input == "r" || input == "reload")
            return ReadResult::reload;
        else if (input == "i" || input == "info")
            return ReadResult::info;
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
    std::cout << "Running in info mode\n";
    std::cout << "Press ENTER to exit back to normal mode\n";
    std::cin.get();
}

ReadResult NormalMode(const std::string& configPath, const std::string& deviceName)
{
    manager::AudioManager manager(configPath);
    midi::CallbackData callbackData(manager);
    midi::Client client(deviceName, midi::MidiInCallback, &callbackData);
    return ReadInput(manager);
}

void Start(const std::string& configPath, const std::string& deviceName)
{
    ReadResult readResult;
    while ((readResult = NormalMode(configPath, deviceName)) != ReadResult::exit)
        if (readResult == ReadResult::info)
            InfoMode(deviceName);
}

int main(int argc, char* argv[])
{
    HRESULT result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(result))
    {
        Error(fmt::format("{}: Error initializing COM (error code {})", helpers::Timestamp(), result));
        return result;
    }
    std::cout << fmt::format("{}: CoInitializeEx\n", helpers::Timestamp());

    const std::string configPath = argc > 1 ? argv[1] : "profile.xml";
    const std::string deviceName = argc > 2 ? argv[2] : "nanoKONTROL2";
    std::cout << fmt::format("{}: Profile = {}\n", helpers::Timestamp(), configPath);
    std::cout << fmt::format("{}: MIDI device = {}\n", helpers::Timestamp(), deviceName);

    result = 0;
    try
    {
        Start(configPath, deviceName);
    }
    catch (const std::exception& e)
    {
        Error(fmt::format("{}: {}", helpers::Timestamp(), e.what()));
        result = 1;
    }

    CoUninitialize();
    std::cout << fmt::format("{}: CoUninitialize\n", helpers::Timestamp());
    return result;
}
