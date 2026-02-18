# Gingoduino Roadmap

## Overview

Gingoduino is a pure music theory library for embedded systems. Future roadmap focuses on **integration** with other tools and ecosystems, not adding complexity to the core library.

**Core Philosophy:**
- Gingoduino = theory (stays lean, portable, testable)
- Complementary libraries = I/O, MIDI, audio, rendering
- Clear separation of concerns

---

## v0.1.0 (🎉 Released)

### Completed
- ✅ Tier 1: Note, Interval, Chord
- ✅ Tier 2: Scale, Field, Duration, Tempo, TimeSig, Fretboard
- ✅ Tier 3: Event, Sequence
- ✅ 177 native tests passing
- ✅ Zero-heap architecture, C++11 compatible
- ✅ PROGMEM lookup tables for AVR
- ✅ Arduino Library Manager ready
- ✅ 5 examples + T-Display S3 interactive GUI
- ✅ Bilingual documentation (EN/PT)

### Known Limitations
- No MIDI I/O (intentional)
- No audio synthesis (too heavy)
- No MusicXML export (requires heavy serialization)
- No FFT or mic input (requires external libraries)

---

## v0.2.0 (Planned)

### A. MIDI Export & Import

**Goal:** Convert between Gingoduino structures and MIDI bytes

```cpp
// Export: Gingoduino → MIDI bytes
GingoSequence seq = ...;
uint8_t midiData[512];
uint16_t len = seq.toMIDI(midiData, sizeof(midiData));

// Import: MIDI bytes → Gingoduino
GingoNote note = GingoNote::fromMIDI(60);        // C4
uint8_t octave = GingoNote::octaveFromMIDI(60); // 4

GingoSequence seq = GingoSequence::fromMIDI(buffer, size);
```

**Scope:**
- MIDI note-on/off encoding
- Timing based on GingoDuration + GingoTempo
- Sequence to MIDI track conversion
- MIDI input parsing (simple)

**Not included:**
- SysEx, CC routing, MIDI 2.0
- Full MIDI file parsing (leave for specialized libs)

### B. ESP32_Host_MIDI Integration

**Goal:** Make ESP32_Host_MIDI compatible with Gingoduino

```
MIDI Input (USB/BLE)
    ↓
ESP32_Host_MIDI decodes
    ↓
GingoNote, GingoDuration, GingoChord
    ↓
Gingoduino processes (identify, transpose, analyze)
    ↓
GingoSequence
    ↓
toMIDI() → MIDI output
    ↓
Synth/Speaker
```

**Improvements to ESP32_Host_MIDI:**
- Add bridge functions: `midiEventToGingo()`
- Support GingoSequence import/export
- Documentation of integration

### C. Better Examples

- `MIDI_to_Gingoduino.ino` — receive MIDI, process with Gingoduino
- `Gingoduino_to_MIDI.ino` — create sequences, export to MIDI
- `RealtimeChordIdentifier.ino` — identify chords from incoming MIDI
- `SequenceTransposer.ino` — MIDI in → transpose → MIDI out

**New Library Recommendation:**
Create optional **`GingoduinoMIDI`** layer:
```cpp
#include <Gingoduino.h>
#include <GingoduinoMIDI.h>  // Optional

GingoSequence seq = ...;
MIDIBuffer buf = seq.toMIDI();  // Via GingoduinoMIDI
```

This keeps Gingoduino lightweight while providing MIDI as an optional add-on.

---

## v0.3.0 (Future)

### A. MusicXML Export (Optional)

**Goal:** Export Gingoduino sequences to standard notation

```cpp
GingoSequence seq = ...;
char xmlBuffer[4096];
uint16_t len = seq.toMusicXML(xmlBuffer, sizeof(xmlBuffer));

// Use with notation software: MuseScore, Finale, Dorico, etc.
```

**Considerations:**
- Heavy: XML serialization
- Could be separate library: `GingoduinoXML`
- Lightweight alt: export to JSON instead

**Not intended to:**
- Support full MusicXML spec (just notes + rhythm)
- Handle complex notation (tuplets, beams, dynamics, etc.)

### B. JSON Format (Lightweight)

Alternative to MusicXML for data interchange:

```cpp
GingoSequence seq = ...;
char jsonBuffer[2048];
uint16_t len = seq.toJSON(jsonBuffer, sizeof(jsonBuffer));

// Output:
// {"tempo": 120, "timeSig": "4/4", "events": [
//   {"type": "note", "pitch": "C4", "duration": "quarter"},
//   {"type": "rest", "duration": "eighth"}
// ]}
```

**Advantages:**
- Lightweight (fits on AVR)
- Web-friendly (send to browser for visualization)
- Easy to parse

---

## v1.0.0+ (Speculative)

### A. GingoduinoAudio (Separate Library)

For synthesis, FFT, mic input — **too heavy for Gingoduino itself**.

