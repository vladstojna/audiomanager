#include "ActionError.h"

action::ActionError::ActionError(const std::string& msg) :
    std::runtime_error(msg)
{
}

action::ActionError::ActionError(const char* msg) :
    std::runtime_error(msg)
{
}

action::ActionError::~ActionError() = default;
