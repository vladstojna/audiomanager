#pragma once

#include "InputAction.h"

#include <cstdint>

namespace action
{
    class KeyboardAction : public InputAction
    {
    public:
        KeyboardAction(const std::string& name, uint8_t vkCode, uint8_t extendedKey = 0);
    };
}
