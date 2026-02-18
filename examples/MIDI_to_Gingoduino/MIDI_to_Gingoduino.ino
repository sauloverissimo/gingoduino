// Gingoduino v0.2.0 — MIDI to Music Theory
//
// This example demonstrates converting MIDI input to Gingoduino structures.
// It uses ESP32_Host_MIDI to receive MIDI from USB or BLE and analyzes
// the incoming notes/chords with Gingoduino.
//
// Hardware: ESP32-S3 (recommended) with T-Display S3 (optional)
// Libraries required:
//   - Gingoduino (this library)
//   - ESP32_Host_MIDI (https://github.com/sauloverissimo/ESP32_Host_MIDI)
//   - TFT_eSPI (if using display)
//
// Controls:
//   - Connect MIDI keyboard via USB (OTG) or BLE from iOS/Android
//   - Serial monitor shows MIDI→Gingoduino conversion in real-time
//

#include <Gingoduino.h>

#if defined(ARDUINO_ARCH_ESP32)
  #include <ESP32_Host_MIDI.h>  // https://github.com/sauloverissimo/ESP32_Host_MIDI
#endif

using namespace gingoduino;

// Forward declaration
void onMIDIEvent(const MIDIEventData& event);
void displayNoteInfo(const GingoNote& note, int8_t octave, uint8_t midiNum);

void setup() {
    Serial.begin(115200);
    delay(2000);  // Wait for serial monitor to open

    Serial.println("\n=== Gingoduino MIDI-to-Theory ===");
    Serial.println("Waiting for MIDI input...\n");

#if defined(ARDUINO_ARCH_ESP32)
    // Initialize ESP32_Host_MIDI for USB OTG and BLE
    midiHandler.begin();
    Serial.println("MIDI handlers initialized (USB + BLE)");
#else
    Serial.println("This example requires ESP32 with ESP32_Host_MIDI");
#endif
}

void loop() {
#if defined(ARDUINO_ARCH_ESP32)
    // Process incoming MIDI messages
    midiHandler.task();

    // Check if there are new events
    const auto& queue = midiHandler.getQueue();
    static int lastProcessed = 0;

    if (!queue.empty()) {
        int newEvents = queue.size() - lastProcessed;
        if (newEvents > 0) {
            // Process only the new events
            for (int i = lastProcessed; i < (int)queue.size(); i++) {
                onMIDIEvent(queue[i]);
            }
            lastProcessed = queue.size();
        }
    }
#endif

    delay(50);  // Small delay to avoid blocking
}

void onMIDIEvent(const MIDIEventData& event) {
    if (event.mensagem != "NoteOn" && event.mensagem != "NoteOff") {
        // Ignore CC, PitchBend, etc.
        return;
    }

    Serial.print("[");
    Serial.print(event.mensagem.c_str());
    Serial.print("] ");

    // Get MIDI note number
    uint8_t midiNum = (uint8_t)event.nota;

    // Convert to Gingoduino
    GingoNote note = GingoNote::fromMIDI(midiNum);
    int8_t octave = GingoNote::octaveFromMIDI(midiNum);

    // Display the note info
    displayNoteInfo(note, octave, midiNum);

    Serial.print(" | Velocity: ");
    Serial.print(event.velocidade);
    Serial.println();
}

void displayNoteInfo(const GingoNote& note, int8_t octave, uint8_t midiNum) {
    // Note name and octave
    Serial.print(note.name());
    Serial.print(octave);
    Serial.print(" (MIDI ");
    Serial.print(midiNum);
    Serial.print(") | ");

    // Semitone
    Serial.print("Semitone: ");
    Serial.print(note.semitone());
    Serial.print(" | ");

    // Frequency (in Hz, at standard A4=440)
    float freq = note.frequency(octave);
    Serial.print("Freq: ");
    Serial.print(freq, 1);
    Serial.print(" Hz");
}

// Notes for the future:
// 1. To identify chords, collect NoteOn events with the same blockIndex,
//    then use GingoChord::identify(notes, noteCount) to find the chord.
//
// 2. Example:
//    std::vector<std::string> notes = midiHandler.getAnswer("som");
//    GingoNote chordNotes[7];
//    for (size_t i = 0; i < notes.size() && i < 7; i++) {
//        chordNotes[i] = GingoNote(notes[i].c_str());
//    }
//    char identified[16];
//    if (GingoChord::identify(chordNotes, (uint8_t)notes.size(), identified, sizeof(identified))) {
//        Serial.println(identified);
//    }
//
// 3. See RealtimeChordIdentifier.ino for a complete implementation.
