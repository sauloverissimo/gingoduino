// Gingoduino — Music Theory Library for Embedded Systems
// GingoNoteContext: per-note harmonic context (degree, interval, function).
//
// Returned by GingoField::noteContext(). Contains the harmonic context
// of a single note within a field: its scale degree, interval from the
// tonic, and harmonic function (Tonic / Subdominant / Dominant).
//
// MIDI 2.0 use: encode degree and function as per-note RCC values
// via GingoMIDI2::perNoteController().
//
// SPDX-License-Identifier: MIT

#ifndef GINGO_NOTE_CONTEXT_H
#define GINGO_NOTE_CONTEXT_H

#include "gingoduino_config.h"

#if GINGODUINO_HAS_FIELD

#include "gingoduino_types.h"
#include "GingoNote.h"
#include "GingoInterval.h"

namespace gingoduino {

/// Per-note harmonic context within a harmonic field.
///
/// Examples:
///   GingoField field("C", SCALE_MAJOR);
///   GingoNoteContext ctx = field.noteContext(GingoNote("E"));
///   // ctx.degree   = 3          (third degree of C major)
///   // ctx.inScale  = true
///   // ctx.function = FUNC_TONIC
///   // ctx.interval = GingoInterval(4)  (major third, 4 semitones)
struct GingoNoteContext {
    GingoNote     note;      ///< The note
    uint8_t       degree;    ///< Scale degree 1–7 (0 = not in scale)
    GingoInterval interval;  ///< Ascending interval from field tonic to this note
    HarmonicFunc  function;  ///< FUNC_TONIC, FUNC_SUBDOMINANT, or FUNC_DOMINANT
    bool          inScale;   ///< true when degree > 0
};

} // namespace gingoduino

#endif // GINGODUINO_HAS_FIELD
#endif // GINGO_NOTE_CONTEXT_H
