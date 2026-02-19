// Native test — compiles with g++ to verify gingoduino logic.
// No Arduino framework needed; gingoduino_config.h provides PROGMEM stubs.
//
// Build (from repo root):
//   g++ -std=c++11 -DGINGODUINO_TIER=3 -I. -o extras/tests/test_native extras/tests/test_native.cpp

#include <cstdio>
#include <cstring>
#include "src/Gingoduino.h"

// Pull in all .cpp files for a single-file build
#include "src/GingoNote.cpp"
#include "src/GingoInterval.cpp"
#include "src/GingoChord.cpp"
#include "src/GingoScale.cpp"
#include "src/GingoField.cpp"
#include "src/GingoDuration.cpp"
#include "src/GingoTempo.cpp"
#include "src/GingoTimeSig.cpp"
#include "src/GingoEvent.cpp"
#include "src/GingoSequence.cpp"
#include "src/GingoFretboard.cpp"
#include "src/GingoTree.cpp"
#include "src/GingoProgression.cpp"

using namespace gingoduino;

static int failures = 0;
static int tests = 0;

#define CHECK(cond, msg) do { \
    tests++; \
    if (!(cond)) { \
        printf("  FAIL: %s\n", msg); \
        failures++; \
    } else { \
        printf("  OK:   %s\n", msg); \
    } \
} while(0)

// =====================================================================
// Note
// =====================================================================

void testNote() {
    printf("\n=== GingoNote ===\n");

    GingoNote c("C");
    CHECK(strcmp(c.name(), "C") == 0, "C name");
    CHECK(c.semitone() == 0, "C semitone=0");
    CHECK(c.sound() == 'C', "C sound='C'");

    GingoNote bb("Bb");
    CHECK(strcmp(bb.natural(), "A#") == 0, "Bb natural=A#");
    CHECK(bb.semitone() == 10, "Bb semitone=10");

    GingoNote a("A");
    CHECK(a.midiNumber(4) == 69, "A4 MIDI=69");
    CHECK(c.midiNumber(4) == 60, "C4 MIDI=60");

    float freqA = a.frequency(4);
    CHECK(freqA > 439.0f && freqA < 441.0f, "A4 freq~440");

    GingoNote g = c.transpose(7);
    CHECK(strcmp(g.name(), "G") == 0, "C+7=G");

    GingoNote aDown = c.transpose(-3);
    CHECK(strcmp(aDown.name(), "A") == 0, "C-3=A");

    CHECK(c.distance(GingoNote("G")) == 1, "C to G fifths dist=1");

    GingoNote aSharp("A#");
    CHECK(bb.isEnharmonic(aSharp), "Bb enharmonic A#");

    GingoNote fSharp("F#");
    CHECK(fSharp.semitone() == 6, "F# semitone=6");

    GingoNote eFlat("Eb");
    CHECK(strcmp(eFlat.natural(), "D#") == 0, "Eb natural=D#");
}

// =====================================================================
// Interval
// =====================================================================

void testInterval() {
    printf("\n=== GingoInterval ===\n");

    GingoInterval p5(7);  // 7 semitones = perfect fifth
    char buf[8];
    CHECK(p5.semitones() == 7, "P5 semitones=7");
    CHECK(strcmp(p5.label(buf, sizeof(buf)), "5J") == 0, "P5 label=5J");
    CHECK(p5.degree() == 5, "P5 degree=5");

    GingoInterval m3(3);  // minor third
    CHECK(m3.semitones() == 3, "m3 semitones=3");
    CHECK(strcmp(m3.label(buf, sizeof(buf)), "3m") == 0, "m3 label=3m");

    GingoInterval fromLabel("3M");
    CHECK(fromLabel.semitones() == 4, "3M semitones=4");

    // Two-note constructor
    GingoInterval cToG(GingoNote("C"), GingoNote("G"));
    CHECK(cToG.semitones() == 7, "C→G = 7 semitones");

    // Octave & compound
    GingoInterval oct((uint8_t)12);
    CHECK(oct.octave() == 2, "octave=2 for 12st");
    CHECK(oct.isCompound(), "12st is compound");

    // Simple reduction
    GingoInterval b9((uint8_t)13);
    GingoInterval simple = b9.simple();
    CHECK(simple.semitones() == 1, "b9 simple = 1st");

    // Invert
    GingoInterval inv = p5.invert();
    CHECK(inv.semitones() == 5, "P5 invert = 5st (P4)");
}

void testIntervalExtended() {
    printf("\n=== GingoInterval (extended) ===\n");
    char buf[32];

    // Consonance
    GingoInterval p1((uint8_t)0);
    p1.consonance(buf, sizeof(buf));
    CHECK(strcmp(buf, "perfect") == 0, "P1 consonance=perfect");
    CHECK(p1.isConsonant(), "P1 is consonant");

    GingoInterval m3(3);
    m3.consonance(buf, sizeof(buf));
    CHECK(strcmp(buf, "imperfect") == 0, "m3 consonance=imperfect");
    CHECK(m3.isConsonant(), "m3 is consonant");

    GingoInterval m2(1);
    m2.consonance(buf, sizeof(buf));
    CHECK(strcmp(buf, "dissonant") == 0, "m2 consonance=dissonant");
    CHECK(!m2.isConsonant(), "m2 is not consonant");

    // Full names
    GingoInterval p5(7);
    p5.fullName(buf, sizeof(buf));
    CHECK(strcmp(buf, "Perfect Fifth") == 0, "P5 fullName=Perfect Fifth");

    p5.fullNamePt(buf, sizeof(buf));
    CHECK(strcmp(buf, "Quinta Justa") == 0, "P5 fullNamePt=Quinta Justa");

    GingoInterval M3(4);
    M3.fullName(buf, sizeof(buf));
    CHECK(strcmp(buf, "Major Third") == 0, "M3 fullName=Major Third");

    M3.fullNamePt(buf, sizeof(buf));
    CHECK(strcmp(buf, "Terca Maior") == 0, "M3 fullNamePt=Terca Maior");

    // Operators
    GingoInterval sum = m3 + p5;
    CHECK(sum.semitones() == 10, "m3 + P5 = 10st");

    GingoInterval diff = p5 - m3;
    CHECK(diff.semitones() == 4, "P5 - m3 = 4st");

    GingoInterval underflow = m3 - p5;
    CHECK(underflow.semitones() == 0, "m3 - P5 = 0st (floor)");

    // Sum cap at 23
    GingoInterval big1((uint8_t)20);
    GingoInterval big2((uint8_t)10);
    GingoInterval capped = big1 + big2;
    CHECK(capped.semitones() == 23, "20 + 10 capped at 23");
}

