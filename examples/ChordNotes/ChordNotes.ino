// Gingoduino — ChordNotes Example
// Demonstrates GingoChord: construction, notes, intervals, identify.
//
// SPDX-License-Identifier: MIT

#include <Gingoduino.h>

using namespace gingoduino;

void printChord(const char* label, GingoChord& chord) {
    Serial.print(label);
    Serial.print(F(": "));
    Serial.print(chord.name());
    Serial.print(F("  root="));
    Serial.print(chord.root().name());
    Serial.print(F("  type="));
    Serial.print(chord.type());
    Serial.print(F("  notes=["));

    GingoNote notes[7];
    uint8_t n = chord.notes(notes, 7);
    for (uint8_t i = 0; i < n; i++) {
        if (i > 0) Serial.print(F(", "));
        Serial.print(notes[i].name());
    }
    Serial.println(F("]"));
}

void setup() {
    Serial.begin(9600);
    while (!Serial) {}

    Serial.println(F("=== Gingoduino: Chord Notes ===\n"));

    // Major chord
    GingoChord cMaj("CM");
    printChord("C Major", cMaj);

    // Minor seventh
    GingoChord dm7("Dm7");
    printChord("D minor 7", dm7);

    // Dominant seventh
    GingoChord g7("G7");
    printChord("G dominant 7", g7);

    // Major seventh
    GingoChord fMaj7("F7M");
    printChord("F major 7", fMaj7);

    // Diminished
    GingoChord bdim("Bdim");
    printChord("B diminished", bdim);

    // Interval labels
    Serial.print(F("\nDm7 intervals: ["));
    LabelStr labels[7];
    uint8_t nl = dm7.intervalLabels(labels, 7);
    for (uint8_t i = 0; i < nl; i++) {
        if (i > 0) Serial.print(F(", "));
        Serial.print(labels[i].c_str());
    }
    Serial.println(F("]"));

    // Contains check
    Serial.print(F("Dm7 contains F? "));
    Serial.println(dm7.contains(GingoNote("F")) ? "yes" : "no");
    Serial.print(F("Dm7 contains F#? "));
    Serial.println(dm7.contains(GingoNote("F#")) ? "yes" : "no");

    // Transpose
    GingoChord transposed = cMaj.transpose(5); // CM + 5 = FM
    Serial.print(F("\nCM transposed +5: "));
    Serial.println(transposed.name());

    // Identify from notes
    GingoNote testNotes[3] = {GingoNote("E"), GingoNote("G#"), GingoNote("B")};
    char identified[16];
    if (GingoChord::identify(testNotes, 3, identified, sizeof(identified))) {
        Serial.print(F("Identified [E, G#, B]: "));
        Serial.println(identified);
    }

    Serial.println(F("\nDone!"));
}

void loop() {}
