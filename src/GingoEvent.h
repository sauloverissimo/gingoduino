// Gingoduino — Music Theory Library for Embedded Systems
// GingoEvent: a musical event (note, chord, or rest) with duration.
//
// SPDX-License-Identifier: MIT

#ifndef GINGO_EVENT_H
#define GINGO_EVENT_H

#include "gingoduino_config.h"

#if GINGODUINO_HAS_EVENT

#include "gingoduino_types.h"
#include "GingoNote.h"
#include "GingoChord.h"
#include "GingoDuration.h"

namespace gingoduino {

/// Event type tag.
enum EventType : uint8_t {
    EVENT_NOTE,
    EVENT_CHORD,
    EVENT_REST
};

/// A musical event: a note, chord, or rest bound to a duration.
///
/// Uses a tagged union (no heap, no std::variant).
///
/// Examples:
///   auto e = GingoEvent::noteEvent(GingoNote("C"), GingoDuration("quarter"), 4);
///   e.type();          // EVENT_NOTE
///   e.note().name();   // "C"
///   e.midiNumber();    // 60
///
///   auto r = GingoEvent::rest(GingoDuration("half"));
///   r.type();          // EVENT_REST
class GingoEvent {
public:
    /// Default constructor (quarter-note rest).
    GingoEvent();

    /// Factory: create a note event.
    static GingoEvent noteEvent(const GingoNote& note,
                                const GingoDuration& duration,
                                uint8_t octave = 4);

    /// Factory: create a chord event.
    static GingoEvent chordEvent(const GingoChord& chord,
                                 const GingoDuration& duration,
                                 uint8_t octave = 4);

    /// Factory: create a rest event.
    static GingoEvent rest(const GingoDuration& duration);

    /// The event type.
    EventType type() const { return type_; }

    /// The note (valid only for EVENT_NOTE).
    const GingoNote& note() const { return note_; }

    /// The chord (valid only for EVENT_CHORD).
    const GingoChord& chord() const { return chord_; }

    /// The duration of this event.
    const GingoDuration& duration() const { return duration_; }

    /// The octave (valid for EVENT_NOTE and EVENT_CHORD).
    uint8_t octave() const { return octave_; }

    /// MIDI number of the note (EVENT_NOTE) or chord root (EVENT_CHORD).
    uint8_t midiNumber() const;

    /// Frequency in Hz (EVENT_NOTE) or root frequency (EVENT_CHORD).
    float frequency() const;

    /// Transpose the event by a number of semitones.
    GingoEvent transpose(int8_t semitones) const;

private:
    EventType    type_;
    GingoNote    note_;
    GingoChord   chord_;
    GingoDuration duration_;
    uint8_t      octave_;
};

} // namespace gingoduino

#endif // GINGODUINO_HAS_EVENT
#endif // GINGO_EVENT_H
