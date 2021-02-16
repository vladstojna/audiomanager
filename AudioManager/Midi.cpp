#include "Midi.h"

#include <iostream>

midi::MidiMessage::MidiMessage(MidiData f, MidiData s, MidiData t) :
    first(f),
    second(s),
    third(t)
{
    // zero the remaining bytes
    raw = raw & 0xffffff;
}

bool midi::MidiMessage::IsMultiValue() const
{
    return third == 0xff;
}

// Hash and Equality specialisations

std::size_t midi::MidiMessage::FullHash::operator()(const MidiMessage& key) const
{
    return key.raw;
}

std::size_t midi::MidiMessage::PartialHash::operator()(const MidiMessage& key) const
{
    // only the first two bytes of a message are relevant
    return key.raw >> 8;
}

bool midi::MidiMessage::FullEquality::operator()(const MidiMessage& lhs, const MidiMessage& rhs) const
{
    return lhs.raw == rhs.raw;
}

bool midi::MidiMessage::PartialEquality::operator()(const MidiMessage& lhs, const MidiMessage& rhs) const
{
    // only the first two bytes of a message are relevant
    return (lhs.raw >> 8) == (rhs.raw >> 8);
}

std::ostream& midi::operator<<(std::ostream& os, const MidiMessage& mm)
{
    os.fill('0');
    os.width(3);
    os << static_cast<int>(mm.first) << " ";
    os.fill('0');
    os.width(3);
    os << static_cast<int>(mm.second) << " ";
    os.fill('0');
    os.width(3);
    if (mm.IsMultiValue())
        os << " - ";
    else
        os << static_cast<int>(mm.third);
    return os;
}
