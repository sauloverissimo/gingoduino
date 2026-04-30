// Gingoduino — Music Theory Library for Embedded Systems
// GingoMIDI1: output adapters that translate musical structures
//             into raw MIDI 1.0 byte streams.
//
// Symmetric to GingoMIDI2 (UMP Flex Data senders): both are stateless,
// pure functions from a musical struct to a serialized format.
//
// Input from MIDI 1.0 byte streams is NOT handled here. Use an external
// parser (Arduino MIDI Library, ESP32_Host_MIDI, midi2_cpp, etc.) and
// call GingoMonitor::noteOn/noteOff/sustainOn/sustainOff directly.
//
// Examples:
//
//   uint8_t buf[6];
//   GingoEvent e = GingoEvent::noteEvent(GingoNote("C"),
//                                        GingoDuration("quarter"), 4);
//   uint8_t n = GingoMIDI1::fromEvent(e, buf, sizeof(buf));
//   // buf = [0x90, 60, 100, 0x80, 60, 0]
//
//   uint8_t out[256];
//   GingoSequence seq(GingoTempo(120), GingoTimeSig(4, 4));
//   seq.add(GingoEvent::noteEvent(GingoNote("C"),
//                                  GingoDuration("quarter"), 4));
//   uint16_t len = GingoMIDI1::fromSequence(seq, out, sizeof(out));
//
// SPDX-License-Identifier: MIT

#ifndef GINGO_MIDI1_H
#define GINGO_MIDI1_H

#include "gingoduino_config.h"

#if GINGODUINO_HAS_MIDI1

#include "gingoduino_types.h"

namespace gingoduino {

// Forward declarations — full headers included from the .cpp.
class GingoEvent;
#if GINGODUINO_HAS_SEQUENCE
class GingoSequence;
#endif

namespace GingoMIDI1 {

/// Serialize a single GingoEvent to raw MIDI 1.0 bytes.
/// Writes NoteOn followed by NoteOff. Rests produce 0 bytes.
/// @param event   Source event.
/// @param buf     Output buffer.
/// @param maxLen  Buffer capacity. Must be at least 6 for note events.
/// @returns       Bytes written: 6 for note/chord events, 0 for rest or insufficient buffer.
uint8_t fromEvent(const GingoEvent& event, uint8_t* buf, uint8_t maxLen);

#if GINGODUINO_HAS_SEQUENCE
/// Serialize a GingoSequence to a raw MIDI 1.0 byte stream.
/// Iterates the sequence and writes each event back to back.
/// @param seq      Source sequence.
/// @param buf      Output buffer.
/// @param maxLen   Buffer capacity. Stops early when the next event would not fit.
/// @param channel  Override MIDI channel (0-15). If 0, each event keeps its own channel.
/// @returns        Total bytes written.
uint16_t fromSequence(const GingoSequence& seq, uint8_t* buf,
                      uint16_t maxLen, uint8_t channel = 0);
#endif

} // namespace GingoMIDI1

} // namespace gingoduino

#endif // GINGODUINO_HAS_MIDI1
#endif // GINGO_MIDI1_H