// =====================================================================
// Chord
// =====================================================================

void testChord() {
    printf("\n=== GingoChord ===\n");

    GingoChord cMaj("CM");
    CHECK(strcmp(cMaj.name(), "CM") == 0, "CM name");
    CHECK(strcmp(cMaj.root().name(), "C") == 0, "CM root=C");
    CHECK(cMaj.size() == 3, "CM size=3");

    GingoNote notes[7];
    uint8_t n = cMaj.notes(notes, 7);
    CHECK(n == 3, "CM notes count=3");
    CHECK(strcmp(notes[0].name(), "C") == 0, "CM note[0]=C");
    CHECK(strcmp(notes[1].name(), "E") == 0, "CM note[1]=E");
    CHECK(strcmp(notes[2].name(), "G") == 0, "CM note[2]=G");

    GingoChord dm7("Dm7");
    CHECK(dm7.size() == 4, "Dm7 size=4");
    n = dm7.notes(notes, 7);
    CHECK(strcmp(notes[0].name(), "D") == 0, "Dm7 note[0]=D");
    CHECK(strcmp(notes[1].name(), "F") == 0, "Dm7 note[1]=F");

    CHECK(dm7.contains(GingoNote("F")), "Dm7 contains F");
    CHECK(!dm7.contains(GingoNote("F#")), "Dm7 !contains F#");

    // Transpose
    GingoChord transposed = cMaj.transpose(5);
    CHECK(strcmp(transposed.root().name(), "F") == 0, "CM+5 root=F");

    // Identify
    GingoNote testNotes[3] = {GingoNote("C"), GingoNote("E"), GingoNote("G")};
    char identified[16];
    bool found = GingoChord::identify(testNotes, 3, identified, sizeof(identified));
    CHECK(found, "identify [C,E,G] found");
    if (found) {
        printf("         identified as: %s\n", identified);
    }
}

void testChordIntervals() {
    printf("\n=== GingoChord intervals() ===\n");

    GingoChord cMaj("CM");
    GingoInterval ivs[7];
    uint8_t n = cMaj.intervals(ivs, 7);
    CHECK(n == 3, "CM intervals count=3");
    CHECK(ivs[0].semitones() == 0, "CM interval[0]=P1 (0st)");
    CHECK(ivs[1].semitones() == 4, "CM interval[1]=M3 (4st)");
    CHECK(ivs[2].semitones() == 7, "CM interval[2]=P5 (7st)");

    GingoChord dm7("Dm7");
    n = dm7.intervals(ivs, 7);
    CHECK(n == 4, "Dm7 intervals count=4");
    CHECK(ivs[3].semitones() == 10, "Dm7 interval[3]=m7 (10st)");

    char buf[8];
    ivs[1].label(buf, sizeof(buf));
    printf("         CM interval[1] label: %s\n", buf);
}

// =====================================================================
// Scale
// =====================================================================

void testScale() {
    printf("\n=== GingoScale ===\n");

    GingoScale cMaj("C", SCALE_MAJOR);
    CHECK(strcmp(cMaj.tonic().name(), "C") == 0, "C Major tonic=C");
    CHECK(cMaj.size() == 7, "C Major size=7");

    GingoNote notes[12];
    uint8_t n = cMaj.notes(notes, 12);
    CHECK(n == 7, "C Major notes count=7");
    printf("         C Major notes: ");
    for (uint8_t i = 0; i < n; i++) printf("%s ", notes[i].name());
    printf("\n");

    CHECK(strcmp(notes[0].name(), "C") == 0, "C Major[0]=C");
    CHECK(strcmp(notes[4].name(), "G") == 0, "C Major[4]=G");

    // Degree
    CHECK(strcmp(cMaj.degree(5).name(), "G") == 0, "C Major degree(5)=G");

    // Contains
    CHECK(cMaj.contains(GingoNote("F")), "C Major contains F");
    CHECK(!cMaj.contains(GingoNote("F#")), "C Major !contains F#");

    // Mode
    GingoScale dorian = cMaj.mode(2);
    n = dorian.notes(notes, 12);
    printf("         D Dorian notes: ");
    for (uint8_t i = 0; i < n; i++) printf("%s ", notes[i].name());
    printf("\n");
    CHECK(strcmp(dorian.tonic().name(), "D") == 0, "Dorian tonic=D");

    char modeBuf[20];
    CHECK(strcmp(dorian.modeName(modeBuf, sizeof(modeBuf)), "Dorian") == 0, "Dorian modeName");

    // Quality
    CHECK(strcmp(cMaj.quality(), "major") == 0, "C Major quality=major");

    // Pentatonic
    GingoScale penta = cMaj.pentatonic();
    n = penta.notes(notes, 12);
    printf("         C Penta notes: ");
    for (uint8_t i = 0; i < n; i++) printf("%s ", notes[i].name());
    printf("\n");
    CHECK(n == 5, "C Penta size=5");

    // By name
    GingoScale blues("A", "blues");
    n = blues.notes(notes, 12);
    printf("         A Blues notes: ");
    for (uint8_t i = 0; i < n; i++) printf("%s ", notes[i].name());
    printf("\n");
}

