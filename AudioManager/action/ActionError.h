#pragma once

#include <stdexcept>

namespace action
{

    class ActionError : public std::runtime_error
    {
    public:
        ActionError(const std::string& msg);
        ActionError(const char* msg);
        ~ActionError(); // = default
    };

}
