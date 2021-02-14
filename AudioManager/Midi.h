#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>

namespace midi
{

using MidiData = uint32_t;

enum MidiValue
{
    Released = 0,
    Pressed = 127,
    Min = Released,
    Max = Pressed
};

namespace pred
{
    template<MidiData value>
    constexpr bool IsValue(MidiData input) { return value == input; }

    constexpr auto IsReleased = IsValue<0>;
    constexpr auto IsPressed = IsValue<127>;
}

// message types are 8n, 9n, An, Bn, Cn, Dn, En where n is the channel number between 0 and 15
// 8 different message types

class MidiMessage
{
public:

    struct Hasher
    {
        std::size_t operator()(const MidiMessage& key) const;
    };

private:
    MidiData _value;

public:
    MidiMessage(MidiData status, MidiData controller);
    MidiMessage(const std::pair<MidiData, MidiData>& pair);

    friend bool operator==(const MidiMessage& lhs, const MidiMessage& rhs)
    {
        return lhs._value == rhs._value;
    }

    friend std::ostream& operator<<(std::ostream& os, const MidiMessage& mm);
};

std::ostream& operator<<(std::ostream& os, const MidiMessage& mm);

}
