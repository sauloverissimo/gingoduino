// Gingoduino — Music Theory Library for Embedded Systems
// GingoMIDI1: raw MIDI 1.0 byte stream dispatcher.
//
// Two components:
//
//   GingoMIDI1 (static class)
//     Stateless dispatcher for pre-parsed MIDI 1.0 messages.
//     Mirrors GingoMIDI2::dispatch() for the raw-byte world.
//     Use when your transport already splits bytes into (status, data1, data2),
//     e.g. Arduino MIDI Library callbacks.
//
//       void handleNoteOn(byte ch, byte note, byte vel) {
//           GingoMIDI1::dispatch(0x90 | (ch - 1), note, vel, mon);
//       }
//       void handleNoteOff(byte ch, byte note, byte vel) {
//           GingoMIDI1::dispatch(0x80 | (ch - 1), note, vel, mon);
//       }
//       void handleCC(byte ch, byte cc, byte val) {
//           GingoMIDI1::dispatch(0xB0 | (ch - 1), cc, val, mon);
//       }
//
//   GingoMIDI1Parser (struct)
//     Stateful parser for raw byte streams (DIN MIDI, BLE MIDI, USB MIDI 1.0).
//     Handles running status, SysEx absorption, and real-time bytes (0xF8-0xFF).
//     Feed one byte at a time via feed(); call reset() to restart cleanly.
//
//       // DIN MIDI at 31250 baud (UART ISR or polling loop):
//       GingoMIDI1Parser parser;
//       while (Serial2.available()) {
//           parser.feed(Serial2.read(), mon);
//       }
//
//       // BLE MIDI (strip the 2-byte BLE header, feed payload bytes):
//       parser.feed(bleByte, mon);
//
// SPDX-License-Identifier: MIT

#ifndef GINGO_MIDI1_H
#define GINGO_MIDI1_H

#include "gingoduino_config.h"

#if GINGODUINO_HAS_MIDI1

#include "gingoduino_types.h"
#include "GingoMonitor.h"

namespace gingoduino {

// ===========================================================================
// GingoMIDI1 — stateless dispatcher for pre-parsed MIDI 1.0 messages
// ===========================================================================

/// Stateless MIDI 1.0 dispatcher.
///
/// Accepts pre-parsed (status, data1, data2) tuples and routes them to a
/// GingoMonitor. All channels are accepted (channel nibble is ignored).
///
/// Handled messages:
///   • 0x9n Note On  — vel > 0 → noteOn, vel == 0 → noteOff (running-status trick)
///   • 0x8n Note Off — noteOff
///   • 0xBn CC 64    — sustain pedal (val >= 64 → on, val < 64 → off)
///   • 0xBn CC 123   — All Notes Off → reset()
class GingoMIDI1 {
public:
    /// Dispatch a pre-parsed MIDI 1.0 message to a GingoMonitor.
    ///
    /// @param status  Status byte (e.g. 0x90, 0x80, 0xB0). Channel nibble ignored.
    /// @param data1   First data byte (note number or CC number).
    /// @param data2   Second data byte (velocity or CC value).
    /// @param mon     GingoMonitor to receive the routed event.
    /// @returns       true if the message was handled.
    static bool dispatch(uint8_t status, uint8_t data1, uint8_t data2,
                         GingoMonitor& mon) {
        uint8_t type = status & 0xF0;

        // Note On — vel=0 treated as Note Off (running-status convention)
        if (type == 0x90) {
            if (data2 > 0) { mon.noteOn(data1, data2); return true; }
            mon.noteOff(data1); return true;
        }

        // Note Off
        if (type == 0x80) {
            mon.noteOff(data1); return true;
        }

        // Control Change
        if (type == 0xB0) {
            if (data1 == 64) {
                (data2 >= 64) ? mon.sustainOn() : mon.sustainOff();
                return true;
            }
            if (data1 == 123) { mon.reset(); return true; }  // All Notes Off
        }

        return false;  // unhandled message type
    }
};

// ===========================================================================
// GingoMIDI1Parser — stateful raw byte stream parser
// ===========================================================================

/// Stateful MIDI 1.0 byte stream parser.
///
/// Handles the full MIDI 1.0 serial protocol including:
///   • Running status — status byte reused until a new one arrives
///   • SysEx (0xF0 … 0xF7) — absorbed silently, parser state preserved
///   • Real-time bytes (0xF8-0xFF) — ignored without disrupting parser state
///
/// All state fits in 4 bytes — safe for stack allocation on any platform.
///
/// Usage:
///   GingoMIDI1Parser parser;
///   // in loop or ISR:
///   while (Serial.available()) parser.feed(Serial.read(), mon);
struct GingoMIDI1Parser {
    uint8_t status_;   ///< Current running status byte (0 = none)
    uint8_t data1_;    ///< First data byte accumulated
    uint8_t count_;    ///< Number of data bytes received for current message
    uint8_t inSysex_;  ///< 1 while absorbing SysEx bytes

