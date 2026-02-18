// Gingoduino — Music Theory Library for Embedded Systems
// Implementation of GingoField.
//
// SPDX-License-Identifier: MIT

#include "GingoField.h"

#if GINGODUINO_HAS_FIELD

#include "gingoduino_progmem.h"

namespace gingoduino {

// ---------------------------------------------------------------------------
// Harmonic function table per scale type (up to 8 degrees)
// ---------------------------------------------------------------------------

// Major:        T  S  T  S  D  T  D
// NaturalMinor: T  S  T  S  D  S  D
// HarmonicMinor:T  S  T  S  D  S  D
// MelodicMinor: T  S  T  S  D  S  D
// Diminished:   T  D  T  D  T  D  T  D

static const uint8_t FUNC_TABLE[][8] PROGMEM = {
    {0, 1, 0, 1, 2, 0, 2, 0},  // Major
    {0, 1, 0, 1, 2, 1, 2, 0},  // NaturalMinor
    {0, 1, 0, 1, 2, 1, 2, 0},  // HarmonicMinor
    {0, 1, 0, 1, 2, 1, 2, 0},  // MelodicMinor
    {0, 2, 0, 2, 0, 2, 0, 2},  // Diminished
    {0, 1, 0, 1, 2, 1, 2, 0},  // HarmonicMajor
    {0, 0, 0, 0, 0, 0, 0, 0},  // WholeTone (all tonic-like)
    {0, 0, 0, 0, 0, 0, 0, 0},  // Augmented
    {0, 1, 2, 0, 1, 2, 0, 0},  // Blues
    {0, 0, 0, 0, 0, 0, 0, 0},  // Chromatic
};

// Role names
static const char R_PRIMARY[]   PROGMEM = "primary";
static const char R_REL_I[]     PROGMEM = "relative of I";
static const char R_REL_IV[]    PROGMEM = "relative of IV";
static const char R_REL_V[]     PROGMEM = "relative of V";
static const char R_TRANS[]     PROGMEM = "transitive";

// Role table for Major scale (7 degrees)
static const char* const ROLE_TABLE_MAJOR[7] PROGMEM = {
    R_PRIMARY, R_REL_IV, R_TRANS, R_PRIMARY,
    R_PRIMARY, R_REL_I, R_REL_V
};

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

GingoField::GingoField(const char* tonic, ScaleType type)
    : scale_(tonic, type)
{}

GingoField::GingoField(const char* tonic, const char* typeName)
    : scale_(tonic, typeName)
{}

// ---------------------------------------------------------------------------
// Build chords by stacking scale degrees
// ---------------------------------------------------------------------------

uint8_t GingoField::buildChords(GingoChord* output, uint8_t maxChords,
                                const uint8_t* offsets, uint8_t offsetCount) const {
    // Get all scale notes
    GingoNote scaleNotes[12];
    uint8_t scaleSize = scale_.notes(scaleNotes, 12);
    if (scaleSize == 0) return 0;

    uint8_t written = 0;
    for (uint8_t i = 0; i < scaleSize && written < maxChords; i++) {
        // Collect chord tones by picking scale notes at the offsets
        GingoNote chordNotes[7];
        uint8_t chordNoteCount = 0;

        for (uint8_t j = 0; j < offsetCount && chordNoteCount < 7; j++) {
            uint8_t idx = (i + offsets[j]) % scaleSize;
            chordNotes[chordNoteCount++] = scaleNotes[idx];
        }

        // Try to identify the chord
        char chordName[16];
        if (GingoChord::identify(chordNotes, chordNoteCount, chordName, sizeof(chordName))) {
            output[written++] = GingoChord(chordName);
        } else {
            // Fallback: root + "M"
            char fallback[8];
            uint8_t pos = 0;
            const char* rn = chordNotes[0].name();
            while (rn[pos] && pos < 6) { fallback[pos] = rn[pos]; pos++; }
            fallback[pos++] = 'M';
            fallback[pos] = '\0';
            output[written++] = GingoChord(fallback);
        }
    }
    return written;
}

// ---------------------------------------------------------------------------
// Public accessors
// ---------------------------------------------------------------------------

uint8_t GingoField::chords(GingoChord* output, uint8_t maxChords) const {
    const uint8_t triadOffsets[] = {0, 2, 4}; // 1st, 3rd, 5th
    return buildChords(output, maxChords, triadOffsets, 3);
}

uint8_t GingoField::sevenths(GingoChord* output, uint8_t maxChords) const {
    const uint8_t seventhOffsets[] = {0, 2, 4, 6}; // 1st, 3rd, 5th, 7th
    return buildChords(output, maxChords, seventhOffsets, 4);
}

GingoChord GingoField::chord(uint8_t degree) const {
    GingoChord buf[12];
    uint8_t n = chords(buf, 12);
    if (degree >= 1 && degree <= n) return buf[degree - 1];
    return GingoChord(); // fallback
}

GingoChord GingoField::seventh(uint8_t degree) const {
    GingoChord buf[12];
    uint8_t n = sevenths(buf, 12);
    if (degree >= 1 && degree <= n) return buf[degree - 1];
    return GingoChord();
}

HarmonicFunc GingoField::function(uint8_t degree) const {
    if (degree < 1 || degree > size()) return FUNC_TONIC;
    uint8_t typeIdx = (uint8_t)scale_.parent();
    if (typeIdx >= SCALE_TYPE_COUNT) typeIdx = 0;
    return (HarmonicFunc)pgm_read_byte(&FUNC_TABLE[typeIdx][degree - 1]);
}

const char* GingoField::role(uint8_t degree, char* buf, uint8_t maxLen) const {
    if (scale_.parent() == SCALE_MAJOR && degree >= 1 && degree <= 7) {
        const char* ptr = (const char*)pgm_read_ptr(&ROLE_TABLE_MAJOR[degree - 1]);
        data::readPgmStr(buf, ptr, maxLen);
    } else {
        data::readPgmStr(buf, R_PRIMARY, maxLen);
    }
    return buf;
}

HarmonicFunc GingoField::functionOf(const GingoChord& chord) const {
    uint8_t deg = scale_.degreeOf(chord.root());
    if (deg == 0) return FUNC_TONIC;
    return function(deg);
}

HarmonicFunc GingoField::functionOf(const char* chordName) const {
    GingoChord c(chordName);
    return functionOf(c);
}

const char* GingoField::roleOf(const GingoChord& chord, char* buf, uint8_t maxLen) const {
    uint8_t deg = scale_.degreeOf(chord.root());
    if (deg == 0) {
        data::readPgmStr(buf, R_PRIMARY, maxLen);
        return buf;
    }
    return role(deg, buf, maxLen);
}

const char* GingoField::roleOf(const char* chordName, char* buf, uint8_t maxLen) const {
    GingoChord c(chordName);
    return roleOf(c, buf, maxLen);
}

} // namespace gingoduino

#endif // GINGODUINO_HAS_FIELD
