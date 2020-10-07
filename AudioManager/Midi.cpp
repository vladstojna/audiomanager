#include "Midi.h"

#include <iostream>

// variables related with MIDI protocol to allow for easy
// conversion between status byte and message acronym

const std::string midi::MidiMessage::MESSAGE_TYPES[] = {"NOn", "NOff", "PA", "CC", "PC", "CA", "PBC"};
const std::size_t midi::MidiMessage::OFFSET = 0x8;

midi::MidiMessage::MidiMessage(MidiData status, MidiData controller) :
    _status(status),
    _controller(controller)
{
    // empty
}

midi::MidiMessage::MidiMessage(const std::pair<MidiData, MidiData>& pair) :
    _status(pair.first),
    _controller(pair.second)
{
    // empty
}

// 00 00 00 b1 << 8c -> 00 00 b1 00
// 00 00 b1 00 | 00 00 00 2d -> 00 00 b1 2d

std::size_t midi::MidiMessage::Hasher::operator()(const MidiMessage& key) const
{
    return (key._status << 8) | (key._controller);
}

std::ostream& midi::operator<<(std::ostream& os, const MidiMessage& mm)
{
    MidiData messageType = mm._status >> 4;
    MidiData channel = mm._status & 0xF;
    os << '[' << midi::MidiMessage::MESSAGE_TYPES[messageType - midi::MidiMessage::OFFSET] << ',' << +channel << ',' << +mm._controller << ']';
    return os;
}
