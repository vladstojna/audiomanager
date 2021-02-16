#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>

namespace midi
{

using MidiData = uint8_t;

enum MidiValue
{
    Min = 0,
    Max = 127
};

struct MidiMessage
{
    struct FullHash
    {
        std::size_t operator()(const MidiMessage& key) const;
    };

    struct FullEquality
    {
        bool operator()(const MidiMessage& lhs, const MidiMessage& rhs) const;
    };

    struct PartialHash
    {
        std::size_t operator()(const MidiMessage& key) const;
    };

    struct PartialEquality
    {
        bool operator()(const MidiMessage& lhs, const MidiMessage& rhs) const;
    };

    union
    {
        uint32_t raw;
        struct
        {
            MidiData third;
            MidiData second;
            MidiData first;
        };
    };

    MidiMessage(MidiData first, MidiData second, MidiData third = 0xff);

    bool IsMultiValue() const;
};

std::ostream& operator<<(std::ostream& os, const MidiMessage& mm);

}
