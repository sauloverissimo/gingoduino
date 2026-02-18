// Gingoduino v0.2.0 — Real-Time Chord Identifier
//
// This example identifies chords in real-time from MIDI input.
// It groups notes by blockIndex (simultaneous note presses) and
// uses GingoChord::identify() to recognize the chord.
//
// Hardware: ESP32-S3 with ESP32_Host_MIDI
// Libraries required:
//   - Gingoduino
//   - ESP32_Host_MIDI
//   - (Optional) TFT_eSPI for T-Display S3
//
// How it works:
//   1. MIDI notes are received via USB keyboard or BLE
//   2. Notes pressed simultaneously have the same blockIndex
//   3. When a note in a block is released, the chord is identified
//   4. Result is displayed on Serial and optionally on display
//

#include <Gingoduino.h>

#if defined(ARDUINO_ARCH_ESP32)
  #include <ESP32_Host_MIDI.h>
#endif

using namespace gingoduino;

#define MAX_CHORD_NOTES 7
#define MAX_CHORD_NAME 16

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n=== Gingoduino Real-Time Chord Identifier ===");
    Serial.println("Play notes on MIDI keyboard (USB/BLE)\n");

#if defined(ARDUINO_ARCH_ESP32)
    midiHandler.begin();
    midiHandler.setQueueLimit(32);
    Serial.println("Ready for MIDI input...\n");
#else
    Serial.println("This example requires ESP32 with ESP32_Host_MIDI");
#endif
}

void loop() {
#if defined(ARDUINO_ARCH_ESP32)
    midiHandler.task();

    const auto& queue = midiHandler.getQueue();

    if (!queue.empty()) {
        // Check the last event to see if a chord was completed
        const MIDIEventData& lastEvent = queue.back();

        // Detect chord completion: NoteOff in a block closes the chord
        if (lastEvent.mensagem == "NoteOff") {
            // Get all notes in the last closed block
            int blockToAnalyze = lastEvent.blockIndex;
            identifyChordInBlock(queue, blockToAnalyze);
        }
    }
#endif

    delay(50);
}

void identifyChordInBlock(const std::deque<MIDIEventData>& queue, int blockIndex) {
    // Collect all unique notes in this block
    GingoNote chordNotes[MAX_CHORD_NOTES];
    uint8_t noteCount = 0;
    GingoNote rootNote;
    bool foundRoot = false;

    // Iterate backward through queue to find all notes in this block
    for (auto it = queue.rbegin(); it != queue.rend() && noteCount < MAX_CHORD_NOTES; ++it) {
        if (it->blockIndex == blockIndex && (it->mensagem == "NoteOn" || it->mensagem == "NoteOff")) {
            // Convert MIDI number to GingoNote
            GingoNote note = GingoNote::fromMIDI((uint8_t)it->nota);

            // Check if we already have this note (avoid duplicates)
            bool hasDuplicate = false;
            for (uint8_t i = 0; i < noteCount; i++) {
                if (chordNotes[i] == note) {
                    hasDuplicate = true;
                    break;
                }
            }

            if (!hasDuplicate) {
                // Store note (we'll sort later if needed)
                // For now, the first note becomes the root
                chordNotes[noteCount++] = note;

                if (!foundRoot) {
                    rootNote = note;
                    foundRoot = true;
                }
            }
        }
    }

    if (noteCount >= 2) {  // Only identify if 2+ notes
        // Try to identify the chord
        char identified[MAX_CHORD_NAME];
        identified[0] = '\0';

        if (GingoChord::identify(chordNotes, noteCount, identified, sizeof(identified))) {
            Serial.print("Block ");
            Serial.print(blockIndex);
            Serial.print(": ");
            Serial.print(noteCount);
            Serial.print(" notes → ");
            Serial.println(identified);

            // Also print the individual notes
            Serial.print("  Notes: ");
            for (uint8_t i = 0; i < noteCount; i++) {
                Serial.print(chordNotes[i].name());
                if (i < noteCount - 1) Serial.print(" ");
            }
            Serial.println();
        }
    } else if (noteCount == 1) {
        // Single note
        Serial.print("Block ");
        Serial.print(blockIndex);
        Serial.print(": Single note = ");
        Serial.println(chordNotes[0].name());
    }
}

// Tips for extending this:
//
// 1. Add display output (T-Display S3):
//    - Create GingoduinoDisplay helper to show chords on ST7789
//    - Show active notes and identified chord in real-time
//
// 2. Add transposition:
//    - Store last identified chord
//    - Allow buttons to transpose up/down
//    - Use GingoChord::transpose() to generate new chord
//
// 3. Add chord substitutions:
//    - Analyze chord function in a given scale
//    - Suggest substitute chords using GingoField
//    - Example: ii-V-I progression
//
// 4. MIDI output:
//    - Create a new transposed sequence
//    - Use GingoSequence::toMIDI() to send back out
//
