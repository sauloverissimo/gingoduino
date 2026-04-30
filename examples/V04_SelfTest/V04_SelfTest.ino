// Gingoduino v0.4.0 Self-Test
//
// Hardware-resident smoke test for the public surface that changed in
// v0.4.0. Runs entirely on a single ESP32 with no external MIDI source.
//
// Coverage:
//   * GingoMIDI1::fromEvent       (new namespaced output adapter)
//   * GingoMIDI1::fromSequence    (new namespaced output adapter)
//   * GingoMIDI1::KEEP_CHANNEL    (sentinel default)
//   * GingoMIDI2::chordName
//   * GingoMIDI2::keySignature
//   * GingoMIDI2::perNoteController(GingoNoteContext, midiNote, ...)  (corrected order)
//
// GingoMonitor analysis is NOT exercised here on purpose: the Monitor is
// pre-existing 0.3.x code scheduled for decomposition in 0.5.0 and is
// outside the v0.4.0 surface. The self-test only validates what 0.4.0
// actually changed.
//
// SPDX-License-Identifier: MIT

#define GINGODUINO_TIER 3
#include <Gingoduino.h>

using namespace gingoduino;

static void printHexBuf(const uint8_t* buf, uint16_t len) {
    char hex[3];
    for (uint16_t i = 0; i < len; i++) {
        if (i) Serial.print(' ');
        snprintf(hex, sizeof(hex), "%02X", buf[i]);
        Serial.print(hex);
    }
    Serial.println();
}

static void printUMP(const GingoUMP& ump) {
    char hex[9];
    for (uint8_t i = 0; i < ump.wordCount; i++) {
        if (i) Serial.print(' ');
        snprintf(hex, sizeof(hex), "%08lX", (unsigned long)ump.words[i]);
        Serial.print(hex);
    }
    Serial.println();
}

static int g_pass = 0;
static int g_fail = 0;

static void check(bool cond, const char* label) {
    if (cond) { g_pass++; Serial.print("  OK   "); }
    else      { g_fail++; Serial.print("  FAIL "); }
    Serial.println(label);
}

