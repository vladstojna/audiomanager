#pragma once

#include <cstdint>
#include <iosfwd>

#include "../Midi.h"

namespace action
{

    using ExecValue = midi::MidiData;
    constexpr ExecValue MaxValue = midi::MidiValue::Max;

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
