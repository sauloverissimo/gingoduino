// Gingoduino — Music Theory Library for Embedded Systems
// GingoField: harmonic field — chords built from each scale degree.
//
// SPDX-License-Identifier: MIT

#ifndef GINGO_FIELD_H
#define GINGO_FIELD_H

#include "gingoduino_config.h"

#if GINGODUINO_HAS_FIELD

#include "gingoduino_types.h"
#include "GingoNote.h"
#include "GingoChord.h"
#include "GingoScale.h"

namespace gingoduino {

/// Represents a harmonic field — the diatonic chords built from each
/// degree of a scale.
///
/// Examples:
///   GingoField f("C", SCALE_MAJOR);
///   GingoChord triads[7];
///   f.chords(triads, 7);
///   // triads: CM, Dm, Em, FM, GM, Am, Bdim
class GingoField {
public:
    GingoField(const char* tonic, ScaleType type);
    GingoField(const char* tonic, const char* typeName);

    /// The tonic note.
    GingoNote tonic() const { return scale_.tonic(); }

    /// The underlying scale.
    const GingoScale& scale() const { return scale_; }

    /// Fill output with triads (3-note chords) for each degree.
    /// Returns number of chords written.
    uint8_t chords(GingoChord* output, uint8_t maxChords) const;

    /// Fill output with seventh chords for each degree.
    uint8_t sevenths(GingoChord* output, uint8_t maxChords) const;

    /// Get the triad at a specific degree (1-indexed).
    GingoChord chord(uint8_t degree) const;

    /// Get the seventh chord at a specific degree (1-indexed).
    GingoChord seventh(uint8_t degree) const;

    /// Harmonic function of a degree (1-indexed): 0=T, 1=S, 2=D.
    HarmonicFunc function(uint8_t degree) const;

    /// Role of a degree. Writes to buffer and returns it.
    const char* role(uint8_t degree, char* buf, uint8_t maxLen) const;

    /// Harmonic function of a chord in this field.
    /// Returns FUNC_TONIC if the chord root is not in the scale.
    HarmonicFunc functionOf(const GingoChord& chord) const;

    /// Harmonic function of a chord by name.
    HarmonicFunc functionOf(const char* chordName) const;

    /// Role of a chord in this field. Writes to buffer and returns it.
    const char* roleOf(const GingoChord& chord, char* buf, uint8_t maxLen) const;

    /// Role of a chord by name.
    const char* roleOf(const char* chordName, char* buf, uint8_t maxLen) const;

    /// Key signature (delegates to scale).
    int8_t signature() const { return scale_.signature(); }

    /// Number of degrees.
    uint8_t size() const { return scale_.size(); }

private:
    GingoScale scale_;

    /// Build chords by stacking intervals at given degree offsets.
    /// offsets: array of scale-degree offsets {0, 2, 4} for triads.
    uint8_t buildChords(GingoChord* output, uint8_t maxChords,
                        const uint8_t* offsets, uint8_t offsetCount) const;
};

} // namespace gingoduino

#endif // GINGODUINO_HAS_FIELD
#endif // GINGO_FIELD_H
