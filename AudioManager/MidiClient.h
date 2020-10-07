#pragma once

#include "AudioManager.h"

#include <combaseapi.h>
#include <mmeapi.h>

#include <mutex>
#include <stdexcept>

namespace midi
{

    struct CallbackData
    {
        manager::AudioManager& manager;
        mutable std::mutex mutex;
        CallbackData(manager::AudioManager& m);
    };

    class ClientException : public std::runtime_error
    {
    public:
        ClientException(const std::string& msg);
        ClientException(const char* msg);
    };

    class Client
    {
        using InCallback = void(__stdcall*)(HMIDIIN, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);

        HMIDIIN _midiDevice;

    public:
        Client(const std::string& deviceName, InCallback callback, CallbackData* data);
        ~Client();

        Client(const Client& other) = delete;
        Client& operator=(const Client& other) = delete;
    };

}
