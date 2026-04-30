// Gingoduino — MIDI 2.0 Monitor
//
// Reads raw MIDI 1.0 bytes from a UART (DIN MIDI optocoupler input),
// translates the bytes into musical events with a tiny inline parser,
// feeds them into a GingoMonitor, and prints the resulting harmonic
// analysis. When a chord or harmonic field is detected, a MIDI 2.0
// UMP Flex Data packet is generated and printed in hex.
//
// Pipeline:
//   UART bytes
//     → inline byte-stream parser (sketch-local, ~30 lines)
//     → GingoMonitor (chord + field detection)
//     → GingoMIDI2 (UMP Flex Data generation)
//
// Hardware:
//   ESP32 (any variant) with a MIDI 1.0 optocoupler on Serial1 RX.
//   Adapt MIDI_RX_PIN / MIDI_TX_PIN to your board.
//
// Why no built-in parser?
//   gingoduino is a music theory engine — it intentionally does not
//   know about UART byte streams, running status or SysEx. Use any
//   parser you already have (Arduino MIDI Library, midi2_cpp,
//   ESP32_Host_MIDI). The parser below is provided inline as a
//   self-contained reference for raw DIN MIDI input.
//
// SPDX-License-Identifier: MIT

#define GINGODUINO_TIER 3
#include <Gingoduino.h>

using namespace gingoduino;

// ---------------------------------------------------------------------------
// Pin configuration
// ---------------------------------------------------------------------------

#define MIDI_RX_PIN  16   // Serial1 RX — connect to MIDI optocoupler output
#define MIDI_TX_PIN  17   // Serial1 TX — unused for monitoring

// ---------------------------------------------------------------------------
// Library objects
// ---------------------------------------------------------------------------

GingoMonitor monitor;   // harmonic state tracker

// ---------------------------------------------------------------------------
// Inline MIDI 1.0 byte-stream parser (sketch-local glue)
// ---------------------------------------------------------------------------
//
// Handles running status, SysEx absorption, and real-time bytes (0xF8+).
// On a complete Note On / Note Off / CC64 / CC123 message, it calls the
// matching method on `monitor`. Anything else is dropped silently.

struct InlineMidiParser {
    uint8_t status = 0;
    uint8_t data1  = 0;
    uint8_t count  = 0;
    bool    inSysex = false;

    void feed(uint8_t b, GingoMonitor& mon) {
        if (b >= 0xF8)            return;            // real-time, ignore
        if (b == 0xF7)            { inSysex = false; return; }
        if (b == 0xF0)            { inSysex = true;  count = 0; return; }
        if (inSysex)              return;
        if (b >= 0xF1 && b <= 0xF6) { status = 0; count = 0; return; }

        if (b & 0x80) { status = b; count = 0; return; }   // status byte
        if (!status)  return;                              // no running status

        if (count == 0) {
            uint8_t need = needsTwoData_(status) ? 2 : 1;
            if (need == 1) { route_(status, b, 0, mon); return; }
            data1 = b; count = 1; return;
        }
        route_(status, data1, b, mon);
        count = 0;
    }

private:
    static bool needsTwoData_(uint8_t s) {
        switch (s & 0xF0) {
            case 0x80: case 0x90: case 0xA0: case 0xB0: case 0xE0: return true;
            default: return false;  // 0xC0 ProgChange, 0xD0 ChanPressure
        }
    }

    static void route_(uint8_t st, uint8_t d1, uint8_t d2, GingoMonitor& mon) {
        uint8_t kind = st & 0xF0;
        uint8_t ch   = st & 0x0F;
        if (kind == 0x90) {
            if (d2 > 0) mon.noteOn(ch, d1, d2);
            else        mon.noteOff(ch, d1);
        } else if (kind == 0x80) {
            mon.noteOff(ch, d1);
        } else if (kind == 0xB0) {
            if (d1 == 64)  { (d2 >= 64) ? mon.sustainOn() : mon.sustainOff(); }
            else if (d1 == 123) { mon.reset(); }
        }
    }
};

InlineMidiParser parser;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void printUMP(const GingoUMP& ump) {
    char hex[9];
    for (uint8_t i = 0; i < ump.wordCount; i++) {
        if (i) Serial.print(' ');
        snprintf(hex, sizeof(hex), "%08lX", (unsigned long)ump.words[i]);
        Serial.print(hex);
    }
    Serial.println();
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Gingoduino MIDI 2.0 Monitor ===");
    Serial.println("Waiting for MIDI on Serial1 (RX=16, 31250 baud)...\n");

#if defined(ARDUINO_ARCH_ESP32)
    Serial1.begin(31250, SERIAL_8N1, MIDI_RX_PIN, MIDI_TX_PIN);
#else
    Serial1.begin(31250);
#endif

    monitor.onNoteOn([](const GingoNoteContext& ctx) {
        Serial.print("[Note On]  ");
        Serial.print(ctx.note.name());
        if (ctx.inScale) {
            static const char* func[] = { "Tonic", "Subdominant", "Dominant" };
            Serial.print("  degree=");
            Serial.print((int)ctx.degree);
            Serial.print("  (");
            Serial.print(func[ctx.function]);
            Serial.print(")");
        } else {
            Serial.print("  (chromatic)");
        }
        Serial.println();
    });

    monitor.onChordDetected([](const GingoChord& chord) {
        Serial.print("[Chord]    ");
        Serial.println(chord.name());
        GingoUMP ump = GingoMIDI2::chordName(chord);
        Serial.print("  >> UMP chordName:    ");
        printUMP(ump);
    });

    monitor.onFieldChanged([](const GingoField& field) {
        int8_t sig = field.signature();
        char sigStr[5];
        snprintf(sigStr, sizeof(sigStr), "%+d", (int)sig);

        Serial.print("[Field]    ");
        Serial.print(field.tonic().name());
        Serial.print(' ');
        Serial.print(field.scale().quality());
        Serial.print("  (sig=");
        Serial.print(sigStr);
        Serial.println(")");

        GingoUMP ump = GingoMIDI2::keySignature(field.scale());
        Serial.print("  >> UMP keySignature: ");
        printUMP(ump);
    });
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------

void loop() {
    while (Serial1.available()) {
        parser.feed((uint8_t)Serial1.read(), monitor);
    }
}
