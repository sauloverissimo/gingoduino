// Gingoduino — Music Theory Library for Embedded Systems
// GingoMonitor: event-driven harmonic state tracker.
//
// Receives MIDI note-on/off events and fires callbacks when the harmonic
// state changes (new chord, new field, or any note-on with context).
//
// Two callback styles are provided:
//   • Function pointers (all tiers, zero-heap, compatible with AVR and
//     the Arduino MIDI Library pattern)
//   • std::function (Tier 3 only: ESP32-S3, Teensy, Daisy Seed — same
//     pattern as AM_MIDI2.0Lib's setChannelVoiceMessage([](UMP&){...}))
//
// Both styles can be used simultaneously; std::function takes precedence
// on Tier 3 when both are registered for the same event.
//
// Polling is always available alongside callbacks for backward compat.
//
// SPDX-License-Identifier: MIT

#ifndef GINGO_MONITOR_H
#define GINGO_MONITOR_H

#include "gingoduino_config.h"

#if GINGODUINO_HAS_MONITOR

#include "gingoduino_types.h"
#include "GingoNote.h"
#include "GingoChord.h"
#include "GingoScale.h"
#include "GingoField.h"
#include "GingoNoteContext.h"

#if GINGODUINO_TIER >= 3
  #include <functional>
#endif

namespace gingoduino {

/// Event-driven harmonic state tracker.
///
/// Feed MIDI events via noteOn() / noteOff(). The monitor identifies
/// the current chord, deduces the most likely harmonic field, and fires
/// registered callbacks when state changes.
///
/// Examples:
///   GingoMonitor mon;
///
///   // Function pointer style (all tiers):
///   mon.onChordDetected([](const GingoChord& c, void*) {
///       Serial.println(c.name());
///   });
///
///   // std::function style (Tier 3 only):
///   mon.onChordDetected([](const GingoChord& c) {
///       display.print(c.name());
///   });
///
///   mon.noteOn(60);  // C4 — triggers analysis
///   mon.noteOn(64);  // E4
///   mon.noteOn(67);  // G4 — onChordDetected fires with "CM"
class GingoMonitor {
public:
    // ------------------------------------------------------------------
    // Callback types — function pointer style (all tiers, zero-heap)
    // ------------------------------------------------------------------

    /// Called when the identified chord changes.
    /// @param chord  The newly detected chord.
    /// @param ctx    User-provided context pointer (passed to onChordDetected).
    typedef void (*ChordCallback)(const GingoChord& chord, void* ctx);

    /// Called when the deduced harmonic field changes.
    /// @param field  The best-matching harmonic field.
    /// @param ctx    User-provided context pointer.
    typedef void (*FieldCallback)(const GingoField& field, void* ctx);

    /// Called on every noteOn, carrying per-note harmonic context.
    /// Fires even when no chord has been identified yet.
    /// @param ctx      Per-note context (degree, interval, function, inScale).
    /// @param userCtx  User-provided context pointer.
    typedef void (*NoteCallback)(const GingoNoteContext& ctx, void* userCtx);

    // ------------------------------------------------------------------
    // Constructor
    // ------------------------------------------------------------------

    GingoMonitor();

    // ------------------------------------------------------------------
    // Callback registration — function pointer style (all tiers)
    // ------------------------------------------------------------------

    /// Register a callback for chord changes.
    /// Pass ctx=nullptr if no user data is needed.
    void onChordDetected(ChordCallback cb, void* ctx = nullptr);

    /// Register a callback for harmonic field changes.
    void onFieldChanged(FieldCallback cb, void* ctx = nullptr);

    /// Register a callback fired on every noteOn with per-note context.
    void onNoteOn(NoteCallback cb, void* ctx = nullptr);

#if GINGODUINO_TIER >= 3
    // ------------------------------------------------------------------
    // Callback registration — std::function style (Tier 3 only)
    // Same ergonomics as AM_MIDI2.0Lib:
    //   processor.setChannelVoiceMessage([](UMP& u){ ... });
    // ------------------------------------------------------------------