void testScaleExtended() {
    printf("\n=== GingoScale (extended) ===\n");

    GingoScale cMaj("C", SCALE_MAJOR);

    // Signature
    CHECK(cMaj.signature() == 0, "C Major signature=0");

    GingoScale gMaj("G", SCALE_MAJOR);
    CHECK(gMaj.signature() == 1, "G Major signature=1");

    GingoScale fMaj("F", SCALE_MAJOR);
    CHECK(fMaj.signature() == -1, "F Major signature=-1");

    // DegreeOf
    CHECK(cMaj.degreeOf(GingoNote("C")) == 1, "C Major degreeOf(C)=1");
    CHECK(cMaj.degreeOf(GingoNote("G")) == 5, "C Major degreeOf(G)=5");
    CHECK(cMaj.degreeOf(GingoNote("F#")) == 0, "C Major degreeOf(F#)=0");

    // Relative
    GingoScale rel = cMaj.relative();
    CHECK(strcmp(rel.tonic().name(), "A") == 0, "C Major relative tonic=A");
    CHECK(strcmp(rel.quality(), "minor") == 0, "C Major relative quality=minor");

    // Parallel
    GingoScale par = cMaj.parallel();
    CHECK(strcmp(par.tonic().name(), "C") == 0, "C Major parallel tonic=C");
    CHECK(strcmp(par.quality(), "minor") == 0, "C Major parallel quality=minor");

    // Brightness
    uint8_t br = cMaj.brightness();
    CHECK(br == 5, "C Ionian brightness=5");

    GingoScale dorian("D", "dorian");
    CHECK(dorian.brightness() == 3, "D Dorian brightness=3");

    // Mask
    uint16_t mask = cMaj.mask();
    CHECK((mask & 1) != 0, "C Major mask has bit 0 (root)");
    CHECK((mask & (1 << 6)) == 0, "C Major mask lacks bit 6 (tritone)");
    printf("         C Major mask: 0x%03X\n", mask);

    // ModeByName
    GingoScale lydian = cMaj.modeByName("lydian");
    CHECK(strcmp(lydian.quality(), "major") == 0, "lydian quality=major");
    CHECK(lydian.modeNumber() == 4, "lydian modeNumber=4");
}

// =====================================================================
// Field
// =====================================================================

void testField() {
    printf("\n=== GingoField ===\n");

    GingoField field("C", SCALE_MAJOR);
    CHECK(field.size() == 7, "C Major field size=7");

    GingoChord triads[7];
    uint8_t nt = field.chords(triads, 7);
    printf("         C Major triads: ");
    for (uint8_t i = 0; i < nt; i++) printf("%s ", triads[i].name());
    printf("\n");
    CHECK(nt == 7, "C Major triads count=7");

    GingoChord sevs[7];
    uint8_t ns = field.sevenths(sevs, 7);
    printf("         C Major 7ths:   ");
    for (uint8_t i = 0; i < ns; i++) printf("%s ", sevs[i].name());
    printf("\n");

    // Functions
    CHECK(field.function(1) == FUNC_TONIC, "I = tonic");
    CHECK(field.function(5) == FUNC_DOMINANT, "V = dominant");

    // Single degree
    GingoChord v = field.chord(5);
    printf("         V chord: %s\n", v.name());
}

void testFieldExtended() {
    printf("\n=== GingoField (extended) ===\n");

    GingoField field("C", SCALE_MAJOR);

    // Signature
    CHECK(field.signature() == 0, "C Major field signature=0");

    // FunctionOf by chord
    GingoChord gM("GM");
    CHECK(field.functionOf(gM) == FUNC_DOMINANT, "functionOf(GM)=dominant");

    GingoChord cM("CM");
    CHECK(field.functionOf(cM) == FUNC_TONIC, "functionOf(CM)=tonic");

    // FunctionOf by name
    CHECK(field.functionOf("Dm7") == FUNC_SUBDOMINANT, "functionOf('Dm7')=subdominant");

    // RoleOf
    char buf[20];
    field.roleOf(cM, buf, sizeof(buf));
    CHECK(strcmp(buf, "primary") == 0, "roleOf(CM)=primary");

    field.roleOf("Em", buf, sizeof(buf));
    CHECK(strcmp(buf, "transitive") == 0, "roleOf('Em')=transitive");
}

// =====================================================================
// Duration
// =====================================================================

void testDuration() {
    printf("\n=== GingoDuration ===\n");

    GingoDuration quarter("quarter");
    CHECK(quarter.numerator() == 1 && quarter.denominator() == 4, "quarter=1/4");
    CHECK(quarter.beats() == 1.0f, "quarter beats=1");

    GingoDuration whole("whole");
    CHECK(whole.numerator() == 1 && whole.denominator() == 1, "whole=1/1");
    CHECK(whole.beats() == 4.0f, "whole beats=4");

    GingoDuration eighth("eighth");
    CHECK(eighth.beats() == 0.5f, "eighth beats=0.5");

    // Dotted quarter
    GingoDuration dotQ("quarter", 1);
    CHECK(dotQ.beats() == 1.5f, "dotted quarter beats=1.5");

    // Triplet quarter
    GingoDuration tripQ("quarter", 0, 3);
    float tripBeats = tripQ.beats();
    CHECK(tripBeats > 0.66f && tripBeats < 0.67f, "triplet quarter beats~0.667");

    // Rational constructor
    GingoDuration rational(3, 8);
    CHECK(rational.numerator() == 3 && rational.denominator() == 8, "rational 3/8");

    char nameBuf[16];
    quarter.name(nameBuf, sizeof(nameBuf));
    CHECK(strcmp(nameBuf, "quarter") == 0, "quarter name()");
}

