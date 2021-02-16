#pragma once

#include <cstdint>
#include <iosfwd>

namespace action
{

    using ExecValue = uint8_t;

    class IAction
    {
    public:
        virtual ~IAction() = default;
        virtual void Execute(ExecValue value) = 0;

        virtual bool Expired() const
        {
            return false;
        }
    
        friend std::ostream& operator<<(std::ostream& os, const IAction& a)
        {
            a.Print(os);
            return os;
        }
    
    protected:
        virtual void Print(std::ostream& os) const = 0;
    };

}
