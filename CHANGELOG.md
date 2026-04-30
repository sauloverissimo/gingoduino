# Changelog

All notable changes to Gingoduino are documented in this file.
Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/);
versioning follows [Semantic Versioning](https://semver.org/).

## [0.4.0] - 2026-04-30

Architectural refocus: Gingoduino narrows to a music theory engine.
Anything that touches the wire (byte stream parsing, UMP receive
dispatch, MIDI-CI discovery and Property Exchange) is removed and
delegated to the transport layer (`midi2_cpp`, `ESP32_Host_MIDI`,
Arduino MIDI Library, etc.).

### Removed (breaking)

- `GingoMIDI1Parser` (running status, SysEx absorption, real-time byte
  pass-through).
- `GingoMIDI1::dispatch(status, data1, data2, monitor)` (input adapter).
  Note: `GingoMIDI1` was a class in 0.3.x and is now a namespace; any
  forward declarations of `class GingoMIDI1;` need to be removed.
- `GingoMIDI2::dispatch(words, monitor)` (UMP receive dispatch).
- `GingoMIDICI` namespace (`discoveryRequest`, `profileInquiryReply`,
  `capabilitiesJSON` and the gingoduino MIDI-CI Profile ID).
- `GingoEvent::toMIDI(buf)` member method.
- `GingoSequence::toMIDI(buf, maxLen, channel)` member method.

### Added

- `GingoMIDI1` namespaced output adapters mirroring `GingoMIDI2`'s shape:
  - `GingoMIDI1::fromEvent(event, buf, maxLen)`.
  - `GingoMIDI1::fromSequence(seq, buf, maxLen, channel)`.

### Changed (breaking)

- `GingoMIDI2::perNoteController` signature is now
  `(const GingoNoteContext& ctx, uint8_t midiNoteNum, uint8_t group, uint8_t channel)`.
  The previous shape (`uint8_t midiNoteNum` first, then `ctx`) did not match
  the documented intent of "encode degree and function as per-note RCC".

### Migration guide

- **Replacing `GingoMIDI1Parser`.** Use any external transport parser and
  call `GingoMonitor::noteOn / noteOff / sustainOn / sustainOff` directly.
  See `examples/MIDI2_Monitor/` for a self-contained ~30-line inline byte
  parser used as glue when reading raw MIDI 1.0 from a UART.

  ```cpp
  // Before
  GingoMIDI1Parser parser;
  while (Serial1.available()) parser.feed(Serial1.read(), monitor);

  // After: glue lives in the sketch (or in your transport library).
  // See examples/MIDI2_Monitor/MIDI2_Monitor.ino for a complete example.
  ```

- **Replacing `GingoMIDI1::dispatch(status, d1, d2, mon)`.** Call the
  matching `GingoMonitor` method directly from your callback:

  ```cpp
  // Before
  GingoMIDI1::dispatch(0x90 | (ch - 1), note, vel, monitor);

  // After (Arduino MIDI Library example)
  MIDI.setHandleNoteOn([](byte ch, byte note, byte vel) {
      monitor.noteOn(ch - 1, note, vel);   // 1-16 -> 0-15 (UMP convention)
  });
  ```

- **Replacing `GingoMIDI2::dispatch(words, mon)`.** Use `midi2_cpp` (or any
  UMP parser) and forward decoded callbacks to the Monitor:

  ```cpp
  ep.on_note_on([&](uint8_t g, uint8_t ch, uint8_t n, uint8_t v) {
      monitor.noteOn(ch, n, v);
  });
  ep.on_note_off([&](uint8_t g, uint8_t ch, uint8_t n, uint8_t v) {
      monitor.noteOff(ch, n);
  });
  ```

- **Replacing `GingoMIDICI`.** MIDI-CI device discovery, Profile Inquiry
  and Property Exchange are protocol-level concerns. Use a dedicated
  MIDI-CI library (the `midi2` C99 library covers responder/initiator
  flows with auto-MUID, collision detection and PE handshake).

- **Replacing `event.toMIDI(buf)`.**

  ```cpp
  // Before
  uint8_t n = event.toMIDI(buf);

  // After
  uint8_t n = GingoMIDI1::fromEvent(event, buf, sizeof(buf));
  ```

- **Replacing `seq.toMIDI(buf, len, channel)`.** Note the channel sentinel
  changed: in 0.3.x, `channel = 0` meant "keep each event's own channel";
  in 0.4.0, `0` is a valid override value (channel 0). Use the new
  `GingoMIDI1::KEEP_CHANNEL` (default) to preserve per-event channels.

  ```cpp
  // Before: channel=0 meant "keep per-event"
  uint16_t n = seq.toMIDI(buf, sizeof(buf), 0);

  // After: KEEP_CHANNEL is the default
  uint16_t n = GingoMIDI1::fromSequence(seq, buf, sizeof(buf));
  // ...or explicitly:
  uint16_t n = GingoMIDI1::fromSequence(seq, buf, sizeof(buf),
                                        GingoMIDI1::KEEP_CHANNEL);

  // Forcing a specific channel still works:
  uint16_t n = GingoMIDI1::fromSequence(seq, buf, sizeof(buf), 5);
  ```

- **Updating `perNoteController` calls.**

  ```cpp
  // Before
  GingoMIDI2::perNoteController(midiNote, ctx, group, channel);

  // After
  GingoMIDI2::perNoteController(ctx, midiNote, group, channel);
  ```

### Tests

- Native test suite: 399 tests, 0 failures (`-Wall -Wextra -Werror`).
- `cmidi2` integration test still passes with zero conflicts.

### Platform support

AVR (Tier 1, e.g. Arduino Uno/Nano) is best-effort starting with this
release. The library still compiles and the Tier 1 modules
(Note, Interval, Chord) keep their AVR-friendly properties, but AVR is
no longer part of CI and future modernizations may relax the C++11
constraints currently maintained for that target. Tier 2 (ESP8266) and
Tier 3 (ESP32, RP2040, Teensy, Daisy Seed) remain officially supported.

## [0.3.1] and earlier

See git history. Channel numbering aligned to 0-15 (UMP convention)
in 0.3.1; comprehensive theory and scale corrections in 0.3.0.