```cpp
#include <GingoduinoAudio.h>

// Synthesize GingoSequence
GingoSequence seq = ...;
AudioSynthesizer synth(pin, sampleRate);
synth.play(seq);

// Recognize chords from microphone
AudioAnalyzer analyzer(micPin);
GingoChord detected = analyzer.identifyChord();
```

**Components:**
- Tone generation (sine, square, sawtooth)
- Envelope (ADSR)
- FFT for analysis
- Mic input interface

### B. GingoduinoMIDI (Separate Library)

Official MIDI bridge with full features:

```cpp
#include <GingoduinoMIDI.h>

// USB Host MIDI input
MIDIInput midiIn;
GingoEvent event = midiIn.nextEvent();  // Auto-converts

// MIDI output
MIDIOutput midiOut;
midiOut.send(gingoSequence);  // Auto-converts
```

### C. Gingo Compatibility Layer

Port remaining modules from gingo C++17 if valuable:

- **Tree** (harmonic progressions via graph)
- **Progression** (cross-tradition progressions)
- **ChordComparison** (Neo-Riemannian, Forte numbers, voice leading)
- **Piano** visualization

**Decision:** Only if community requests or significant use case found.

---

## Architecture Decisions

### 1. **Separation of Concerns**

| Library | Responsibility | Dependencies |
|---------|-----------------|--------------|
| Gingoduino | Theory only | None |
| GingoduinoMIDI | MIDI I/O | Gingoduino |
| GingoduinoAudio | Synthesis/Analysis | Gingoduino |
| GingoduinoXML | Notation export | Gingoduino |
| ESP32_Host_MIDI | USB/BLE MIDI | GingoduinoMIDI (optional) |

### 2. **Tiered Compilation**

Keep using tier system:
- **Tier 1** (AVR): Note, Interval, Chord only
- **Tier 2** (ESP8266): + Scales, Fields, Fretboard
- **Tier 3** (ESP32+): + Events, Sequences

Additional tiers possible:
- **Tier 4** (future): MIDI I/O, JSON export (if added)

### 3. **No Runtime Dependencies**

- Gingoduino uses only C++ standard library + Arduino framework stubs
- MIDI layer (if any) stays minimal
- Audio layer (separate) can depend on external libs

---

## Contributing Guidelines

### For Gingoduino Core
- Keep it light, testable, portable
- No external dependencies
- All code must pass 177+ tests
- Update tests when adding features

### For Integration (v0.2+)
- Create separate `GingoduinoXYZ` libraries
- Document integration clearly
- Provide example sketches

### Testing
- Native tests: `g++ -std=c++11 -DGINGODUINO_TIER=3 -I. -o test_native extras/tests/test_native.cpp`
- Arduino tests: Upload examples to boards
- arduino-lint validation: `arduino-lint --library-manager submit --compliance strict .`

---

## Release Schedule

- **v0.1.0**: Initial Arduino Library Manager (Feb 2026) ✅
- **v0.2.0**: MIDI I/O + examples (Q2 2026)
- **v0.3.0**: XML/JSON export (Q3 2026)
- **v1.0.0**: Stable API + community feedback (Q4 2026+)

---

## Related Projects

### gingo (Original)
- Language: C++17
- Platform: macOS, Linux, Windows
- Features: Full music theory, CLI, Python bindings, audio, MusicXML
- Repository: https://github.com/sauloverissimo/gingo

### ESP32_Host_MIDI
- Language: C++ (Arduino)
- Platform: ESP32, T-Display S3
- Features: USB host MIDI, BLE MIDI, display handling
- Repository: https://github.com/sauloverissimo/ESP32_Host_MIDI
- **Future**: Integrate with Gingoduino for music analysis

### Gingoduino (This Project)
- Language: C++11 (Arduino)
- Platform: AVR, ESP8266, ESP32, RP2040, Teensy, Daisy Seed
- Features: Embedded music theory (Tier 1-3)
- Repository: https://github.com/sauloverissimo/gingoduino

---

## FAQ

### Q: Why not include MIDI in Gingoduino v0.1?
**A:** MIDI complicates testing and adds dependencies. Better to have a minimal core + modular add-ons.

### Q: Will Gingoduino support audio synthesis?
**A:** No. Too heavy for embedded. That's `GingoduinoAudio` (separate).

### Q: Can I generate MusicXML?
**A:** Not in v0.1. Planned for v0.3 (optional, separate library).

### Q: How do I convert MIDI to notes?
**A:** In v0.2: `GingoNote note = GingoNote::fromMIDI(60);`

### Q: Is Gingoduino 1:1 with gingo?
**A:** No. Gingoduino is a **port**, not a full port. Some features (Piano, Tree) dropped for embedded constraints. APIs are similar but not identical.

### Q: Can I use Gingoduino on my synth?
**A:** Yes! Gingoduino provides the music theory. You add MIDI/audio via separate libraries or custom code.

---

## Contact & Support

- **Author:** Saulo Verissimo
- **Email:** sauloverissimo@gmail.com
- **GitHub:** https://github.com/sauloverissimo
- **Issues:** https://github.com/sauloverissimo/gingoduino/issues