    /// Register a chord-change callback with lambda capture support.
    void onChordDetected(std::function<void(const GingoChord&)> fn);

    /// Register a field-change callback with lambda capture support.
    void onFieldChanged(std::function<void(const GingoField&)> fn);

    /// Register a per-note callback with lambda capture support.
    void onNoteOn(std::function<void(const GingoNoteContext&)> fn);
#endif

    // ------------------------------------------------------------------
    // MIDI event feed
    // ------------------------------------------------------------------

    /// Process a MIDI Note On event.
    /// Adds the note, updates chord/field state, fires callbacks.
    /// @param midiNum   MIDI note number (0–127).
    /// @param velocity  MIDI velocity (1–127; ignored for state but stored).
    void noteOn(uint8_t midiNum, uint8_t velocity = 100);

    /// Process a MIDI Note Off event.
    /// Removes the note and re-evaluates state.
    /// @param midiNum  MIDI note number (0–127).
    void noteOff(uint8_t midiNum);

    /// Reset all held notes and clear chord/field state.
    void reset();

    // ------------------------------------------------------------------
    // Sustain pedal — called by the user or via CC64
    // ------------------------------------------------------------------

    /// Enable sustain. Notes released while sustain is active remain
    /// in the held list (contributing to chord/field detection) until
    /// sustainOff() is called.
    void sustainOn();

    /// Release sustain. Notes that were released while the pedal was
    /// held are removed and harmonic state is re-evaluated.
    void sustainOff();

    // ------------------------------------------------------------------
    // State access (polling — always available)
    // ------------------------------------------------------------------

    /// Number of currently held notes (includes sustained notes).
    uint8_t activeNoteCount() const { return heldCount_; }

    /// Whether the sustain pedal is active.
    bool hasSustain() const { return sustainHeld_; }

    /// Whether a chord has been identified from the held notes.
    bool hasChord() const { return chordValid_; }

    /// Currently identified chord. Check hasChord() first.
    const GingoChord& currentChord() const { return chord_; }

    /// Whether a harmonic field has been deduced.
    bool hasField() const { return fieldValid_; }

    /// Currently deduced harmonic field. Check hasField() first.
    const GingoField& currentField() const { return field_; }

private:
    // Held MIDI note numbers (max 16 simultaneous — practical theory limit)
    static const uint8_t MAX_HELD = 16;
    uint8_t held_[MAX_HELD];
    uint8_t heldCount_;

    // Sustain pedal state
    bool    sustainHeld_;          ///< Pedal is currently held
    bool    sustained_[MAX_HELD];  ///< Per-slot: key was released while sustain active

    // Current harmonic state
    GingoChord chord_;
    bool       chordValid_;
    GingoField field_;
    bool       fieldValid_;

    // Function pointer callbacks
    ChordCallback chordCb_;
    void*         chordCtx_;
    FieldCallback fieldCb_;
    void*         fieldCtx_;
    NoteCallback  noteCb_;
    void*         noteCtx_;

#if GINGODUINO_TIER >= 3
    // std::function callbacks (heap, Tier 3 only)
    std::function<void(const GingoChord&)>       chordFn_;
    std::function<void(const GingoField&)>       fieldFn_;
    std::function<void(const GingoNoteContext&)> noteFn_;
#endif

    // Internal helpers
    void analyse_();
    bool buildChordFromHeld_(GingoChord& out) const;
    bool deduceFieldFromHeld_(GingoField& out) const;
    void fireChord_(const GingoChord& c);
    void fireField_(const GingoField& f);
    void fireNote_(const GingoNoteContext& ctx);
};

} // namespace gingoduino

#endif // GINGODUINO_HAS_MONITOR
#endif // GINGO_MONITOR_H
