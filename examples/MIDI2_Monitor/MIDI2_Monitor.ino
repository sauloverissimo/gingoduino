// Gingoduino — MIDI 2.0 Monitor
//
// Demonstrates the full pipeline:
//
//   Serial MIDI in (31250 baud)
//     → GingoMIDI1Parser (running status, SysEx, real-time bytes)
//     → GingoMonitor (chord + field detection)
//     → GingoMIDI2 (UMP Flex Data generation)
//     → Serial output (115200 baud, USB debug)
//
// Example output:
//
//   === Gingoduino MIDI 2.0 Monitor ===
//   Waiting for MIDI on Serial1 (RX=16, 31250 baud)...
//
//   [Note On]  D  (chromatic)
//   [Note On]  F  (chromatic)
//   [Chord]    Dm
//     >> UMP chordName:    D0010006 20070000 00000000 00000000
//   [Note On]  A  degree=5  (Dominant)
//   [Field]    F Major  (sig=-1)
//     >> UMP keySignature: D0010005 20300000 00000000 00000000
//   [Note Off] D
//
// Hardware:
//   ESP32 (any variant), 3.3 V logic.
//   MIDI IN circuit (optocoupler 6N138 or H11L1):
//     DIN-5 pin 5 → 220 Ω → LED+ of optocoupler
//     DIN-5 pin 2 → GND
//     Collector of phototransistor → 3.3 V via 4.7 kΩ → GPIO 16 (RX)
//     Emitter → GND
//
// Pin mapping (ESP32-S3 default — adjust for your board):
//   Serial1 RX = GPIO 16   (MIDI data in)
//   Serial1 TX = GPIO 17   (not used for monitoring, keep floating)
//
// Tier: 3 (ESP32, RP2040, Teensy 4.x, Daisy Seed)
// Requires: Gingoduino library (Tier 3)
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

GingoMonitor     monitor;   // Harmonic state tracker
GingoMIDI1Parser parser;    // Stateful MIDI 1.0 byte stream parser

// ---------------------------------------------------------------------------
// Helper: print a GingoUMP as space-separated 8-char uppercase hex words
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
    // USB debug Serial (any baud the host accepts)
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== Gingoduino MIDI 2.0 Monitor ===");
    Serial.println("Waiting for MIDI on Serial1 (RX=16, 31250 baud)...\n");

    // Hardware UART for MIDI input (31250 baud, 8N1)
#if defined(ARDUINO_ARCH_ESP32)
    Serial1.begin(31250, SERIAL_8N1, MIDI_RX_PIN, MIDI_TX_PIN);
#else
    // Generic fallback — configure RX pin in your board's variant file
    Serial1.begin(31250);
#endif

    // -----------------------------------------------------------------------
    // Callbacks (std::function lambdas, Tier 3 feature)
    // -----------------------------------------------------------------------

    // Fires on every incoming Note On with per-note harmonic context.
    // GingoNoteContext carries: note, degree, interval, function, inScale.
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

    // Fires when the set of held notes forms a recognized chord.
    // Generates a MIDI 2.0 UMP Flex Data chordName packet (Status 0x06).
    // Forward ump.toBytesBE() to your MIDI 2.0 host or USB stack.
    monitor.onChordDetected([](const GingoChord& chord) {
        Serial.print("[Chord]    ");
        Serial.println(chord.name());

        GingoUMP ump = GingoMIDI2::chordName(chord);
        Serial.print("  >> UMP chordName:    ");
        printUMP(ump);
    });

    // Fires when the deduced harmonic field changes.
    // Generates a MIDI 2.0 UMP Flex Data keySignature packet (Status 0x05).
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
// Loop — feed raw MIDI bytes into the parser
// ---------------------------------------------------------------------------

void loop() {
    // Feed every incoming byte directly to the parser.
    // GingoMIDI1Parser handles: running status, SysEx, real-time bytes (0xF8+).
    // It calls monitor.noteOn() / noteOff() / sustainOn() / sustainOff()
    // automatically, which in turn fire the callbacks registered in setup().
    while (Serial1.available()) {
        parser.feed((uint8_t)Serial1.read(), monitor);
    }
}
