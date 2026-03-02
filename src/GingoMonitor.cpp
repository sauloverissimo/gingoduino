// Gingoduino — Music Theory Library for Embedded Systems
// Implementation of GingoMonitor.
//
// SPDX-License-Identifier: MIT

#include "GingoMonitor.h"

#if GINGODUINO_HAS_MONITOR

#include "GingoInterval.h"

namespace gingoduino {

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

GingoMonitor::GingoMonitor()
    : heldCount_(0)
    , sustainHeld_(false)
    , chordValid_(false)
    , fieldValid_(false)
    , chordCb_(nullptr), chordCtx_(nullptr)
    , fieldCb_(nullptr), fieldCtx_(nullptr)
    , noteCb_(nullptr),  noteCtx_(nullptr)
{
    for (uint8_t i = 0; i < MAX_HELD; i++) sustained_[i] = false;
}

// ---------------------------------------------------------------------------
// Callback registration — function pointer
// ---------------------------------------------------------------------------

void GingoMonitor::onChordDetected(ChordCallback cb, void* ctx) {
    chordCb_  = cb;
    chordCtx_ = ctx;
}

void GingoMonitor::onFieldChanged(FieldCallback cb, void* ctx) {
    fieldCb_  = cb;
    fieldCtx_ = ctx;
}

void GingoMonitor::onNoteOn(NoteCallback cb, void* ctx) {
    noteCb_  = cb;
    noteCtx_ = ctx;
}

#if GINGODUINO_TIER >= 3
// ---------------------------------------------------------------------------
// Callback registration — std::function (Tier 3)
// ---------------------------------------------------------------------------

void GingoMonitor::onChordDetected(std::function<void(const GingoChord&)> fn) {
    chordFn_ = fn;
}

void GingoMonitor::onFieldChanged(std::function<void(const GingoField&)> fn) {
    fieldFn_ = fn;
}

void GingoMonitor::onNoteOn(std::function<void(const GingoNoteContext&)> fn) {
    noteFn_ = fn;
}
#endif

// ---------------------------------------------------------------------------
// Internal: fire callbacks
// ---------------------------------------------------------------------------

void GingoMonitor::fireChord_(const GingoChord& c) {
    if (chordCb_) chordCb_(c, chordCtx_);
#if GINGODUINO_TIER >= 3
    if (chordFn_) chordFn_(c);
#endif
}

void GingoMonitor::fireField_(const GingoField& f) {
    if (fieldCb_) fieldCb_(f, fieldCtx_);
#if GINGODUINO_TIER >= 3
    if (fieldFn_) fieldFn_(f);
#endif
}

void GingoMonitor::fireNote_(const GingoNoteContext& ctx) {
    if (noteCb_) noteCb_(ctx, noteCtx_);
#if GINGODUINO_TIER >= 3
    if (noteFn_) noteFn_(ctx);
#endif
}

// ---------------------------------------------------------------------------
// Internal: build chord from held notes (first held note = root)
// ---------------------------------------------------------------------------

bool GingoMonitor::buildChordFromHeld_(GingoChord& out) const {
    if (heldCount_ < 2) return false;

    GingoNote notes[MAX_HELD];
    for (uint8_t i = 0; i < heldCount_; i++) {
        notes[i] = GingoNote::fromMIDI(held_[i]);
    }

    char buf[16];
    if (!GingoChord::identify(notes, heldCount_, buf, sizeof(buf))) return false;
    out = GingoChord(buf);
    return true;
}

// ---------------------------------------------------------------------------
// Internal: deduce field from held notes (uses GingoField::deduce)
// ---------------------------------------------------------------------------

bool GingoMonitor::deduceFieldFromHeld_(GingoField& out) const {
    if (heldCount_ < 1) return false;

    // Build note name array for deduce()
    char nameBufs[MAX_HELD][6];
    const char* names[MAX_HELD];
    for (uint8_t i = 0; i < heldCount_; i++) {
        GingoNote n = GingoNote::fromMIDI(held_[i]);
        const char* nm = n.name();
        uint8_t j = 0;
        while (nm[j] && j < 5) { nameBufs[i][j] = nm[j]; j++; }
        nameBufs[i][j] = '\0';
        names[i] = nameBufs[i];
    }

    FieldMatch matches[1];
    uint8_t found = GingoField::deduce(names, heldCount_, matches, 1);
    if (found == 0) return false;

    out = GingoField(matches[0].tonicName, matches[0].scaleType);
    return true;
}

// ---------------------------------------------------------------------------
// Internal: analyse held notes, update state, fire callbacks
// ---------------------------------------------------------------------------

void GingoMonitor::analyse_() {
    // --- Chord ---
    GingoChord newChord;
    bool newChordValid = buildChordFromHeld_(newChord);

    bool chordChanged = (newChordValid != chordValid_);
    if (!chordChanged && newChordValid && chordValid_) {
        chordChanged = (strcmp(newChord.name(), chord_.name()) != 0);
    }

    if (chordChanged) {
        chord_      = newChord;
        chordValid_ = newChordValid;
        if (chordValid_) fireChord_(chord_);
    }

    // --- Field (only deduced when chord is valid; debounced on change) ---
    if (chordValid_) {
        GingoField newField;
        bool newFieldValid = deduceFieldFromHeld_(newField);

        bool fieldChanged = (newFieldValid != fieldValid_);
        if (!fieldChanged && newFieldValid && fieldValid_) {
            // Compare tonic + scale type
            fieldChanged = (
                newField.tonic().semitone() != field_.tonic().semitone() ||
                newField.scale().parent()   != field_.scale().parent()
            );
        }

        if (fieldChanged) {
            field_      = newField;
            fieldValid_ = newFieldValid;
            if (fieldValid_) fireField_(field_);
        }
    } else if (!chordValid_ && fieldValid_) {
        fieldValid_ = false;
    }
}

// ---------------------------------------------------------------------------
// MIDI event feed
// ---------------------------------------------------------------------------

void GingoMonitor::noteOn(uint8_t midiNum, uint8_t velocity) {
    (void)velocity;

    // Add note (avoid duplicates); re-analyse only if new
    bool isNew = true;
    for (uint8_t i = 0; i < heldCount_; i++) {
        if (held_[i] == midiNum) { isNew = false; break; }
    }

    if (isNew) {
        // Un-sustain if the note was being sustained
        for (uint8_t i = 0; i < heldCount_; i++) {
            if (held_[i] == midiNum) { sustained_[i] = false; break; }
        }
        if (heldCount_ < MAX_HELD) {
            sustained_[heldCount_] = false;
            held_[heldCount_++] = midiNum;
        }
        analyse_();
    }

    // Fire per-note callback with harmonic context
    GingoNote note = GingoNote::fromMIDI(midiNum);
    GingoNoteContext ctx;
    if (fieldValid_) {
        ctx = field_.noteContext(note);
    } else {
        ctx.note     = note;
        ctx.degree   = 0;
        ctx.inScale  = false;
        ctx.function = FUNC_TONIC;
        ctx.interval = GingoInterval(static_cast<uint8_t>(0));
    }
    fireNote_(ctx);
}

void GingoMonitor::noteOff(uint8_t midiNum) {
    if (sustainHeld_) {
        // Mark note as sustained instead of removing
        for (uint8_t i = 0; i < heldCount_; i++) {
            if (held_[i] == midiNum) { sustained_[i] = true; break; }
        }
        return;  // Note stays in held_, no re-analysis
    }

    // Remove note
    uint8_t newCount = 0;
    for (uint8_t i = 0; i < heldCount_; i++) {
        if (held_[i] != midiNum) {
            held_[newCount] = held_[i];
            sustained_[newCount] = sustained_[i];
            newCount++;
        }
    }
    heldCount_ = newCount;
    analyse_();
}

void GingoMonitor::sustainOn() {
    sustainHeld_ = true;
}

void GingoMonitor::sustainOff() {
    sustainHeld_ = false;

    // Remove all notes that were released while pedal was held
    uint8_t newCount = 0;
    for (uint8_t i = 0; i < heldCount_; i++) {
        if (!sustained_[i]) {
            held_[newCount] = held_[i];
            sustained_[newCount] = false;
            newCount++;
        }
    }
    heldCount_ = newCount;
    analyse_();
}

void GingoMonitor::reset() {
    heldCount_    = 0;
    chordValid_   = false;
    fieldValid_   = false;
    sustainHeld_  = false;
    for (uint8_t i = 0; i < MAX_HELD; i++) sustained_[i] = false;
}

} // namespace gingoduino

#endif // GINGODUINO_HAS_MONITOR
