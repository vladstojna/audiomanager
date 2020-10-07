#include "KeyboardAction.h"

INPUT GetKeyboardInput(uint8_t vkCode, uint8_t extended, uint8_t keyup = 0)
{
    INPUT i;
    i.type = INPUT_KEYBOARD;
    i.ki = { vkCode, 0, static_cast<uint8_t>(extended | keyup), 0, NULL };
    return i;
}

action::KeyboardAction::KeyboardAction(const std::string& name, uint8_t vkCode, uint8_t extendedKey) :
    InputAction(name,
        GetKeyboardInput(vkCode, extendedKey),
        GetKeyboardInput(vkCode, extendedKey, KEYEVENTF_KEYUP))
{
}