void testDurationExtended() {
    printf("\n=== GingoDuration (extended) ===\n");

    GingoDuration quarter("quarter");
    GingoDuration eighth("eighth");

    // operator+
    GingoDuration sum = quarter + eighth;
    float sumBeats = sum.beats();
    CHECK(sumBeats > 1.49f && sumBeats < 1.51f, "quarter + eighth = 1.5 beats");

    // operator<
    CHECK(eighth < quarter, "eighth < quarter");
    CHECK(!(quarter < eighth), "!(quarter < eighth)");
    CHECK(!(quarter < quarter), "!(quarter < quarter)");

    // operator>
    CHECK(quarter > eighth, "quarter > eighth");

    // operator<=
    CHECK(eighth <= quarter, "eighth <= quarter");
    CHECK(quarter <= quarter, "quarter <= quarter");

    // Sum of two quarters
    GingoDuration half = quarter + quarter;
    CHECK(half.beats() == 2.0f, "quarter + quarter = 2.0 beats");
}

// =====================================================================
// Tempo
// =====================================================================

void testTempo() {
    printf("\n=== GingoTempo ===\n");

    GingoTempo t120(120.0f);
    CHECK(t120.bpm() == 120.0f, "120 bpm");

    float ms = t120.msPerBeat();
    CHECK(ms == 500.0f, "120bpm msPerBeat=500");

    char buf[14];
    t120.marking(buf, sizeof(buf));
    printf("         120 BPM marking: %s\n", buf);

    // From marking
    GingoTempo adagio("Adagio");
    printf("         Adagio BPM: %.0f\n", adagio.bpm());
    CHECK(adagio.bpm() > 50 && adagio.bpm() < 80, "Adagio bpm in range");

    // Seconds
    GingoDuration quarter("quarter");
    float secs = t120.seconds(quarter);
    CHECK(secs == 0.5f, "120bpm quarter=0.5s");
}

// =====================================================================
// TimeSignature
// =====================================================================

void testTimeSig() {
    printf("\n=== GingoTimeSig ===\n");

    GingoTimeSig ts44(4, 4);
    CHECK(ts44.beatsPerBar() == 4, "4/4 beats=4");
    CHECK(ts44.beatUnit() == 4, "4/4 unit=4");
    CHECK(!ts44.isCompound(), "4/4 not compound");

    char buf[16];
    ts44.commonName(buf, sizeof(buf));
    CHECK(strcmp(buf, "common time") == 0, "4/4 common time");

    ts44.toString(buf, sizeof(buf));
    CHECK(strcmp(buf, "4/4") == 0, "4/4 toString");

    GingoTimeSig ts68(6, 8);
    CHECK(ts68.isCompound(), "6/8 compound");

    GingoTimeSig ts22(2, 2);
    ts22.commonName(buf, sizeof(buf));
    CHECK(strcmp(buf, "cut time") == 0, "2/2 cut time");

    // Bar duration
    GingoDuration bar44 = ts44.barDuration();
    CHECK(bar44.numerator() == 4 && bar44.denominator() == 4, "4/4 bar=4/4");
    CHECK(bar44.beats() == 4.0f, "4/4 bar beats=4");

    GingoDuration bar68 = ts68.barDuration();
    CHECK(bar68.numerator() == 6 && bar68.denominator() == 8, "6/8 bar=6/8");
    float b68 = bar68.beats();
    CHECK(b68 == 3.0f, "6/8 bar beats=3");

    // Classification
    ts44.classification(buf, sizeof(buf));
    CHECK(strcmp(buf, "simple") == 0, "4/4 classification=simple");

    ts68.classification(buf, sizeof(buf));
    CHECK(strcmp(buf, "compound") == 0, "6/8 classification=compound");
}

// =====================================================================
// Event (Tier 3)
// =====================================================================

void testEvent() {
    printf("\n=== GingoEvent ===\n");

    // Note event
    GingoEvent ne = GingoEvent::noteEvent(GingoNote("C"), GingoDuration("quarter"), 4);
    CHECK(ne.type() == EVENT_NOTE, "noteEvent type=NOTE");
    CHECK(strcmp(ne.note().name(), "C") == 0, "noteEvent note=C");
    CHECK(ne.octave() == 4, "noteEvent octave=4");
    CHECK(ne.midiNumber() == 60, "noteEvent midi=60");

    float freq = ne.frequency();
    CHECK(freq > 260.0f && freq < 263.0f, "noteEvent freq~261.6 (C4)");

    // Chord event
    GingoEvent ce = GingoEvent::chordEvent(GingoChord("CM"), GingoDuration("half"), 3);
    CHECK(ce.type() == EVENT_CHORD, "chordEvent type=CHORD");
    CHECK(strcmp(ce.chord().name(), "CM") == 0, "chordEvent chord=CM");
    CHECK(ce.octave() == 3, "chordEvent octave=3");

    // Rest event
    GingoEvent re = GingoEvent::rest(GingoDuration("whole"));
    CHECK(re.type() == EVENT_REST, "rest type=REST");
    CHECK(re.midiNumber() == 0, "rest midi=0");

    // Transpose
    GingoEvent transposed = ne.transpose(7);
    CHECK(strcmp(transposed.note().name(), "G") == 0, "noteEvent+7 = G");
    CHECK(transposed.midiNumber() == 67, "noteEvent+7 midi=67");
}

