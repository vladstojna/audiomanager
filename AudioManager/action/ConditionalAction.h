#pragma once

#include "IAction.h"

#include <functional>

namespace action
{

    template<typename T>
    class ConditionalAction : public T
    {
        static_assert(std::is_convertible<T*, IAction*>::value, "T must publicly inherit from IAction");

    private:
        std::function<bool(ExecValue)> _condition;

    public:
        template<typename... Args>
        ConditionalAction(const std::function<bool(ExecValue)>& condition, Args&&... args) :
            T(std::forward<Args>(args)...),
            _condition(condition)
        {
        }

        virtual void Execute(ExecValue value) override
        {
            if (_condition(value))
            {
                T::Execute(value);
            }
        }
    };

}
