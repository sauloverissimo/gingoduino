// Gingoduino — HarmonicField Example
// Demonstrates GingoField: triads, sevenths, harmonic functions.
// Requires Tier 2 (ESP8266+, ESP32, Teensy, DaisySeed, Pico).

#include <Gingoduino.h>

#if !GINGODUINO_HAS_FIELD
  #error "HarmonicField requires Tier 2 or higher (define GINGODUINO_TIER >= 2)"
#endif

using namespace gingoduino;

const char* funcName(uint8_t f) {
    switch (f) {
        case FUNC_TONIC:       return "T";
        case FUNC_SUBDOMINANT: return "S";
        case FUNC_DOMINANT:    return "D";
        default:               return "?";
    }
}

void setup() {
    Serial.begin(9600);
    while (!Serial) {}

    Serial.println(F("=== Gingoduino: Harmonic Field ===\n"));

    // C Major field
    GingoField field("C", SCALE_MAJOR);

    Serial.println(F("--- C Major: Triads ---"));
    GingoChord triads[7];
    uint8_t nt = field.chords(triads, 7);
    for (uint8_t i = 0; i < nt; i++) {
        char roleBuf[16];
        Serial.print(F("  "));
        Serial.print(i + 1);
        Serial.print(F(". "));
        Serial.print(triads[i].name());

        // Pad to 8 chars for alignment
        uint8_t pad = 8 - strlen(triads[i].name());
        while (pad-- > 0) Serial.print(' ');

        Serial.print(F("func="));
        Serial.print(funcName(field.function(i + 1)));
        Serial.print(F("  role="));
        Serial.println(field.role(i + 1, roleBuf, sizeof(roleBuf)));
    }

    Serial.println(F("\n--- C Major: Sevenths ---"));
    GingoChord sevs[7];
    uint8_t ns = field.sevenths(sevs, 7);
    for (uint8_t i = 0; i < ns; i++) {
        Serial.print(F("  "));
        Serial.print(i + 1);
        Serial.print(F(". "));
        Serial.println(sevs[i].name());
    }

    // A Natural Minor field
    Serial.println(F("\n--- A Natural Minor: Triads ---"));
    GingoField minorField("A", SCALE_NATURAL_MINOR);
    GingoChord minTriads[7];
    uint8_t nm = minorField.chords(minTriads, 7);
    for (uint8_t i = 0; i < nm; i++) {
        Serial.print(F("  "));
        Serial.print(i + 1);
        Serial.print(F(". "));
        Serial.println(minTriads[i].name());
    }

    // Check specific degree
    Serial.println(F("\n--- Degree lookup ---"));
    GingoChord c5 = field.chord(5);  // V of C Major = G
    Serial.print(F("V of C Major: "));
    Serial.println(c5.name());

    GingoChord c5_7 = field.seventh(5);  // V7 of C Major = G7
    Serial.print(F("V7 of C Major: "));
    Serial.println(c5_7.name());

    Serial.println(F("\nDone!"));
}

void loop() {}
