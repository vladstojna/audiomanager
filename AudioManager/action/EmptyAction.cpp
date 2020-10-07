#include "EmptyAction.h"
#include "../Macros.h"

#include <iostream>

void action::EmptyAction::Execute(ExecValue value)
{
    DBG(std::cout << "No action performed\n");
}

void action::EmptyAction::Print(std::ostream& os) const
{
    os << "empty action";
}