// =====================================================================
// Sequence (Tier 3)
// =====================================================================

void testSequence() {
    printf("\n=== GingoSequence ===\n");

    GingoSequence seq(GingoTempo(120), GingoTimeSig(4, 4));
    CHECK(seq.empty(), "new sequence is empty");
    CHECK(seq.size() == 0, "new sequence size=0");

    // Add events
    seq.add(GingoEvent::noteEvent(GingoNote("C"), GingoDuration("quarter"), 4));
    seq.add(GingoEvent::noteEvent(GingoNote("E"), GingoDuration("quarter"), 4));
    seq.add(GingoEvent::rest(GingoDuration("half")));
    CHECK(seq.size() == 3, "sequence size=3");
    CHECK(!seq.empty(), "sequence not empty");

    // Total beats
    float beats = seq.totalBeats();
    CHECK(beats == 4.0f, "totalBeats=4.0 (q+q+h)");

    // Total seconds (120 BPM, 4 beats = 2 seconds)
    float secs = seq.totalSeconds();
    CHECK(secs > 1.99f && secs < 2.01f, "totalSeconds~2.0");

    // Bar count
    float bars = seq.barCount();
    CHECK(bars > 0.99f && bars < 1.01f, "barCount~1.0");

    // At
    const GingoEvent& e0 = seq.at(0);
    CHECK(e0.type() == EVENT_NOTE, "at(0) type=NOTE");
    CHECK(strcmp(e0.note().name(), "C") == 0, "at(0) note=C");

    // Remove
    seq.remove(1);
    CHECK(seq.size() == 2, "after remove size=2");

    // Transpose
    seq.transpose(5);
    CHECK(strcmp(seq.at(0).note().name(), "F") == 0, "after transpose(5) note=F");

    // Clear
    seq.clear();
    CHECK(seq.empty(), "after clear is empty");
}

// =====================================================================
// MIDI Conversion (Tier 1+3)
// =====================================================================

void testMIDI() {
    printf("\n=== MIDI Conversion ===\n");

    // GingoNote::fromMIDI
    GingoNote c4 = GingoNote::fromMIDI(60);
    CHECK(c4.semitone() == 0, "fromMIDI(60) semitone=0");

    GingoNote a4 = GingoNote::fromMIDI(69);
    CHECK(a4.semitone() == 9, "fromMIDI(69) semitone=9");

    // GingoNote::octaveFromMIDI
    int8_t octave60 = GingoNote::octaveFromMIDI(60);
    CHECK(octave60 == 4, "octaveFromMIDI(60)=4");

    int8_t octave69 = GingoNote::octaveFromMIDI(69);
    CHECK(octave69 == 4, "octaveFromMIDI(69)=4");

    int8_t octave0 = GingoNote::octaveFromMIDI(12);
    CHECK(octave0 == 0, "octaveFromMIDI(12)=0");

    // Roundtrip: midiNumber -> fromMIDI -> midiNumber
    GingoNote cTest("C");
    uint8_t midiOrig = cTest.midiNumber(4);  // 60
    GingoNote cFromMidi = GingoNote::fromMIDI(midiOrig);
    uint8_t midiRoundtrip = cFromMidi.midiNumber(4);
    CHECK(midiOrig == midiRoundtrip, "C4: MIDI roundtrip");

    // GingoEvent::fromMIDI
    GingoEvent e60 = GingoEvent::fromMIDI(60, GingoDuration("quarter"));
    CHECK(e60.type() == EVENT_NOTE, "fromMIDI event type=NOTE");
    CHECK(e60.midiNumber() == 60, "fromMIDI event midi=60");
    CHECK(e60.octave() == 4, "fromMIDI event octave=4");

    // GingoEvent::toMIDI (uses internal velocity and channel)
    uint8_t midiBuffer[6];
    uint8_t written = e60.toMIDI(midiBuffer);
    CHECK(written == 6, "noteEvent toMIDI writes 6 bytes");
    CHECK(midiBuffer[0] == 0x90, "NoteOn status=0x90");
    CHECK(midiBuffer[1] == 60, "NoteOn note=60");
    CHECK(midiBuffer[2] == 100, "NoteOn velocity=100");
    CHECK(midiBuffer[3] == 0x80, "NoteOff status=0x80");
    CHECK(midiBuffer[4] == 60, "NoteOff note=60");
    CHECK(midiBuffer[5] == 0, "NoteOff velocity=0");

    // Rest event toMIDI (should return 0)
    GingoEvent rest = GingoEvent::rest(GingoDuration("quarter"));
    uint8_t restWritten = rest.toMIDI(midiBuffer);
    CHECK(restWritten == 0, "rest toMIDI writes 0 bytes");

    // Test velocity and channel customization
    GingoEvent eVel = GingoEvent::noteEvent(GingoNote("C"), GingoDuration("quarter"), 4, 64, 2);
    CHECK(eVel.velocity() == 64, "custom velocity=64");
    CHECK(eVel.midiChannel() == 2, "custom channel=2");
    eVel.toMIDI(midiBuffer);
    CHECK(midiBuffer[0] == 0x91, "channel 2 = 0x90 | 1 = 0x91");
    CHECK(midiBuffer[2] == 64, "velocity=64");

    // Test setVelocity/setMidiChannel
    GingoEvent eMod = GingoEvent::noteEvent(GingoNote("E"), GingoDuration("eighth"), 4);
    eMod.setVelocity(127);
    eMod.setMidiChannel(16);
    CHECK(eMod.velocity() == 127, "setVelocity(127)");
    CHECK(eMod.midiChannel() == 16, "setMidiChannel(16)");
    eMod.toMIDI(midiBuffer);
    CHECK(midiBuffer[0] == 0x9F, "channel 16 = 0x90 | 15 = 0x9F");
    CHECK(midiBuffer[2] == 127, "velocity=127");

    // GingoSequence::toMIDI
    GingoSequence seq(GingoTempo(120), GingoTimeSig(4, 4));
    seq.add(GingoEvent::noteEvent(GingoNote("C"), GingoDuration("quarter"), 4));
    seq.add(GingoEvent::noteEvent(GingoNote("E"), GingoDuration("quarter"), 4));
    seq.add(GingoEvent::rest(GingoDuration("half")));

    uint8_t seqBuffer[32];
    uint16_t seqWritten = seq.toMIDI(seqBuffer, sizeof(seqBuffer), 1);
    CHECK(seqWritten == 12, "sequence with 2 notes toMIDI writes 12 bytes (6+6)");

    // Check specific bytes from first event (C4)
    CHECK(seqBuffer[0] == 0x90, "seq[0] NoteOn status");
    CHECK(seqBuffer[1] == 60, "seq[1] C4 note");
    CHECK(seqBuffer[3] == 0x80, "seq[3] NoteOff status");

    // Check second event (E4 = 64)
    CHECK(seqBuffer[6] == 0x90, "seq[6] second NoteOn");
    CHECK(seqBuffer[7] == 64, "seq[7] E4 note");
}

