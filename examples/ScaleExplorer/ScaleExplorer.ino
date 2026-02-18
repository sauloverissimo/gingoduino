// Gingoduino — ScaleExplorer Example
// Demonstrates GingoScale: modes, pentatonic, contains.
// Requires Tier 2 (ESP8266+, ESP32, Teensy, DaisySeed, Pico).
//
// SPDX-License-Identifier: MIT

#include <Gingoduino.h>

#if !GINGODUINO_HAS_SCALE
  #error "ScaleExplorer requires Tier 2 or higher (define GINGODUINO_TIER >= 2)"
#endif

using namespace gingoduino;

void printScale(const char* label, GingoScale& scale) {
    Serial.print(label);
    Serial.print(F(":  tonic="));
    Serial.print(scale.tonic().name());

    char modeBuf[20];
    Serial.print(F("  mode="));
    Serial.print(scale.modeName(modeBuf, sizeof(modeBuf)));

    Serial.print(F("  quality="));
    Serial.print(scale.quality());

    Serial.print(F("  notes=["));
    GingoNote notes[12];
    uint8_t n = scale.notes(notes, 12);
    for (uint8_t i = 0; i < n; i++) {
        if (i > 0) Serial.print(F(", "));
        Serial.print(notes[i].name());
    }
    Serial.println(F("]"));
}

void setup() {
    Serial.begin(9600);
    while (!Serial) {}

    Serial.println(F("=== Gingoduino: Scale Explorer ===\n"));

    // C Major
    GingoScale cMaj("C", SCALE_MAJOR);
    printScale("C Major", cMaj);

    // A Natural Minor
    GingoScale aNatMin("A", SCALE_NATURAL_MINOR);
    printScale("A Natural Minor", aNatMin);

    // D Dorian (mode 2 of C Major)
    GingoScale dDorian = cMaj.mode(2);
    printScale("D Dorian", dDorian);

    // G Mixolydian (mode 5 of C Major)
    GingoScale gMixo = cMaj.mode(5);
    printScale("G Mixolydian", gMixo);

    // Pentatonic version
    GingoScale cPenta = cMaj.pentatonic();
    printScale("C Major Pentatonic", cPenta);

    // Harmonic minor
    GingoScale aHarm("A", SCALE_HARMONIC_MINOR);
    printScale("A Harmonic Minor", aHarm);

    // Phrygian Dominant (mode 5 of Harmonic Minor)
    GingoScale phryDom = aHarm.mode(5);
    printScale("Phrygian Dominant", phryDom);

    // Contains check
    Serial.println();
    Serial.print(F("C Major contains F#? "));
    Serial.println(cMaj.contains(GingoNote("F#")) ? "yes" : "no");
    Serial.print(F("C Major contains F? "));
    Serial.println(cMaj.contains(GingoNote("F")) ? "yes" : "no");

    // Degree access
    Serial.print(F("C Major degree 5: "));
    Serial.println(cMaj.degree(5).name());

    // By name
    GingoScale blues("A", "blues");
    printScale("A Blues", blues);

    Serial.println(F("\nDone!"));
}

void loop() {}
