#pragma once

#include "IAction.h"

namespace action
{

    class EmptyAction : public IAction
    {
        // Inherited via IAction
        virtual void Execute(ExecValue value) override;
        virtual void Print(std::ostream& os) const override;
    };

}