// =====================================================================
// Fretboard
// =====================================================================

void testFretboard() {
    printf("\n=== GingoFretboard ===\n");

    // Violao (guitar)
    GingoFretboard guitar = GingoFretboard::violao();
    CHECK(guitar.numStrings() == 6, "violao numStrings=6");
    CHECK(guitar.numFrets() == 19, "violao numFrets=19");
    CHECK(strcmp(guitar.name(), "Violao") == 0, "violao name");

    // Open string MIDI
    CHECK(guitar.openMidi(0) == 40, "open E2 = MIDI 40");
    CHECK(guitar.openMidi(5) == 64, "open E4 = MIDI 64");

    // Note at position
    GingoNote n = guitar.noteAt(0, 5);
    CHECK(strcmp(n.name(), "A") == 0, "string 0 fret 5 = A");

    n = guitar.noteAt(1, 0);
    CHECK(strcmp(n.name(), "A") == 0, "string 1 open = A");

    // MIDI at position
    CHECK(guitar.midiAt(0, 0) == 40, "midiAt(0,0)=40 (E2)");
    CHECK(guitar.midiAt(0, 12) == 52, "midiAt(0,12)=52 (E3)");

    // Position struct
    GingoFretPos pos = guitar.position(0, 5);
    CHECK(pos.string == 0, "pos.string=0");
    CHECK(pos.fret == 5, "pos.fret=5");
    CHECK(pos.midi == 45, "pos.midi=45");

    // Find positions of a note
    GingoFretPos positions[48];
    uint8_t count = guitar.positions(GingoNote("E"), positions, 48);
    CHECK(count > 0, "E positions found on guitar");
    printf("         E on guitar: %d positions\n", count);

    // Scale positions
    GingoScale cMaj("C", SCALE_MAJOR);
    count = guitar.scalePositions(cMaj, positions, 48, 0, 4);
    CHECK(count > 0, "C Major positions (frets 0-4)");
    printf("         C Major (frets 0-4): %d positions\n", count);

    // Fingering
    GingoFingering fg;
    bool found = guitar.fingering(GingoChord("CM"), 0, fg);
    CHECK(found, "CM fingering found at pos 0");
    if (found) {
        printf("         CM fingering: score=%d notes=%d\n", fg.score, fg.numNotes);
    }

    // Multiple fingerings
    GingoFingering fgs[5];
    uint8_t nfg = guitar.fingerings(GingoChord("CM"), fgs, 5);
    CHECK(nfg > 0, "CM has at least 1 fingering");
    printf("         CM fingerings found: %d\n", nfg);

    // Identify from fret positions
    uint8_t frets[6] = { 255, 0, 2, 2, 1, 0 };  // x02210 = Am
    char chordName[16];
    bool identified = guitar.identify(frets, 6, chordName, sizeof(chordName));
    if (identified) {
        printf("         x02210 identified as: %s\n", chordName);
    }

    // Capo
    GingoFretboard capo2 = guitar.capo(2);
    CHECK(capo2.openMidi(0) == 42, "capo 2 open E2 = MIDI 42 (F#2)");
    n = capo2.noteAt(0, 0);
    CHECK(strcmp(n.name(), "F#") == 0, "capo 2 string 0 open = F#");

    // Cavaquinho
    GingoFretboard cav = GingoFretboard::cavaquinho();
    CHECK(cav.numStrings() == 4, "cavaquinho numStrings=4");
    CHECK(strcmp(cav.name(), "Cavaquinho") == 0, "cavaquinho name");

    // Ukulele
    GingoFretboard uke = GingoFretboard::ukulele();
    CHECK(uke.numStrings() == 4, "ukulele numStrings=4");
    CHECK(strcmp(uke.name(), "Ukulele") == 0, "ukulele name");
}

// =====================================================================
// Field deduce
// =====================================================================

