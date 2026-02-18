// Gingoduino — BasicNote Example
// Demonstrates GingoNote: creation, transposition, frequency, MIDI.

#include <Gingoduino.h>

using namespace gingoduino;

void setup() {
    Serial.begin(9600);
    while (!Serial) {}  // wait for Serial on boards with native USB

    Serial.println(F("=== Gingoduino: Basic Note ===\n"));

    // Create notes
    GingoNote c("C");
    GingoNote bb("Bb");
    GingoNote fSharp("F#");

    // Name and natural form
    Serial.print(F("Note: "));      Serial.println(c.name());
    Serial.print(F("Bb natural: ")); Serial.println(bb.natural());
    Serial.print(F("Bb semitone: ")); Serial.println(bb.semitone());

    // Enharmonic check
    GingoNote aSharp("A#");
    Serial.print(F("Bb == A#? "));
    Serial.println(bb.isEnharmonic(aSharp) ? "yes" : "no");

    // Frequency
    Serial.print(F("A4 frequency: "));
    Serial.println(GingoNote("A").frequency(4), 2);

    Serial.print(F("C4 frequency: "));
    Serial.println(c.frequency(4), 2);

    // MIDI numbers
    Serial.print(F("C4 MIDI: "));  Serial.println(c.midiNumber(4));
    Serial.print(F("A4 MIDI: "));  Serial.println(GingoNote("A").midiNumber(4));

    // Transposition
    GingoNote up5 = c.transpose(7);  // C + 5J = G
    Serial.print(F("C + 7 semitones: ")); Serial.println(up5.name());

    GingoNote down3 = c.transpose(-3); // C - 3m = A
    Serial.print(F("C - 3 semitones: ")); Serial.println(down3.name());

    // Distance
    Serial.print(F("C to F# distance: "));
    Serial.println(c.distance(fSharp));

    Serial.println(F("\nDone!"));
}

void loop() {
    // nothing
}
