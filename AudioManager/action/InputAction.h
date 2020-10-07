#pragma once

#include "IAction.h"

#include <vector>
#include <functional>

#include <Windows.h>

namespace action
{

    class InputAction : public IAction
    {
    private:
        std::string _name;
        std::vector<INPUT> _inputs;

    public:
        template<typename... T>
        InputAction(const std::string& name, T&&... args) :
            _name(name),
            _inputs()
        {
            _inputs.reserve(sizeof...(T));
            (_inputs.emplace_back(std::forward<T>(args)), ...);
        }

        // Inherited via IAction
        virtual void Execute(ExecValue value) override;

    protected:
        virtual void Print(std::ostream& os) const override;
    };

}
