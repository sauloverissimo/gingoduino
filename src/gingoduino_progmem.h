// Gingoduino — Music Theory Library for Embedded Systems
// All music theory lookup data stored in PROGMEM (flash memory).
//
// SPDX-License-Identifier: MIT

#ifndef GINGODUINO_PROGMEM_H
#define GINGODUINO_PROGMEM_H

#include "gingoduino_config.h"

namespace gingoduino {
namespace data {

// ===================================================================
// 1. CHROMATIC SCALE — 12 pitch classes
// ===================================================================

static const char CHROMATIC_NAMES[12][3] PROGMEM = {
    "C", "C#", "D", "D#", "E", "F",
    "F#", "G", "G#", "A", "A#", "B"
};

// Base letter (sound) for each chromatic index
static const char CHROMATIC_SOUND[12] PROGMEM = {
    'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B'
};

// Circle of fifths order
static const uint8_t FIFTHS_ORDER[12] PROGMEM = {
    0, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10, 5
    // C, G, D, A, E, B, F#, C#, G#, D#, A#, F
};

// ===================================================================
// 2. ENHARMONIC MAP — sorted for binary search (ASCII only)
// ===================================================================

struct EnharmonicEntry {
    char input[5];    // max "C##" + null
    char output[3];   // max "F#" + null
};

// Sorted alphabetically by input for binary search.
// Unicode entries (♭) omitted — embedded uses ASCII notation.
static const EnharmonicEntry ENHARMONIC_MAP[] PROGMEM = {
    {"#B",  "C" }, {"#E",  "F" },
    {"##A", "B" }, {"##B", "C#"}, {"##C", "D" }, {"##D", "E" },
    {"##E", "F#"}, {"##F", "G" }, {"##G", "A" },
    {"A##", "B" }, {"Ab",  "G#"}, {"Abb", "G" },
    {"B##", "C#"}, {"B#",  "C" }, {"Bb",  "A#"}, {"Bbb", "A" },
    {"C##", "D" }, {"Cb",  "B" }, {"Cbb", "B" },
    {"D##", "E" }, {"Db",  "C#"}, {"Dbb", "C" },
    {"E##", "F#"}, {"E#",  "F" }, {"Eb",  "D#"}, {"Ebb", "D" },
    {"F##", "G" }, {"Fb",  "E" }, {"Fbb", "E" },
    {"G##", "A" }, {"Gb",  "F#"}, {"Gbb", "F" },
    {"bA",  "G#"}, {"bB",  "A#"}, {"bC",  "B" },
    {"bD",  "C#"}, {"bE",  "D#"}, {"bF",  "E" }, {"bG",  "F#"},
    {"bbA", "G" }, {"bbB", "A" }, {"bbC", "B" },
    {"bbD", "C" }, {"bbE", "D" }, {"bbF", "E" }, {"bbG", "F" },
};

static const uint8_t ENHARMONIC_MAP_SIZE = sizeof(ENHARMONIC_MAP) / sizeof(ENHARMONIC_MAP[0]);

// ===================================================================
// 3. INTERVAL TABLE — 24 intervals across two octaves
// ===================================================================

struct IntervalData {
    char    label[5];     // "P1", "5J", "#11", "b13"
    char    anglo[5];     // "P1", "P5", "d11", "mi13"
    uint8_t degree;       // diatonic degree 1-14
    uint8_t octave;       // 1 or 2
    // semitones = array index (0-23)
};

static const IntervalData INTERVAL_TABLE[24] PROGMEM = {
    // idx  label   anglo   degree octave
    /*  0*/ {"P1",  "P1",   1,  1},
    /*  1*/ {"2m",  "mi2",  2,  1},
    /*  2*/ {"2M",  "ma2",  2,  1},
    /*  3*/ {"3m",  "mi3",  3,  1},
    /*  4*/ {"3M",  "ma3",  3,  1},
    /*  5*/ {"4J",  "P4",   4,  1},
    /*  6*/ {"d5",  "d5",   5,  1},
    /*  7*/ {"5J",  "P5",   5,  1},
    /*  8*/ {"#5",  "mi6",  6,  1},
    /*  9*/ {"M6",  "ma6",  6,  1},
    /* 10*/ {"7m",  "mi7",  7,  1},
    /* 11*/ {"7M",  "ma7",  7,  1},
    /* 12*/ {"8J",  "P8",   8,  2},
    /* 13*/ {"b9",  "mi9",  9,  2},
    /* 14*/ {"9",   "ma9",  9,  2},
    /* 15*/ {"#9",  "mi10", 10, 2},
    /* 16*/ {"b11", "ma10", 10, 2},
    /* 17*/ {"11",  "P11",  11, 2},
    /* 18*/ {"#11", "d11",  11, 2},
    /* 19*/ {"5",   "P12",  12, 2},
    /* 20*/ {"b13", "mi13", 13, 2},
    /* 21*/ {"13",  "ma13", 13, 2},
    /* 22*/ {"#13", "mi14", 14, 2},
    /* 23*/ {"bI",  "ma14", 14, 2},
};

// ===================================================================
// 4. SCALE MASKS — 10 scale types x 24-bit bitmask
// ===================================================================
//
// Each uint32_t has bit N set if semitone position N is active.
// Bit 0 = P1, Bit 1 = 2m, ..., Bit 11 = 7M, Bit 12 = 8J, ...

static const uint32_t SCALE_MASKS[10] PROGMEM = {
    // Major:          P1 . 2M .  3M 4J .  5J .  M6 .  7M | .  .  9  .  .  11 .  .  .  13 .  .
    0b00000000001000100010101010110101UL,  // 0 Major
    // NatMinor:       P1 . 2M 3m .  4J .  5J #5 .  7m .  | .  .  9  .  .  11 .  .  b13 .  .  .
    0b00000000000101000010101011010101UL,  // 1 Natural minor
    // HarmMinor:      P1 . 2M 3m .  4J .  5J #5 .  .  7M | .  .  9  .  .  11 .  .  .  13 .  .
    0b00000000001000100010100011010101UL,  // 2 Harmonic minor
    // MelodicMinor:   P1 . 2M 3m .  4J .  5J .  M6 .  7M | .  .  9  .  .  11 .  .  .  13 .  .
    0b00000000001000100010101010010101UL,  // 3 Melodic minor
    // Diminished:     P1 . 2M 3m .  4J d5 .  #5 .  7m 7M | .  .  9  .  .  11 .  .  .  13 .  .
    0b00000000001000100010110001110101UL,  // 4 Diminished
    // HarmonicMajor:  P1 . 2M .  3M 4J .  5J #5 .  .  7M | .  .  9  .  .  11 .  .  .  13 .  .
    0b00000000001000100010100110110101UL,  // 5 Harmonic major
    // WholeTone:      P1 . 2M .  3M .  d5 .  #5 .  7m .  | .  .  9  .  .  .  #11 .  .  13 .  .
    0b00000000001001000000010101010101UL,  // 6 Whole tone
    // Augmented:      P1 .  .  3m 3M .  .  5J #5 .  .  7M | .  .  .  #9 .  .  .  5  .  .  #13 .
    0b00000000010010000010000110011001UL,  // 7 Augmented
    // Blues:          P1 .  .  3m .  4J d5 5J .  .  7m .  | .  .  .  #9 .  11 #11 .  .  .  #13 .
    0b00000000010011000010000011101001UL,  // 8 Blues
    // Chromatic:      all bits set
    0b00000000111111111111111111111111UL,  // 9 Chromatic
};

// Modality masks (diatonic filter and pentatonic filter)
// Applied as AND with scale mask to select active positions
static const uint32_t MODALITY_DIATONIC   PROGMEM = 0b00000000001101100010111111111111UL;
static const uint32_t MODALITY_PENTATONIC PROGMEM = 0b00000000001101100010110111101111UL;

// Scale size (number of notes in each parent scale)
static const uint8_t SCALE_SIZES[10] PROGMEM = {
    7, 7, 7, 7, 8, 7, 6, 6, 6, 12
};

// Scale type name strings
static const char STN_0[]  PROGMEM = "major";
static const char STN_1[]  PROGMEM = "natural minor";
static const char STN_2[]  PROGMEM = "harmonic minor";
static const char STN_3[]  PROGMEM = "melodic minor";
static const char STN_4[]  PROGMEM = "diminished";
static const char STN_5[]  PROGMEM = "harmonic major";
static const char STN_6[]  PROGMEM = "whole tone";
static const char STN_7[]  PROGMEM = "augmented";
static const char STN_8[]  PROGMEM = "blues";
static const char STN_9[]  PROGMEM = "chromatic";

static const char* const SCALE_TYPE_NAMES[10] PROGMEM = {
    STN_0, STN_1, STN_2, STN_3, STN_4,
    STN_5, STN_6, STN_7, STN_8, STN_9
};

// ===================================================================
// 5. CHORD FORMULAS — 42 types
// ===================================================================
//
// Each formula stores interval indices (into INTERVAL_TABLE).
// Since interval index = semitone count, these are also semitone offsets.

struct ChordFormula {
    uint8_t intervals[7];  // interval table indices (= semitone offsets)
    uint8_t count;         // how many intervals
};

static const ChordFormula CHORD_FORMULAS[42] PROGMEM = {
    /*  0 M       */ {{0, 4, 7, 0, 0, 0, 0}, 3},
    /*  1 7M      */ {{0, 4, 7, 11, 0, 0, 0}, 4},
    /*  2 6       */ {{0, 4, 7, 9, 0, 0, 0}, 4},
    /*  3 6(9)    */ {{0, 4, 7, 9, 14, 0, 0}, 5},
    /*  4 M9      */ {{0, 4, 7, 11, 14, 0, 0}, 5},
    /*  5 m       */ {{0, 3, 7, 0, 0, 0, 0}, 3},
    /*  6 m7      */ {{0, 3, 7, 10, 0, 0, 0}, 4},
    /*  7 m6      */ {{0, 3, 7, 9, 0, 0, 0}, 4},
    /*  8 m11     */ {{0, 3, 7, 10, 17, 0, 0}, 5},
    /*  9 mM7     */ {{0, 3, 7, 11, 0, 0, 0}, 4},
    /* 10 7       */ {{0, 4, 7, 10, 0, 0, 0}, 4},
    /* 11 9       */ {{0, 4, 7, 10, 14, 0, 0}, 5},
    /* 12 11      */ {{0, 4, 7, 10, 14, 17, 0}, 6},
    /* 13 dim     */ {{0, 3, 6, 0, 0, 0, 0}, 3},
    /* 14 dim7    */ {{0, 3, 6, 9, 0, 0, 0}, 4},
    /* 15 m7(b5)  */ {{0, 3, 6, 10, 0, 0, 0}, 4},
    /* 16 aug     */ {{0, 4, 8, 0, 0, 0, 0}, 3},
    /* 17 7#5     */ {{0, 4, 8, 10, 0, 0, 0}, 4},
    /* 18 7(b5)   */ {{0, 4, 6, 10, 0, 0, 0}, 4},
    /* 19 13      */ {{0, 4, 7, 10, 14, 17, 21}, 7},
    /* 20 13(#11) */ {{0, 4, 7, 10, 14, 18, 21}, 7},
    /* 21 7+5     */ {{0, 4, 8, 10, 0, 0, 0}, 4},
    /* 22 7+9     */ {{0, 4, 7, 10, 15, 0, 0}, 5},
    /* 23 7(b9)   */ {{0, 4, 7, 10, 13, 0, 0}, 5},
    /* 24 7(#11)  */ {{0, 4, 7, 10, 18, 0, 0}, 5},
    /* 25 5       */ {{0, 7, 0, 0, 0, 0, 0}, 2},
    /* 26 add9    */ {{0, 4, 7, 14, 0, 0, 0}, 4},
    /* 27 add2    */ {{0, 2, 4, 7, 0, 0, 0}, 4},
    /* 28 add11   */ {{0, 4, 7, 17, 0, 0, 0}, 4},
    /* 29 add4    */ {{0, 4, 5, 7, 0, 0, 0}, 4},
    /* 30 sus2    */ {{0, 2, 7, 0, 0, 0, 0}, 3},
    /* 31 sus4    */ {{0, 5, 7, 0, 0, 0, 0}, 3},
    /* 32 sus7    */ {{0, 5, 7, 10, 0, 0, 0}, 4},
    /* 33 sus9    */ {{0, 5, 7, 14, 0, 0, 0}, 4},
    /* 34 m13     */ {{0, 3, 7, 10, 14, 17, 21}, 7},
    /* 35 maj13   */ {{0, 4, 7, 11, 14, 18, 21}, 7},
    /* 36 sus     */ {{0, 5, 7, 0, 0, 0, 0}, 3},
    /* 37 m9      */ {{0, 3, 7, 10, 14, 0, 0}, 5},
    /* 38 M7#5    */ {{0, 4, 8, 11, 0, 0, 0}, 4},
    /* 39 m7(11)  */ {{0, 3, 7, 10, 17, 0, 0}, 5},
    /* 40 (b9)    */ {{0, 4, 7, 13, 0, 0, 0}, 4},
    /* 41 (b13)   */ {{0, 4, 7, 20, 0, 0, 0}, 4},
};

// ===================================================================
// 5b. CHORD TYPE ALIASES — sorted for binary search
// ===================================================================

struct ChordTypeAlias {
    char    name[10];      // chord type string
    uint8_t formulaIdx;    // index into CHORD_FORMULAS
};

// Sorted by strcmp order for binary search
static const ChordTypeAlias CHORD_TYPE_MAP[] PROGMEM = {
    {"(9)",     26},
    {"(b13)",   41},
    {"(b9)",    40},
    {"+",       16},
    {"+M7",     38},
    {"11",      12},
    {"13",      19},
    {"13(#11)", 20},
    {"5",       25},
    {"6",        2},
    {"6(9)",     3},
    {"7",       10},
    {"7#5",     17},
    {"7(#11)",  24},
    {"7(9)",    11},
    {"7(b5)",   18},
    {"7(b9)",   23},
    {"7+5",     21},
    {"7+9",     22},
    {"7/9",     11},
    {"7M",       1},
    {"7M(#5)",  38},
    {"M",        0},
    {"M13",     35},
    {"M6",       2},
    {"M7#5",    38},
    {"M9",       4},
    {"add11",   28},
    {"add2",    27},
    {"add4",    29},
    {"add9",    26},
    {"aug",     16},
    {"dim",     13},
    {"dim7",    14},
    {"dom7",    10},
    {"m",        5},
    {"m11",      8},
    {"m13",     34},
    {"m6",       7},
    {"m7",       6},
    {"m7(11)",  39},
    {"m7(b5)",  15},
    {"m7M",      9},
    {"m9",      37},
    {"mM7",      9},
    {"maj",      0},
    {"maj13",   35},
    {"maj7",     1},
    {"maj9",     4},
    {"mi",       5},
    {"min",      5},
    {"min7",     6},
    {"sus",     36},
    {"sus2",    30},
    {"sus4",    31},
    {"sus7",    32},
    {"sus9",    33},
};

static const uint8_t CHORD_TYPE_MAP_SIZE = sizeof(CHORD_TYPE_MAP) / sizeof(CHORD_TYPE_MAP[0]);

// ===================================================================
// 6. TEMPO MARKINGS
// ===================================================================

#if GINGODUINO_HAS_TEMPO

struct TempoMarking {
    char    name[14];
    uint8_t bpm_low;
    uint8_t bpm_high;
    uint8_t bpm_mid;  // typical BPM
};

static const TempoMarking TEMPO_MARKINGS[] PROGMEM = {
    {"Grave",        25,  45,  35},
    {"Largo",        40,  60,  50},
    {"Adagio",       55,  75,  60},
    {"Andante",      73,  108, 80},
    {"Moderato",     108, 120, 114},
    {"Allegretto",   112, 140, 120},
    {"Allegro",      120, 168, 140},
    {"Vivace",       140, 180, 160},
    {"Presto",       168, 200, 184},
    {"Prestissimo",  200, 240, 220},
};

static const uint8_t TEMPO_MARKINGS_SIZE = sizeof(TEMPO_MARKINGS) / sizeof(TEMPO_MARKINGS[0]);

#endif // GINGODUINO_HAS_TEMPO

// ===================================================================
// 7. DURATION NAMES
// ===================================================================

#if GINGODUINO_HAS_DURATION

struct DurationDef {
    char    name[16];
    uint8_t numerator;
    uint8_t denominator;
};

static const DurationDef DURATION_NAMES[] PROGMEM = {
    {"whole",          1,   1},
    {"half",           1,   2},
    {"quarter",        1,   4},
    {"eighth",         1,   8},
    {"sixteenth",      1,  16},
    {"thirty_second",  1,  32},
    {"sixty_fourth",   1,  64},
};

static const uint8_t DURATION_NAMES_SIZE = sizeof(DURATION_NAMES) / sizeof(DURATION_NAMES[0]);

#endif // GINGODUINO_HAS_DURATION

// ===================================================================
// 8. HARMONIC FUNCTION TABLE — function per degree (7 degrees)
// ===================================================================

#if GINGODUINO_HAS_FIELD

// Harmonic functions for Major scale degrees 1-7: T S D T S T D
static const uint8_t HARMONIC_FUNCTIONS_MAJOR[7] PROGMEM = {
    0, 1, 2, 0, 2, 0, 2  // T, S, D, T, D, T, D
};

// Role names (index by degree-1 for major)
static const char ROLE_0[] PROGMEM = "primary";
static const char ROLE_1[] PROGMEM = "primary";
static const char ROLE_2[] PROGMEM = "transitive";
static const char ROLE_3[] PROGMEM = "primary";
static const char ROLE_4[] PROGMEM = "primary";
static const char ROLE_5[] PROGMEM = "relative of I";
static const char ROLE_6[] PROGMEM = "transitive";

static const char* const ROLE_NAMES_MAJOR[7] PROGMEM = {
    ROLE_0, ROLE_1, ROLE_2, ROLE_3, ROLE_4, ROLE_5, ROLE_6
};

#endif // GINGODUINO_HAS_FIELD

// ===================================================================
// 9. MODE DATA (for Scale)
// ===================================================================

#if GINGODUINO_HAS_SCALE

// Mode names for the Major family (7 modes)
static const char MODE_MAJ_1[] PROGMEM = "Ionian";
static const char MODE_MAJ_2[] PROGMEM = "Dorian";
static const char MODE_MAJ_3[] PROGMEM = "Phrygian";
static const char MODE_MAJ_4[] PROGMEM = "Lydian";
static const char MODE_MAJ_5[] PROGMEM = "Mixolydian";
static const char MODE_MAJ_6[] PROGMEM = "Aeolian";
static const char MODE_MAJ_7[] PROGMEM = "Locrian";

static const char* const MODE_NAMES_MAJOR[7] PROGMEM = {
    MODE_MAJ_1, MODE_MAJ_2, MODE_MAJ_3, MODE_MAJ_4,
    MODE_MAJ_5, MODE_MAJ_6, MODE_MAJ_7
};

// Brightness values for Major modes (1=Locrian, 7=Lydian)
static const uint8_t MODE_BRIGHTNESS_MAJOR[7] PROGMEM = {
    5, 3, 1, 7, 6, 2, 0  // Ionian=5, Dorian=3, Phrygian=1, Lydian=7, ...
};

// Mode names for Harmonic Minor (7 modes)
static const char MODE_HM_1[] PROGMEM = "Harmonic Minor";
static const char MODE_HM_2[] PROGMEM = "Locrian nat6";
static const char MODE_HM_3[] PROGMEM = "Ionian #5";
static const char MODE_HM_4[] PROGMEM = "Dorian #4";
static const char MODE_HM_5[] PROGMEM = "Phrygian Dominant";
static const char MODE_HM_6[] PROGMEM = "Lydian #2";
static const char MODE_HM_7[] PROGMEM = "Superlocrian bb7";

static const char* const MODE_NAMES_HARMONIC_MINOR[7] PROGMEM = {
    MODE_HM_1, MODE_HM_2, MODE_HM_3, MODE_HM_4,
    MODE_HM_5, MODE_HM_6, MODE_HM_7
};

// Mode names for Melodic Minor (7 modes)
static const char MODE_MM_1[] PROGMEM = "Melodic Minor";
static const char MODE_MM_2[] PROGMEM = "Dorian b2";
static const char MODE_MM_3[] PROGMEM = "Lydian Augmented";
static const char MODE_MM_4[] PROGMEM = "Lydian Dominant";
static const char MODE_MM_5[] PROGMEM = "Mixolydian b6";
static const char MODE_MM_6[] PROGMEM = "Locrian nat2";
static const char MODE_MM_7[] PROGMEM = "Altered";

static const char* const MODE_NAMES_MELODIC_MINOR[7] PROGMEM = {
    MODE_MM_1, MODE_MM_2, MODE_MM_3, MODE_MM_4,
    MODE_MM_5, MODE_MM_6, MODE_MM_7
};

#endif // GINGODUINO_HAS_SCALE

// ===================================================================
// 10. INTERVAL CONSONANCE & FULL NAMES
// ===================================================================

// Consonance classification by simple semitone (0-11):
// 0=perfect, 1=imperfect, 2=dissonant
static const uint8_t INTERVAL_CONSONANCE[12] PROGMEM = {
    0, // 0  P1  perfect
    2, // 1  2m  dissonant
    2, // 2  2M  dissonant
    1, // 3  3m  imperfect
    1, // 4  3M  imperfect
    0, // 5  4J  perfect
    2, // 6  d5  dissonant
    0, // 7  5J  perfect
    1, // 8  #5  imperfect
    1, // 9  M6  imperfect
    2, // 10 7m  dissonant
    2, // 11 7M  dissonant
};

static const char CONS_0[] PROGMEM = "perfect";
static const char CONS_1[] PROGMEM = "imperfect";
static const char CONS_2[] PROGMEM = "dissonant";

static const char* const CONSONANCE_NAMES[3] PROGMEM = {
    CONS_0, CONS_1, CONS_2
};

// Full interval names in English (24 entries, indexed by semitone 0-23)
static const char IFN_EN_0[]  PROGMEM = "Perfect Unison";
static const char IFN_EN_1[]  PROGMEM = "Minor Second";
static const char IFN_EN_2[]  PROGMEM = "Major Second";
static const char IFN_EN_3[]  PROGMEM = "Minor Third";
static const char IFN_EN_4[]  PROGMEM = "Major Third";
static const char IFN_EN_5[]  PROGMEM = "Perfect Fourth";
static const char IFN_EN_6[]  PROGMEM = "Diminished Fifth";
static const char IFN_EN_7[]  PROGMEM = "Perfect Fifth";
static const char IFN_EN_8[]  PROGMEM = "Minor Sixth";
static const char IFN_EN_9[]  PROGMEM = "Major Sixth";
static const char IFN_EN_10[] PROGMEM = "Minor Seventh";
static const char IFN_EN_11[] PROGMEM = "Major Seventh";
static const char IFN_EN_12[] PROGMEM = "Perfect Octave";
static const char IFN_EN_13[] PROGMEM = "Minor Ninth";
static const char IFN_EN_14[] PROGMEM = "Major Ninth";
static const char IFN_EN_15[] PROGMEM = "Augmented Ninth";
static const char IFN_EN_16[] PROGMEM = "Minor Tenth";
static const char IFN_EN_17[] PROGMEM = "Perfect Eleventh";
static const char IFN_EN_18[] PROGMEM = "Augmented Eleventh";
static const char IFN_EN_19[] PROGMEM = "Perfect Twelfth";
static const char IFN_EN_20[] PROGMEM = "Minor Thirteenth";
static const char IFN_EN_21[] PROGMEM = "Major Thirteenth";
static const char IFN_EN_22[] PROGMEM = "Augmented Thirteenth";
static const char IFN_EN_23[] PROGMEM = "Major Fourteenth";

static const char* const INTERVAL_FULL_NAMES_EN[24] PROGMEM = {
    IFN_EN_0,  IFN_EN_1,  IFN_EN_2,  IFN_EN_3,  IFN_EN_4,  IFN_EN_5,
    IFN_EN_6,  IFN_EN_7,  IFN_EN_8,  IFN_EN_9,  IFN_EN_10, IFN_EN_11,
    IFN_EN_12, IFN_EN_13, IFN_EN_14, IFN_EN_15, IFN_EN_16, IFN_EN_17,
    IFN_EN_18, IFN_EN_19, IFN_EN_20, IFN_EN_21, IFN_EN_22, IFN_EN_23
};

// Full interval names in Portuguese (24 entries, indexed by semitone 0-23)
static const char IFN_PT_0[]  PROGMEM = "Unissono Justo";
static const char IFN_PT_1[]  PROGMEM = "Segunda Menor";
static const char IFN_PT_2[]  PROGMEM = "Segunda Maior";
static const char IFN_PT_3[]  PROGMEM = "Terca Menor";
static const char IFN_PT_4[]  PROGMEM = "Terca Maior";
static const char IFN_PT_5[]  PROGMEM = "Quarta Justa";
static const char IFN_PT_6[]  PROGMEM = "Quinta Diminuta";
static const char IFN_PT_7[]  PROGMEM = "Quinta Justa";
static const char IFN_PT_8[]  PROGMEM = "Sexta Menor";
static const char IFN_PT_9[]  PROGMEM = "Sexta Maior";
static const char IFN_PT_10[] PROGMEM = "Setima Menor";
static const char IFN_PT_11[] PROGMEM = "Setima Maior";
static const char IFN_PT_12[] PROGMEM = "Oitava Justa";
static const char IFN_PT_13[] PROGMEM = "Nona Menor";
static const char IFN_PT_14[] PROGMEM = "Nona Maior";
static const char IFN_PT_15[] PROGMEM = "Nona Aumentada";
static const char IFN_PT_16[] PROGMEM = "Decima Menor";
static const char IFN_PT_17[] PROGMEM = "Decima Primeira Justa";
static const char IFN_PT_18[] PROGMEM = "Decima Primeira Aumentada";
static const char IFN_PT_19[] PROGMEM = "Decima Segunda Justa";
static const char IFN_PT_20[] PROGMEM = "Decima Terceira Menor";
static const char IFN_PT_21[] PROGMEM = "Decima Terceira Maior";
static const char IFN_PT_22[] PROGMEM = "Decima Terceira Aumentada";
static const char IFN_PT_23[] PROGMEM = "Decima Quarta Maior";

static const char* const INTERVAL_FULL_NAMES_PT[24] PROGMEM = {
    IFN_PT_0,  IFN_PT_1,  IFN_PT_2,  IFN_PT_3,  IFN_PT_4,  IFN_PT_5,
    IFN_PT_6,  IFN_PT_7,  IFN_PT_8,  IFN_PT_9,  IFN_PT_10, IFN_PT_11,
    IFN_PT_12, IFN_PT_13, IFN_PT_14, IFN_PT_15, IFN_PT_16, IFN_PT_17,
    IFN_PT_18, IFN_PT_19, IFN_PT_20, IFN_PT_21, IFN_PT_22, IFN_PT_23
};

// ===================================================================
// 11. FRETBOARD TUNING DATA
// ===================================================================

#if GINGODUINO_HAS_FRETBOARD

// Standard tunings stored as MIDI note numbers for each open string.
// Strings ordered from lowest pitch to highest pitch.

// Violao (guitar) — E2 A2 D3 G3 B3 E4
static const uint8_t TUNING_VIOLAO[6] PROGMEM = { 40, 45, 50, 55, 59, 64 };

// Cavaquinho — D4 G4 B4 D5
static const uint8_t TUNING_CAVAQUINHO[4] PROGMEM = { 62, 67, 71, 74 };

// Bandolim (mandolin) — G3 D4 A4 E5
static const uint8_t TUNING_BANDOLIM[4] PROGMEM = { 55, 62, 69, 76 };

// Ukulele — G4 C4 E4 A4
static const uint8_t TUNING_UKULELE[4] PROGMEM = { 67, 60, 64, 69 };

#endif // GINGODUINO_HAS_FRETBOARD

// ===================================================================
// PROGMEM read helpers
// ===================================================================

/// Read a PROGMEM string into a buffer
inline void readPgmStr(char* dest, const char* pgmSrc, uint8_t maxLen) {
    uint8_t i = 0;
    char c;
    while (i < maxLen - 1 && (c = (char)pgm_read_byte(pgmSrc + i)) != '\0') {
        dest[i++] = c;
    }
    dest[i] = '\0';
}

/// Binary search in sorted PROGMEM EnharmonicEntry array
inline int8_t findEnharmonic(const char* input) {
    int8_t lo = 0;
    int8_t hi = (int8_t)(ENHARMONIC_MAP_SIZE - 1);
    while (lo <= hi) {
        int8_t mid = (lo + hi) / 2;
        char buf[5];
        readPgmStr(buf, ENHARMONIC_MAP[mid].input, sizeof(buf));
        int cmp = strcmp(input, buf);
        if (cmp == 0) return mid;
        if (cmp < 0) hi = mid - 1;
        else lo = mid + 1;
    }
    return -1;
}

/// Binary search in sorted PROGMEM ChordTypeAlias array
inline int8_t findChordType(const char* typeName) {
    int8_t lo = 0;
    int8_t hi = (int8_t)(CHORD_TYPE_MAP_SIZE - 1);
    while (lo <= hi) {
        int8_t mid = (lo + hi) / 2;
        char buf[10];
        readPgmStr(buf, CHORD_TYPE_MAP[mid].name, sizeof(buf));
        int cmp = strcmp(typeName, buf);
        if (cmp == 0) return mid;
        if (cmp < 0) hi = mid - 1;
        else lo = mid + 1;
    }
    return -1;
}

/// Read a ChordFormula from PROGMEM
inline void readChordFormula(uint8_t idx, uint8_t* intervals, uint8_t* count) {
    *count = pgm_read_byte(&CHORD_FORMULAS[idx].count);
    for (uint8_t i = 0; i < *count; i++) {
        intervals[i] = pgm_read_byte(&CHORD_FORMULAS[idx].intervals[i]);
    }
}

/// Read chromatic note name from PROGMEM
inline void readChromaticName(uint8_t semitone, char* dest, uint8_t maxLen) {
    readPgmStr(dest, CHROMATIC_NAMES[semitone % 12], maxLen);
}

} // namespace data
} // namespace gingoduino

#endif // GINGODUINO_PROGMEM_H
