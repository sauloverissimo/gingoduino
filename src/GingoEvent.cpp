// Gingoduino — Music Theory Library for Embedded Systems
// Implementation of GingoEvent.
//
// SPDX-License-Identifier: MIT

#include "GingoEvent.h"

#if GINGODUINO_HAS_EVENT

namespace gingoduino {

GingoEvent::GingoEvent()
    : type_(EVENT_REST), octave_(4), velocity_(100), midiChannel_(0)
{}

GingoEvent GingoEvent::noteEvent(const GingoNote& note,
                                 const GingoDuration& duration,
                                 uint8_t octave,
                                 uint8_t velocity,
                                 uint8_t midiChannel) {
    GingoEvent e;
    e.type_ = EVENT_NOTE;
    e.note_ = note;
    e.duration_ = duration;
    e.octave_ = octave;
    e.velocity_ = velocity & 0x7F;
    e.midiChannel_ = midiChannel & 0x0F;
    return e;
}

GingoEvent GingoEvent::chordEvent(const GingoChord& chord,
                                  const GingoDuration& duration,
                                  uint8_t octave,
                                  uint8_t velocity,
                                  uint8_t midiChannel) {
    GingoEvent e;
    e.type_ = EVENT_CHORD;
    e.chord_ = chord;
    e.duration_ = duration;
    e.octave_ = octave;
    e.velocity_ = velocity & 0x7F;
    e.midiChannel_ = midiChannel & 0x0F;
    return e;
}

GingoEvent GingoEvent::rest(const GingoDuration& duration) {
    GingoEvent e;
    e.type_ = EVENT_REST;
    e.duration_ = duration;
    return e;
}

GingoEvent GingoEvent::fromMIDI(uint8_t midiNote,
                                const GingoDuration& duration,
                                uint8_t velocity,
                                uint8_t midiChannel) {
    GingoNote note = GingoNote::fromMIDI(midiNote);
    int8_t octave = GingoNote::octaveFromMIDI(midiNote);
    return noteEvent(note, duration, (uint8_t)octave, velocity, midiChannel);
}

uint8_t GingoEvent::midiNumber() const {
    switch (type_) {
        case EVENT_NOTE:  return note_.midiNumber(octave_);
        case EVENT_CHORD: return chord_.root().midiNumber(octave_);
        default:          return 0;
    }
}

float GingoEvent::frequency() const {
    switch (type_) {
        case EVENT_NOTE:  return note_.frequency(octave_);
        case EVENT_CHORD: return chord_.root().frequency(octave_);
        default:          return 0.0f;
    }
}

GingoEvent GingoEvent::transpose(int8_t semitones) const {
    switch (type_) {
        case EVENT_NOTE:
            return noteEvent(note_.transpose(semitones), duration_, octave_);
        case EVENT_CHORD:
            return chordEvent(chord_.transpose(semitones), duration_, octave_);
        default:
            return *this;
    }
}

} // namespace gingoduino

#endif // GINGODUINO_HAS_EVENT