void testFieldDeduce() {
    printf("\n=== GingoField::deduce ===\n");

    // Deduce from chords — full C major field
    {
        const char* items[] = {"CM", "Dm", "Em", "FM", "G7", "Am"};
        FieldMatch results[10];
        uint8_t n = GingoField::deduce(items, 6, results, 10);
        CHECK(n > 0, "deduce chords returns results");
        // Top result should be C major (6/6 match)
        CHECK(results[0].matched == 6, "C major chords: matched=6");
        CHECK(strcmp(results[0].tonicName, "C") == 0, "C major chords: tonic=C");
        CHECK(results[0].scaleType == SCALE_MAJOR, "C major chords: type=major");
    }

    // Deduce from partial chords — Am, Dm, Em -> C major (vi, ii, iii)
    {
        const char* items[] = {"Am", "Dm", "Em"};
        FieldMatch results[10];
        uint8_t n = GingoField::deduce(items, 3, results, 10);
        CHECK(n > 0, "deduce Am/Dm/Em returns results");
        // Should find C major with 3 matches
        bool found = false;
        for (uint8_t i = 0; i < n; i++) {
            if (strcmp(results[i].tonicName, "C") == 0 &&
                results[i].scaleType == SCALE_MAJOR && results[i].matched == 3) {
                found = true;
                break;
            }
        }
        CHECK(found, "Am/Dm/Em: C major with 3 matches");
    }

    // Deduce from notes
    {
        const char* items[] = {"C", "E", "G", "A"};
        FieldMatch results[10];
        uint8_t n = GingoField::deduce(items, 4, results, 10);
        CHECK(n > 0, "deduce notes returns results");
        CHECK(results[0].matched == 4, "C/E/G/A: top match=4");
        CHECK(strcmp(results[0].tonicName, "C") == 0, "C/E/G/A: tonic=C");
    }

    // Deduce ordering: higher match count first
    {
        const char* items[] = {"CM", "FM"};
        FieldMatch results[10];
        uint8_t n = GingoField::deduce(items, 2, results, 10);
        CHECK(n >= 2, "deduce CM/FM returns multiple results");
        CHECK(results[0].matched >= results[1].matched,
              "results sorted by matched desc");
    }

    // Roles are populated
    {
        const char* items[] = {"CM", "G7"};
        FieldMatch results[5];
        uint8_t n = GingoField::deduce(items, 2, results, 5);
        CHECK(n > 0, "deduce CM/G7 returns results");
        // Find the C major result
        for (uint8_t i = 0; i < n; i++) {
            if (strcmp(results[i].tonicName, "C") == 0 &&
                results[i].scaleType == SCALE_MAJOR) {
                CHECK(results[i].roleCount == 2, "CM/G7 in C major: 2 roles");
                CHECK(strcmp(results[i].roles[0], "I") == 0, "CM role = I");
                CHECK(strcmp(results[i].roles[1], "V7") == 0, "G7 role = V7");
                break;
            }
        }
    }
}

// =====================================================================
// Tree
// =====================================================================

void testTree() {
    printf("\n=== GingoTree ===\n");

    // harmonic_tree, C major
    GingoTree ht("C", SCALE_MAJOR, 0);
    CHECK(ht.traditionId() == 0, "harmonic_tree id=0");
    CHECK(ht.context() == 0, "C major context=0 (major)");

    char tradName[20];
    ht.traditionName(tradName, sizeof(tradName));
    CHECK(strcmp(tradName, "harmonic_tree") == 0, "tradition name=harmonic_tree");

    // Valid transitions
    CHECK(ht.isValid("I", "V7") == true, "I→V7 valid in HT major");
    CHECK(ht.isValid("I", "VIm") == true, "I→VIm valid in HT major");
    CHECK(ht.isValid("V7", "I") == true, "V7→I valid in HT major");
    CHECK(ht.isValid("IIm", "V7") == true, "IIm→V7 valid in HT major");

    // Invalid transitions
    CHECK(ht.isValid("I", "IVm") == false, "I→IVm invalid in HT major");
    CHECK(ht.isValid("V7", "IIm") == false, "V7→IIm invalid in HT major");

    // Sequence validation
    {
        const char* seq[] = {"I", "V7", "I"};
        CHECK(ht.isValidSequence(seq, 3) == true, "I-V7-I valid sequence");
    }
    {
        const char* seq[] = {"IIm", "V7", "I"};
        CHECK(ht.isValidSequence(seq, 3) == true, "IIm-V7-I valid sequence");
    }
    {
        const char* seq[] = {"I", "IVm", "I"};
        CHECK(ht.isValidSequence(seq, 3) == false, "I-IVm-I invalid sequence");
    }

    // Count valid transitions
    {
        const char* seq[] = {"I", "V7", "I"};
        CHECK(ht.countValidTransitions(seq, 3) == 2, "I-V7-I: 2 valid transitions");
    }

    // Neighbors
    {
        const char* neigh[16];
        uint8_t n = ht.neighbors("I", neigh, 16);
        CHECK(n > 0, "I has neighbors in HT major");
        // Check that V7 is among them
        bool hasV7 = false;
        for (uint8_t i = 0; i < n; i++) {
            char buf[24];
            data::readPgmStr(buf, neigh[i], sizeof(buf));
            if (strcmp(buf, "V7") == 0) hasV7 = true;
        }
        CHECK(hasV7, "V7 is a neighbor of I");
    }

    // Resolve branch to chord
    {
        char chord[16];
        CHECK(ht.resolve("I", chord, sizeof(chord)) == true, "resolve I");
        CHECK(strcmp(chord, "CM") == 0, "I in C major = CM");

        CHECK(ht.resolve("V7", chord, sizeof(chord)) == true, "resolve V7");
        CHECK(strcmp(chord, "G7") == 0, "V7 in C major = G7");

        CHECK(ht.resolve("IIm", chord, sizeof(chord)) == true, "resolve IIm");
        CHECK(strcmp(chord, "Dm") == 0, "IIm in C major = Dm");

        CHECK(ht.resolve("VIm", chord, sizeof(chord)) == true, "resolve VIm");
        CHECK(strcmp(chord, "Am") == 0, "VIm in C major = Am");

        CHECK(ht.resolve("IV", chord, sizeof(chord)) == true, "resolve IV");
        CHECK(strcmp(chord, "FM") == 0, "IV in C major = FM");
    }

    // Resolve secondary dominant
    {
        char chord[16];
        CHECK(ht.resolve("V7 / IIm", chord, sizeof(chord)) == true, "resolve V7/IIm");
        CHECK(strcmp(chord, "A7") == 0, "V7/IIm in C = A7");
    }

    // Resolve diminished
    {
        char chord[16];
        CHECK(ht.resolve("#Idim", chord, sizeof(chord)) == true, "resolve #Idim");
        CHECK(strcmp(chord, "C#dim") == 0, "#Idim in C = C#dim");
    }

    // Jazz tree
    GingoTree jz("C", SCALE_MAJOR, 1);
    CHECK(jz.traditionId() == 1, "jazz id=1");
    CHECK(jz.isValid("IIm", "V7") == true, "IIm→V7 valid in jazz");
    CHECK(jz.isValid("V7", "I") == true, "V7→I valid in jazz");
    CHECK(jz.isValid("IVm", "bVII") == true, "IVm→bVII valid in jazz (backdoor)");
    CHECK(jz.isValid("bVII", "I") == true, "bVII→I valid in jazz (backdoor)");

    // Minor context
    GingoTree htMin("A", SCALE_NATURAL_MINOR, 0);
    CHECK(htMin.context() == 1, "A minor context=1 (minor)");
    CHECK(htMin.isValid("Im", "V7 / I") == true, "Im→V7/I valid in HT minor");
    CHECK(htMin.isValid("V7 / I", "Im") == true, "V7/I→Im valid in HT minor");
}

