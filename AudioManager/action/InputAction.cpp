#include "InputAction.h"
#include "ActionError.h"
#include "../Macros.h"

#include <iostream>

#include <fmt/format.h>

void action::InputAction::Execute(ExecValue value)
{
    size_t size = _inputs.size();
    unsigned int result = SendInput(size, _inputs.data(), sizeof(INPUT));
    if (result != size)
    {
        throw action::ActionError(
            fmt::format("Only {} out of {} inputs entered the input stream (error code {})",
                result, size, GetLastError()));
    }
    DBG(std::cout << "executed " << _name << "\n");
}

void action::InputAction::Print(std::ostream& os) const
{
    os << "action=" << _name;
}