    GingoMIDI1Parser() : status_(0), data1_(0), count_(0), inSysex_(0) {}

    /// Reset parser to initial state (e.g. after a MIDI port reconnection).
    void reset() {
        status_ = 0;
        data1_  = 0;
        count_  = 0;
        inSysex_= 0;
    }

    /// Feed one byte from the raw MIDI stream.
    ///
    /// Internally accumulates bytes and calls GingoMIDI1::dispatch() when a
    /// complete message is ready.
    ///
    /// @param b    Raw byte from the MIDI serial stream.
    /// @param mon  GingoMonitor to receive routed events.
    /// @returns    true if feeding this byte completed a handled message.
    bool feed(uint8_t b, GingoMonitor& mon) {
        // ── Real-time bytes (0xF8-0xFF): ignore, no state change ───────────
        if (b >= 0xF8) return false;

        // ── SysEx end ───────────────────────────────────────────────────────
        if (b == 0xF7) { inSysex_ = 0; return false; }

        // ── SysEx start ─────────────────────────────────────────────────────
        if (b == 0xF0) { inSysex_ = 1; count_ = 0; return false; }

        // ── Inside SysEx: absorb until 0xF7 ─────────────────────────────────
        if (inSysex_) return false;

        // ── System Common (0xF1-0xF6): clear running status, skip ───────────
        if (b >= 0xF1) { status_ = 0; count_ = 0; return false; }

        // ── Status byte (bit 7 set): start new message ───────────────────────
        if (b & 0x80) {
            status_ = b;
            count_  = 0;
            return false;
        }

        // ── Data byte: accumulate ─────────────────────────────────────────────
        if (!status_) return false;  // no running status yet — discard

        if (count_ == 0) {
            // First data byte
            uint8_t expected = dataLength_(status_);
            if (expected == 1) {
                // Single-data-byte message (e.g. Program Change, Channel Pressure)
                return GingoMIDI1::dispatch(status_, b, 0, mon);
            }
            data1_  = b;
            count_  = 1;
            return false;
        }

        // Second (and final) data byte — dispatch
        bool handled = GingoMIDI1::dispatch(status_, data1_, b, mon);
        count_ = 0;  // reset for running status: next data byte starts fresh
        return handled;
    }

private:
    /// Expected number of data bytes for a given status byte.
    /// Returns 1 for single-data messages, 2 for two-data messages.
    static uint8_t dataLength_(uint8_t status) {
        switch (status & 0xF0) {
            case 0x80:  // Note Off
            case 0x90:  // Note On
            case 0xA0:  // Aftertouch (polyphonic)
            case 0xB0:  // Control Change
            case 0xE0:  // Pitch Bend
                return 2;
            case 0xC0:  // Program Change
            case 0xD0:  // Channel Pressure
                return 1;
            default:
                return 2;
        }
    }
};

} // namespace gingoduino

#endif // GINGODUINO_HAS_MIDI1
#endif // GINGO_MIDI1_H