// =====================================================================
// Progression
// =====================================================================

void testProgression() {
    printf("\n=== GingoProgression ===\n");

    GingoProgression p("C", SCALE_MAJOR);

    // identify: ii-V-I → jazz
    {
        const char* seq[] = {"IIm", "V7", "I"};
        ProgressionMatch m;
        bool found = p.identify(seq, 3, &m);
        CHECK(found, "identify IIm-V7-I found");
        CHECK(strcmp(m.schema, "ii-V-I") == 0, "identify IIm-V7-I → ii-V-I schema");
        CHECK(m.scoreNum == 100, "ii-V-I exact match score=100");
    }

    // identify: I-V7-I → direct (harmonic_tree)
    {
        const char* seq[] = {"I", "V7", "I"};
        ProgressionMatch m;
        bool found = p.identify(seq, 3, &m);
        CHECK(found, "identify I-V7-I found");
        CHECK(strcmp(m.schema, "direct") == 0, "identify I-V7-I → direct schema");
    }

    // deduce: returns multiple results
    {
        const char* seq[] = {"I", "V7", "I"};
        ProgressionMatch results[10];
        uint8_t n = p.deduce(seq, 3, results, 10);
        CHECK(n > 0, "deduce I-V7-I returns results");
        // Top result should have high score
        CHECK(results[0].scoreNum >= 50, "deduce top score >= 50");
    }

    // deduce: IIm-V7 (prefix of ii-V-I)
    {
        const char* seq[] = {"IIm", "V7"};
        ProgressionMatch results[10];
        uint8_t n = p.deduce(seq, 2, results, 10);
        CHECK(n > 0, "deduce IIm-V7 returns results");
        // Should match as prefix of ii-V-I
        bool foundIiVI = false;
        for (uint8_t i = 0; i < n; i++) {
            if (strcmp(results[i].schema, "ii-V-I") == 0) {
                foundIiVI = true;
                break;
            }
        }
        CHECK(foundIiVI, "IIm-V7 matches as prefix of ii-V-I");
    }

    // predict: after IIm-V7, should suggest I with high confidence
    {
        const char* seq[] = {"IIm", "V7"};
        ProgressionRoute routes[16];
        uint8_t n = p.predict(seq, 2, routes, 16);
        CHECK(n > 0, "predict after IIm-V7 returns routes");
        // "I" should be among predictions
        bool foundI = false;
        for (uint8_t i = 0; i < n; i++) {
            if (strcmp(routes[i].next, "I") == 0) {
                foundI = true;
                CHECK(routes[i].confidenceNum > 30, "I prediction confidence > baseline");
                break;
            }
        }
        CHECK(foundI, "I predicted after IIm-V7");
    }

    // predict: after I, multiple options
    {
        const char* seq[] = {"I"};
        ProgressionRoute routes[32];
        uint8_t n = p.predict(seq, 1, routes, 32);
        CHECK(n >= 2, "predict after I returns multiple options");
    }

    // Minor progression
    {
        GingoProgression pm("A", SCALE_NATURAL_MINOR);
        const char* seq[] = {"Im", "V7 / I", "Im"};
        ProgressionMatch m;
        bool found = pm.identify(seq, 3, &m);
        CHECK(found, "identify Im-V7/I-Im found in minor");
        CHECK(strcmp(m.schema, "minor_descending") == 0, "minor_descending schema");
    }
}

// =====================================================================
// Main
// =====================================================================

int main() {
    printf("Gingoduino Native Test\n");
    printf("======================\n");

    testNote();
    testInterval();
    testIntervalExtended();
    testChord();
    testChordIntervals();
    testScale();
    testScaleExtended();
    testField();
    testFieldExtended();
    testDuration();
    testDurationExtended();
    testTempo();
    testTimeSig();
    testEvent();
    testSequence();
    testMIDI();
    testFretboard();
    testFieldDeduce();
    testTree();
    testProgression();

    printf("\n======================\n");
    printf("Tests: %d  Passed: %d  Failed: %d\n", tests, tests - failures, failures);
    return failures > 0 ? 1 : 0;
}
