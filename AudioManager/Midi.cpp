#include "Midi.h"

#include <iostream>
#include <iomanip>

// 00 00 00 b1 << 8c -> 00 00 b1 00
// 00 00 b1 00 | 00 00 00 2d -> 00 00 b1 2d

midi::MidiMessage::MidiMessage(MidiData status, MidiData controller) :
    _value((status << 8) | controller)
{
    // empty
}

midi::MidiMessage::MidiMessage(const std::pair<MidiData, MidiData>& pair) :
    _value((pair.first << 8) | pair.second)
{
    // empty
}

std::size_t midi::MidiMessage::Hasher::operator()(const MidiMessage& key) const
{
    return key._value;
}

std::ostream& midi::operator<<(std::ostream& os, const MidiMessage& mm)
{
    MidiData statusByte = mm._value >> 8;
    MidiData secondByte = mm._value & 0xff;
    return os << std::setw(3) << statusByte << "," << std::left << std::setw(3) << secondByte;
}
