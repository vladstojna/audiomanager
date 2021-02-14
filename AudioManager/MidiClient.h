#pragma once

#include <combaseapi.h>
#include <mmeapi.h>

#include <stdexcept>

namespace midi
{

    struct CallbackData;

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
