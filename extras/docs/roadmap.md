# Gingoduino Roadmap

## Current state — v0.3.0

**275 passing tests** · 0 failures · Tier 1 / 2 / 3 complete

### Implemented modules

| Module | Tier | Status |
|--------|------|--------|
| GingoNote | 1 | ✅ complete |
| GingoInterval | 1 | ✅ complete |
| GingoChord | 1 | ✅ complete — 42 chord formulas, `identify()` |
| GingoScale | 2 | ✅ complete — modes, signature, brightness, `colors()`, `walk()`, `neighbors()` |
| GingoField | 2 | ✅ complete — triads, sevenths, T/S/D functions, `deduce()`, `relative()`, `parallel()`, `applied()`, `neighbors()` |
| GingoDuration | 2 | ✅ complete — dots, tuplets, `operator+`, `operator*`, comparisons |
| GingoTempo | 2 | ✅ complete |
| GingoTimeSig | 2 | ✅ complete |
| GingoFretboard | 2 | ✅ complete — violão, cavaquinho, bandolim, ukulele |
| GingoEvent | 3 | ✅ complete — note/chord/rest, MIDI roundtrip |
| GingoSequence | 3 | ✅ complete — timeline, `transpose()`, MIDI export |
| GingoTree | 3 | ✅ complete — harmonic graph, traditions |
| GingoProgression | 3 | ✅ complete — `identify()`, `deduce()`, `predict()` |
| GingoChordComparison | 3 | ✅ complete — 17 dimensions (Neo-Riemannian, voice leading, Forte vectors) |
| GingoNoteContext | 2+ | ✅ complete — per-note harmonic context struct |
| GingoMonitor | 2+ | ✅ complete — event-driven tracker, dual callback API |
| GingoMIDI1 | 2+ | ✅ complete — raw MIDI byte parser, running status, SysEx |
| GingoMIDI2 | 3 | ✅ complete — UMP Flex Data (chordName, keySignature, perNoteController) |
| GingoMIDICI | 3 | ✅ complete — discoveryRequest, profileInquiryReply, capabilitiesJSON |

### What is NOT implemented (vs. gingo v1.1.0)

| Feature | Notes |
|---------|-------|
| FieldComparison | 21 contextual comparison dimensions — deferred (memory pressure on ESP8266) |
| Piano | Keyboard rendering — output-only, low priority for embedded |
| MusicXML | Full export — too verbose for embedded; MIDI export covers the use case |
| `Scale::formal_notes()` | Enharmonic spelling rules (e.g. Db major uses flats) |
| `Chord::formal_notes()` | Same, chord voice spelling |
| SVG rendering | Piano/fretboard SVG — host-side tool, not embedded |

---

## Path to v1.0.0

The v1.0.0 milestone signals **API stability**: no breaking changes after this tag.

### v0.4.0 — Formal notes + API polish
- `Scale::formal_notes()` — enharmonic spelling per scale (e.g. F major: Bb not A#)
- `Chord::formal_notes()` — same, chord voicing spelling
- Fix docstring: `GingoNoteContext.h` refers to `GingoField::noteContext()` — method was never added to `GingoField`; either implement it there or update the docstring to reflect inline construction
- Update test count in CLAUDE.md to match reality

### v0.5.0 — FieldComparison (conditional)
- `GingoFieldComparison` — 21 contextual comparison dimensions
- Only if RAM analysis shows it fits in ESP8266 Tier 2 budget; otherwise defer to post-v1.0.0
- Alternative: Tier 3 only, with `#if GINGODUINO_TIER >= 3` guard

### v1.0.0 — Stability signal
Requirements:
- ✅ CI green (GitHub Actions badge)
- ✅ Arduino Library Manager listed
- ✅ All Tier 1/2/3 modules tested
- ✅ Integration test proving zero conflict with cmidi2.h
- All module docstrings match the actual implementation (no phantom methods)
- `CHANGELOG.md` covering v0.1.0 → v1.0.0
- API freeze: `GINGODUINO_API_VERSION 1` macro in `gingoduino_config.h`

---

## Versioning policy

- **Patch** (0.x.y → 0.x.y+1): bug fixes, test additions, doc updates
- **Minor** (0.x → 0.x+1): new modules or new methods — backward compatible
- **Major** (0.x → 1.0.0): API freeze signal; no breaking changes after this

---

## Arduino Library Manager

Already listed: https://www.arduino.cc/reference/en/libraries/gingoduino/

---

## Ecosystem

| Project | Relationship | Status |
|---------|-------------|--------|
| [gingo](https://github.com/sauloverissimo/gingo) | Upstream C++17/Python source | Active |
| [cmidi2](https://github.com/atsushieno/cmidi2) | UMP reference library — zero-conflict verified | External |
| AM_MIDI2.0Lib | MIDI 2.0 transport for Arduino — complementary | External |
| LilyGo T-Display-S3 | Hardware target for GUI example | Validated |
