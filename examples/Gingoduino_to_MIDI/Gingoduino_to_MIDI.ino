// Gingoduino v0.2.0 — Export Sequence to MIDI
//
// This example demonstrates creating a musical sequence with Gingoduino
// and exporting it as raw MIDI bytes to send to a synthesizer or DAW.
//
// Hardware: Arduino (AVR, ESP8266, ESP32) with MIDI serial output
// Libraries: Gingoduino only (no external dependencies)
//
// MIDI Output Options:
//   1. Serial MIDI (DIN-5 connector via 6N138 optocoupler)
//   2. USB MIDI (ESP32-S3 native)
//   3. BLE MIDI (ESP32 via separate library)
//   4. Software MIDI (any board via SoftwareSerial)
//
// This example uses Serial MIDI at 31250 baud.
//

#include <Gingoduino.h>

using namespace gingoduino;

// MIDI configuration
#define MIDI_BAUD     31250
#define MIDI_CHANNEL  1
#define MIDI_VELOCITY 100

// Sequence parameters
#define SEQUENCE_TEMPO    120
#define SEQUENCE_TIMESIG  44  // 4/4

// MIDI buffer size (adjust based on sequence length)
// Each note event produces 6 MIDI bytes (NoteOn + NoteOff)
#define MIDI_BUFFER_SIZE  256

void setup() {
    // Serial 1: MIDI output (hardware UART on ESP32/Teensy)
    // Serial: USB debug output
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n=== Gingoduino MIDI Export ===");
    Serial.println("Creating a C Major arpeggio sequence...\n");

    // Initialize MIDI serial (UART 1 on pins 17/18 for ESP32-S3)
    // For Arduino Uno/Nano, use SoftwareSerial instead
#if defined(ARDUINO_ARCH_ESP32)
    Serial1.begin(MIDI_BAUD, SERIAL_8N1, 17, 18);  // RX, TX
    Serial.println("MIDI output on Serial1 (pins 17/18, 31250 baud)");
#else
    // For AVR boards, implement SoftwareSerial MIDI output
    // #include <SoftwareSerial.h>
    // SoftwareSerial midiSerial(10, 11);  // RX, TX
    // midiSerial.begin(MIDI_BAUD);
    Serial.println("Configure MIDI output for your board (SoftwareSerial for AVR)");
#endif

    // Create sequence: C Major arpeggio
    GingoSequence seq(GingoTempo(SEQUENCE_TEMPO), GingoTimeSig(4, 4));

    // Build: C E G C E G (ascending arpeggio, quarter notes)
    seq.add(GingoEvent::noteEvent(GingoNote("C"), GingoDuration("quarter"), 4));
    seq.add(GingoEvent::noteEvent(GingoNote("E"), GingoDuration("quarter"), 4));
    seq.add(GingoEvent::noteEvent(GingoNote("G"), GingoDuration("quarter"), 4));
    seq.add(GingoEvent::noteEvent(GingoNote("C"), GingoDuration("quarter"), 5));

    Serial.print("Sequence created: ");
    Serial.print(seq.size());
    Serial.print(" notes, ");
    Serial.print(seq.totalBeats());
    Serial.println(" beats");

    // Export to MIDI
    uint8_t midiBuffer[MIDI_BUFFER_SIZE];
    uint16_t midiSize = seq.toMIDI(midiBuffer, sizeof(midiBuffer), MIDI_CHANNEL);

    Serial.print("MIDI data: ");
    Serial.print(midiSize);
    Serial.println(" bytes");

    // Send MIDI to serial port
    Serial.println("\nSending MIDI...");
    sendMIDI(midiBuffer, midiSize);

    Serial.println("Done!");
}

void loop() {
    // Repeat sequence every 5 seconds
    delay(5000);

    Serial.println("\nSending sequence again...");

    GingoSequence seq(GingoTempo(SEQUENCE_TEMPO), GingoTimeSig(4, 4));
    seq.add(GingoEvent::noteEvent(GingoNote("C"), GingoDuration("quarter"), 4));
    seq.add(GingoEvent::noteEvent(GingoNote("E"), GingoDuration("quarter"), 4));
    seq.add(GingoEvent::noteEvent(GingoNote("G"), GingoDuration("quarter"), 4));
    seq.add(GingoEvent::noteEvent(GingoNote("C"), GingoDuration("quarter"), 5));

    uint8_t midiBuffer[MIDI_BUFFER_SIZE];
    uint16_t midiSize = seq.toMIDI(midiBuffer, sizeof(midiBuffer), MIDI_CHANNEL);

    sendMIDI(midiBuffer, midiSize);
}

void sendMIDI(uint8_t* data, uint16_t size) {
#if defined(ARDUINO_ARCH_ESP32)
    Serial1.write(data, size);
    Serial1.flush();
    Serial.print("Sent ");
    Serial.print(size);
    Serial.println(" bytes on Serial1");
#else
    // For other boards, implement your MIDI output here
    Serial.println("Configure MIDI output for your board");
#endif

    // Also print hex dump for debugging
    Serial.print("Hex: ");
    for (uint16_t i = 0; i < size; i++) {
        if (data[i] < 0x10) Serial.print("0");
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

// Example: More advanced sequence with different note values
/*
void createAdvancedSequence(GingoSequence& seq) {
    // C Major scale: C D E F G A B C (whole notes become half notes)
    seq.add(GingoEvent::noteEvent(GingoNote("C"), GingoDuration("half"), 4));
    seq.add(GingoEvent::noteEvent(GingoNote("D"), GingoDuration("half"), 4));
    seq.add(GingoEvent::noteEvent(GingoNote("E"), GingoDuration("half"), 4));
    seq.add(GingoEvent::rest(GingoDuration("half")));  // Rest for 2 beats

    // Chord as individual notes
    seq.add(GingoEvent::noteEvent(GingoNote("C"), GingoDuration("quarter"), 4));
    seq.add(GingoEvent::noteEvent(GingoNote("E"), GingoDuration("quarter"), 4));
    seq.add(GingoEvent::noteEvent(GingoNote("G"), GingoDuration("quarter"), 4));
    seq.add(GingoEvent::noteEvent(GingoNote("E"), GingoDuration("quarter"), 4));
}
*/

// MIDI Wiring (DIN-5 male connector):
//   2 (GND)         ← Arduino GND
//   4 (+5V via resistor) ← 220Ω ← Arduino +5V
//   5 (MIDI Out)    ← 6N138 pin 6 (output)
//
// 6N138 Optocoupler:
//   Pin 1 (Cathode) ← Arduino TX (Serial1 TX on pin 18)
//   Pin 2 (Anode)   ← Arduino GND (via 220Ω resistor)
//   Pin 4 (GND)     ← Arduino GND
//   Pin 5 (VCC)     ← Arduino +5V
//   Pin 6 (Output)  → MIDI DIN-5 pin 5
//