void setup() {
    Serial.begin(115200);
    delay(2500);
    Serial.println();
    Serial.println("=== Gingoduino v0.4.0 Self-Test ===");
    Serial.println();

    // -----------------------------------------------------------------
    // 1. GingoMIDI1::fromEvent
    // -----------------------------------------------------------------
    Serial.println("[1] GingoMIDI1::fromEvent");
    {
        GingoEvent e = GingoEvent::noteEvent(GingoNote("C"),
                                              GingoDuration("quarter"), 4,
                                              100, 0);
        uint8_t buf[6];
        uint8_t n = GingoMIDI1::fromEvent(e, buf, sizeof(buf));
        Serial.print("    Bytes: "); Serial.println(n);
        Serial.print("    Hex:   "); printHexBuf(buf, n);
        check(n == 6,            "fromEvent writes 6 bytes");
        check(buf[0] == 0x90,    "NoteOn status 0x90 (ch0)");
        check(buf[1] == 60,      "NoteOn note 60 (C4)");
        check(buf[2] == 100,     "NoteOn velocity 100");
        check(buf[3] == 0x80,    "NoteOff status 0x80 (ch0)");
        check(buf[4] == 60,      "NoteOff note 60");
        check(buf[5] == 0,       "NoteOff velocity 0");
    }

    // Buffer too small returns zero
    {
        GingoEvent e = GingoEvent::noteEvent(GingoNote("C"),
                                              GingoDuration("quarter"), 4);
        uint8_t buf[4];
        uint8_t n = GingoMIDI1::fromEvent(e, buf, sizeof(buf));
        check(n == 0, "fromEvent returns 0 when maxLen<6");
    }
    Serial.println();

    // -----------------------------------------------------------------
    // 2. GingoMIDI1::fromSequence (default = KEEP_CHANNEL)
    // -----------------------------------------------------------------
    Serial.println("[2] GingoMIDI1::fromSequence (KEEP_CHANNEL default)");
    {
        GingoSequence seq(GingoTempo(120), GingoTimeSig(4, 4));
        seq.add(GingoEvent::noteEvent(GingoNote("C"),
                                       GingoDuration("quarter"), 4,
                                       100, 7));   // event channel = 7
        uint8_t buf[6];
        uint16_t n = GingoMIDI1::fromSequence(seq, buf, sizeof(buf));
        Serial.print("    Hex (default): "); printHexBuf(buf, n);
        check(n == 6,            "fromSequence default writes 6 bytes");
        check(buf[0] == (0x90 | 7), "default keeps event channel 7");
    }

    // Explicit override forces channel
    {
        GingoSequence seq(GingoTempo(120), GingoTimeSig(4, 4));
        seq.add(GingoEvent::noteEvent(GingoNote("C"),
                                       GingoDuration("quarter"), 4,
                                       100, 9));   // event channel = 9
        uint8_t buf[6];
        uint16_t n = GingoMIDI1::fromSequence(seq, buf, sizeof(buf), 0);
        Serial.print("    Hex (override 0): "); printHexBuf(buf, n);
        check(buf[0] == 0x90, "explicit channel=0 overrides event ch=9");
    }
    {
        GingoSequence seq(GingoTempo(120), GingoTimeSig(4, 4));
        seq.add(GingoEvent::noteEvent(GingoNote("C"),
                                       GingoDuration("quarter"), 4));
        uint8_t buf[6];
        uint16_t n = GingoMIDI1::fromSequence(seq, buf, sizeof(buf), 5);
        Serial.print("    Hex (override 5): "); printHexBuf(buf, n);
        check(buf[0] == (0x90 | 5), "explicit channel=5 forces channel 5");
        (void)n;
    }
    Serial.println();

    // -----------------------------------------------------------------
    // 3. GingoMIDI2::chordName
    // -----------------------------------------------------------------
    Serial.println("[3] GingoMIDI2::chordName");
    {
        GingoUMP ump = GingoMIDI2::chordName(GingoChord("CM"));
        Serial.print("    UMP CM:    "); printUMP(ump);
        check(ump.wordCount == 4,                       "wordCount=4");
        check(((ump.words[0] >> 28) & 0xF) == 0xD,      "MT=0xD (Flex Data)");
        check((ump.words[0] & 0xFF) == 0x06,            "status=0x06 (chord name)");
        check(((ump.words[1] >> 24) & 0xF) == 3,        "tonic letter=3 (C)");
        check(((ump.words[1] >> 16) & 0xFF) == 1,       "type=1 (Major)");
    }
    {
        GingoUMP ump = GingoMIDI2::chordName(GingoChord("Am7"));
        Serial.print("    UMP Am7:   "); printUMP(ump);
        check(((ump.words[1] >> 24) & 0xF) == 1,        "tonic letter=1 (A)");
        check(((ump.words[1] >> 16) & 0xFF) == 9,       "type=9 (Minor 7th)");
    }
    Serial.println();

    // -----------------------------------------------------------------
    // 4. GingoMIDI2::keySignature
    // -----------------------------------------------------------------
    Serial.println("[4] GingoMIDI2::keySignature");
    {
        GingoScale s("C", SCALE_MAJOR);
        GingoUMP ump = GingoMIDI2::keySignature(s);
        Serial.print("    UMP C major:   "); printUMP(ump);
        check(ump.wordCount == 4,                       "wordCount=4");
        check((ump.words[0] & 0xFF) == 0x05,            "status=0x05 (key signature)");
        check(((ump.words[1] >> 24) & 0xF) == 3,        "letter=3 (C)");
        check(((ump.words[1] >> 16) & 0xFF) == 0,       "mode=0 (major)");
    }
    {
        GingoScale s("A", SCALE_NATURAL_MINOR);
        GingoUMP ump = GingoMIDI2::keySignature(s, 3, 5);
        Serial.print("    UMP A minor (g=3, ch=5): "); printUMP(ump);
        check(((ump.words[0] >> 24) & 0xF) == 3,        "group=3");
        check(((ump.words[0] >> 16) & 0xF) == 5,        "channel=5");
        check(((ump.words[1] >> 24) & 0xF) == 1,        "letter=1 (A)");
        check(((ump.words[1] >> 16) & 0xFF) == 1,       "mode=1 (minor)");
    }
    Serial.println();

    // -----------------------------------------------------------------
    // 5. GingoMIDI2::perNoteController (new ctx-first signature)
    // -----------------------------------------------------------------
    Serial.println("[5] GingoMIDI2::perNoteController");
    {
        GingoField f("C", SCALE_MAJOR);
        GingoNoteContext ctx = f.noteContext(GingoNote("E"));
        GingoUMP ump = GingoMIDI2::perNoteController(ctx, /*midiNote=*/64);
        Serial.print("    UMP E in C major (midi=64): "); printUMP(ump);
        check(ump.wordCount == 2,                       "wordCount=2");
        check(((ump.words[0] >> 28) & 0xF) == 0x4,      "MT=0x4");
        check(((ump.words[1] >> 24) & 0xFF) == 3,       "degree=3");
        check(((ump.words[1] >> 16) & 0xFF) == (uint32_t)FUNC_TONIC,
                                                         "function=Tonic");
        check((ump.words[1] & 0xFF) == 1,               "inScale=1");
    }
    Serial.println();

    // -----------------------------------------------------------------
    // 6. GingoUMP serialization
    // -----------------------------------------------------------------
    Serial.println("[6] GingoUMP::toBytesBE");
    {
        GingoUMP ump = GingoMIDI2::chordName(GingoChord("CM"));
        uint8_t bytes[16];
        uint8_t n = ump.toBytesBE(bytes, sizeof(bytes));
        Serial.print("    Hex: "); printHexBuf(bytes, n);
        check(n == 16,                "toBytesBE writes 16 bytes");
        check(ump.byteCount() == 16,  "byteCount()=16");
        check(bytes[0] == 0xD0,       "first byte 0xD0 (MT=0xD, group=0)");
    }
    Serial.println();

    Serial.print("=== Self-Test ");
    Serial.print(g_fail == 0 ? "PASSED " : "FAILED ");
    Serial.print("("); Serial.print(g_pass); Serial.print("/");
    Serial.print(g_pass + g_fail); Serial.println(") ===");
}

void loop() {
    static uint32_t last = 0;
    uint32_t now = millis();
    if (now - last >= 5000) {
        last = now;
        Serial.print("[heartbeat] uptime=");
        Serial.print(now / 1000);
        Serial.println("s");
    }
}
