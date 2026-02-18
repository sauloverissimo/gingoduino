// Gingoduino — T-Display-S3 Music Theory Explorer
//
// SPDX-License-Identifier: MIT
//
// Interactive demo for LilyGo T-Display-S3 (ESP32-S3 + ST7789 170x320).
// Navigate with the 2 built-in buttons:
//   LEFT  (GPIO 0 / BOOT): switch page
//   RIGHT (GPIO 14):       cycle items within page
//
// Pages:
//   1. Note Explorer       — 12 chromatic notes with frequency/MIDI
//   2. Interval Explorer   — 12 simple intervals with consonance
//   3. Chord Explorer      — common chords with notes and intervals
//   4. Scale Explorer      — scales/modes with signature, brightness
//   5. Harmonic Field      — triads + sevenths with T/S/D functions
//   6. Fretboard           — guitar diagram with chord/scale overlay
//   7. Sequence            — timeline visualization of musical events
//
// REQUIRES: TFT_eSPI library configured for T-Display-S3
//   In TFT_eSPI/User_Setup_Select.h:
//     - Comment out:   #include <User_Setup.h>
//     - Uncomment:     #include <User_Setups/Setup206_LilyGo_T_Display_S3.h>

#include <TFT_eSPI.h>
#include <Gingoduino.h>

using namespace gingoduino;

// ---------------------------------------------------------------------------
// Hardware
// ---------------------------------------------------------------------------
#define BTN_LEFT   0   // BOOT button — switch page
#define BTN_RIGHT  14  // KEY  button — cycle items
#define TFT_BL_PIN 38

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------
TFT_eSPI tft = TFT_eSPI();

// Colors (RGB565)
#define C_BG        0x1082  // dark grey-blue
#define C_HEADER    0x2945  // slightly lighter
#define C_ACCENT    0x07FF  // cyan
#define C_NOTE      0xFFE0  // yellow
#define C_INTERVAL  0x07FF  // cyan
#define C_CHORD     0xFBE0  // orange
#define C_SCALE     0x87F0  // green
#define C_FIELD     0xF81F  // magenta
#define C_FRETBOARD 0xFD20  // warm orange
#define C_SEQUENCE  0xB71C  // purple
#define C_TEXT      0xFFFF  // white
#define C_DIM       0x8410  // grey
#define C_TONIC     0x07E0  // green
#define C_SUBDOM    0xFFE0  // yellow
#define C_DOMINANT  0xF800  // red
#define C_PERFECT   0x07E0  // green  — perfect consonance
#define C_IMPERFECT 0x04DF  // blue   — imperfect consonance
#define C_DISSONANT 0xF800  // red    — dissonance
#define C_STRING    0xBDF7  // light grey — fretboard strings
#define C_FRET_LINE 0x6B4D  // medium grey — frets
#define C_NUT       0xFFFF  // white — nut (fret 0)
#define C_DOT       0x07E0  // green — fretted positions
#define C_OPEN_STR  0x07FF  // cyan — open string
#define C_MUTED_STR 0xF800  // red — muted string
#define C_EVT_NOTE  0xFFE0  // yellow
#define C_EVT_CHORD 0xFBE0  // orange
#define C_EVT_REST  0x4208  // dark grey
#define C_BEAT_LINE 0x6B4D  // grey

// Screen dimensions (landscape)
#define SCR_W 320
#define SCR_H 170

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------
enum Page {
    PAGE_NOTE, PAGE_INTERVAL, PAGE_CHORD, PAGE_SCALE,
    PAGE_FIELD, PAGE_FRETBOARD, PAGE_SEQUENCE, PAGE_COUNT
};

static Page    currentPage = PAGE_NOTE;
static uint8_t itemIdx     = 0;
static bool    needRedraw  = true;

// Debounce
static unsigned long lastBtnLeft  = 0;
static unsigned long lastBtnRight = 0;
#define DEBOUNCE_MS 250

// ---------------------------------------------------------------------------
// Data for cycling
// ---------------------------------------------------------------------------
static const char* NOTE_NAMES[12] = {
    "C", "C#", "D", "D#", "E", "F",
    "F#", "G", "G#", "A", "A#", "B"
};

static const char* CHORD_LIST[] = {
    "CM", "Cm", "C7", "C7M", "Cm7", "Cdim", "Caug",
    "DM", "Dm7", "D7", "EM", "Em", "FM", "F7M",
    "GM", "G7", "Am", "Am7", "Bdim", "Bm7(b5)"
};
static const uint8_t CHORD_LIST_SIZE = sizeof(CHORD_LIST) / sizeof(CHORD_LIST[0]);

struct ScaleEntry {
    const char* tonic;
    ScaleType   type;
    const char* label;
};
static const ScaleEntry SCALE_LIST[] = {
    {"C",  SCALE_MAJOR,          "C Major"},
    {"A",  SCALE_NATURAL_MINOR,  "A Natural Minor"},
    {"A",  SCALE_HARMONIC_MINOR, "A Harmonic Minor"},
    {"A",  SCALE_MELODIC_MINOR,  "A Melodic Minor"},
    {"C",  SCALE_DIMINISHED,     "C Diminished"},
    {"C",  SCALE_WHOLE_TONE,     "C Whole Tone"},
    {"A",  SCALE_BLUES,          "A Blues"},
};
static const uint8_t SCALE_LIST_SIZE = sizeof(SCALE_LIST) / sizeof(SCALE_LIST[0]);

struct FieldEntry {
    const char* tonic;
    ScaleType   type;
    const char* label;
};
static const FieldEntry FIELD_LIST[] = {
    {"C",  SCALE_MAJOR,          "C Major"},
    {"G",  SCALE_MAJOR,          "G Major"},
    {"D",  SCALE_MAJOR,          "D Major"},
    {"A",  SCALE_NATURAL_MINOR,  "A Minor"},
    {"E",  SCALE_NATURAL_MINOR,  "E Minor"},
    {"D",  SCALE_NATURAL_MINOR,  "D Minor"},
};
static const uint8_t FIELD_LIST_SIZE = sizeof(FIELD_LIST) / sizeof(FIELD_LIST[0]);

// Fretboard cycling: chords then scales
static const char* FRET_CHORD_LIST[] = {
    "CM", "Dm", "Em", "FM", "GM", "Am", "Bdim",
    "C7", "D7", "E7", "G7", "A7", "Cm", "Dm7", "Em7", "Am7"
};
static const uint8_t FRET_CHORD_SIZE = sizeof(FRET_CHORD_LIST) / sizeof(FRET_CHORD_LIST[0]);

struct FretScaleEntry {
    const char* tonic;
    ScaleType   type;
    const char* label;
};
static const FretScaleEntry FRET_SCALE_LIST[] = {
    {"C", SCALE_MAJOR,          "C Major"},
    {"A", SCALE_NATURAL_MINOR,  "A Minor"},
    {"G", SCALE_MAJOR,          "G Major"},
    {"E", SCALE_NATURAL_MINOR,  "E Minor"},
    {"A", SCALE_BLUES,          "A Blues"},
};
static const uint8_t FRET_SCALE_SIZE = sizeof(FRET_SCALE_LIST) / sizeof(FRET_SCALE_LIST[0]);
static const uint8_t FRET_TOTAL_SIZE = FRET_CHORD_SIZE + FRET_SCALE_SIZE;

// Sequence presets
struct SeqPreset {
    const char* name;
    uint16_t    bpm;
    uint8_t     beatsPerBar;
    uint8_t     beatUnit;
};
static const SeqPreset SEQ_PRESETS[] = {
    {"I-IV-V-I in C",   120, 4, 4},
    {"ii-V-I Jazz",     110, 4, 4},
    {"Simple Melody",   100, 4, 4},
    {"Rests & Notes",   120, 4, 4},
    {"Bossa Pattern",   140, 4, 4},
};
static const uint8_t SEQ_PRESETS_SIZE = sizeof(SEQ_PRESETS) / sizeof(SEQ_PRESETS[0]);

// ---------------------------------------------------------------------------
// Drawing helpers
// ---------------------------------------------------------------------------

void drawHeader(const char* title, uint16_t color) {
    tft.fillRect(0, 0, SCR_W, 28, C_HEADER);
    tft.setTextColor(color, C_HEADER);
    tft.setTextSize(2);
    tft.setCursor(8, 6);
    tft.print(title);

    // Page indicator dots
    for (uint8_t i = 0; i < PAGE_COUNT; i++) {
        int x = SCR_W - 12 - (PAGE_COUNT - 1 - i) * 14;
        if (i == (uint8_t)currentPage) {
            tft.fillCircle(x, 14, 4, color);
        } else {
            tft.drawCircle(x, 14, 4, C_DIM);
        }
    }

    tft.drawFastHLine(0, 28, SCR_W, color);
}

void drawFooter() {
    tft.drawFastHLine(0, SCR_H - 18, SCR_W, C_DIM);
    tft.setTextSize(1);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(6, SCR_H - 13);
    tft.print("[BOOT] page   [KEY] next");
}

void clearContent() {
    tft.fillRect(0, 29, SCR_W, SCR_H - 29 - 18, C_BG);
}

/// Draw key signature string (e.g. "2#", "3b", "0").
void drawSignature(int x, int y, int8_t sig) {
    tft.setCursor(x, y);
    if (sig > 0) {
        tft.print(sig);
        tft.print("#");
    } else if (sig < 0) {
        tft.print(-sig);
        tft.print("b");
    } else {
        tft.print("0");
    }
}

/// Return RGB565 color for an interval's consonance class.
uint16_t consonanceColor(const GingoInterval& iv) {
    char buf[12];
    iv.consonance(buf, sizeof(buf));
    if (buf[0] == 'p') return C_PERFECT;
    if (buf[0] == 'i') return C_IMPERFECT;
    return C_DISSONANT;
}

// ---------------------------------------------------------------------------
// Page: Note Explorer
// ---------------------------------------------------------------------------

void drawNotePage() {
    drawHeader("Note Explorer", C_NOTE);
    clearContent();
    drawFooter();

    GingoNote note(NOTE_NAMES[itemIdx % 12]);

    // Big note name
    tft.setTextColor(C_NOTE, C_BG);
    tft.setTextSize(5);
    tft.setCursor(20, 42);
    tft.print(note.name());

    // Natural form
    tft.setTextSize(2);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(20, 96);
    tft.print("nat: ");
    tft.setTextColor(C_TEXT, C_BG);
    tft.print(note.natural());

    // Info column on the right
    int rx = 150;
    tft.setTextSize(2);

    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(rx, 40);
    tft.print("Semi: ");
    tft.setTextColor(C_TEXT, C_BG);
    tft.print(note.semitone());

    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(rx, 62);
    tft.print("MIDI: ");
    tft.setTextColor(C_TEXT, C_BG);
    tft.print(note.midiNumber(4));

    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(rx, 84);
    tft.print("Freq: ");
    tft.setTextColor(C_ACCENT, C_BG);
    tft.print(note.frequency(4), 1);

    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(rx, 106);
    tft.print("Oct4  Hz");

    // Chromatic bar at bottom (highlight current)
    int barY = 126;
    tft.setTextSize(1);
    for (uint8_t i = 0; i < 12; i++) {
        int bx = 8 + i * 25;
        if (i == itemIdx % 12) {
            tft.fillRoundRect(bx, barY, 23, 16, 3, C_NOTE);
            tft.setTextColor(0x0000, C_NOTE);
        } else {
            tft.drawRoundRect(bx, barY, 23, 16, 3, C_DIM);
            tft.setTextColor(C_DIM, C_BG);
        }
        tft.setCursor(bx + (strlen(NOTE_NAMES[i]) == 1 ? 8 : 4), barY + 4);
        tft.print(NOTE_NAMES[i]);
    }
}

// ---------------------------------------------------------------------------
// Page: Interval Explorer
// ---------------------------------------------------------------------------

void drawIntervalPage() {
    drawHeader("Interval Explorer", C_INTERVAL);
    clearContent();
    drawFooter();

    uint8_t semi = itemIdx % 12;
    GingoInterval iv((uint8_t)semi);

    // Label (large)
    char labelBuf[8];
    iv.label(labelBuf, sizeof(labelBuf));
    tft.setTextColor(C_INTERVAL, C_BG);
    tft.setTextSize(3);
    tft.setCursor(12, 36);
    tft.print(labelBuf);

    // Full name EN
    char nameBuf[32];
    iv.fullName(nameBuf, sizeof(nameBuf));
    tft.setTextSize(2);
    tft.setTextColor(C_TEXT, C_BG);
    tft.setCursor(12, 64);
    tft.print(nameBuf);

    // Full name PT
    char namePtBuf[32];
    iv.fullNamePt(namePtBuf, sizeof(namePtBuf));
    tft.setTextSize(1);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(12, 86);
    tft.print(namePtBuf);

    // Right column: consonance, semitones, degree
    int rx = 180;

    // Consonance box
    char consBuf[12];
    iv.consonance(consBuf, sizeof(consBuf));
    uint16_t consColor = consonanceColor(iv);
    tft.fillRoundRect(rx, 36, 130, 18, 3, consColor);
    tft.setTextColor(0x0000, consColor);
    tft.setTextSize(1);
    int consLen = strlen(consBuf);
    tft.setCursor(rx + 65 - (consLen * 3), 41);
    tft.print(consBuf);

    // Semitones
    tft.setTextSize(1);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(rx, 60);
    tft.print("Semitones: ");
    tft.setTextColor(C_TEXT, C_BG);
    tft.print(iv.semitones());

    // Degree
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(rx, 74);
    tft.print("Degree: ");
    tft.setTextColor(C_TEXT, C_BG);
    tft.print(iv.degree());

    // Anglo-Saxon
    char angloBuf[8];
    iv.angloSaxon(angloBuf, sizeof(angloBuf));
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(rx, 88);
    tft.print("Anglo: ");
    tft.setTextColor(C_ACCENT, C_BG);
    tft.print(angloBuf);

    // Chromatic bar at bottom (highlight current with consonance color)
    static const char* IV_LABELS[12] = {
        "P1", "m2", "M2", "m3", "M3", "P4",
        "TT", "P5", "m6", "M6", "m7", "M7"
    };
    int barY = 114;
    tft.setTextSize(1);
    for (uint8_t i = 0; i < 12; i++) {
        int bx = 8 + i * 25;
        if (i == semi) {
            tft.fillRoundRect(bx, barY, 23, 16, 3, consColor);
            tft.setTextColor(0x0000, consColor);
        } else {
            tft.drawRoundRect(bx, barY, 23, 16, 3, C_DIM);
            tft.setTextColor(C_DIM, C_BG);
        }
        int lblLen = strlen(IV_LABELS[i]);
        tft.setCursor(bx + 12 - (lblLen * 3), barY + 4);
        tft.print(IV_LABELS[i]);
    }
}

// ---------------------------------------------------------------------------
// Page: Chord Explorer
// ---------------------------------------------------------------------------

void drawChordPage() {
    drawHeader("Chord Explorer", C_CHORD);
    clearContent();
    drawFooter();

    uint8_t idx = itemIdx % CHORD_LIST_SIZE;
    GingoChord chord(CHORD_LIST[idx]);

    // Chord name
    tft.setTextSize(3);
    tft.setTextColor(C_CHORD, C_BG);
    tft.setCursor(12, 36);
    tft.print(chord.name());

    // Root + type
    tft.setTextSize(1);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(12, 64);
    tft.print("root: ");
    tft.setTextColor(C_TEXT, C_BG);
    tft.print(chord.root().name());
    tft.setTextColor(C_DIM, C_BG);
    tft.print("  type: ");
    tft.setTextColor(C_TEXT, C_BG);
    tft.print(chord.type());

    // Notes as boxes
    GingoNote notes[7];
    uint8_t n = chord.notes(notes, 7);

    tft.setTextSize(2);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(12, 80);
    tft.print("Notes:");

    for (uint8_t i = 0; i < n; i++) {
        int bx = 12 + i * 42;
        int by = 98;
        tft.fillRoundRect(bx, by, 38, 22, 4, C_CHORD);
        tft.setTextColor(0x0000, C_CHORD);
        tft.setTextSize(2);
        int nameLen = strlen(notes[i].name());
        tft.setCursor(bx + (nameLen == 1 ? 12 : 5), by + 3);
        tft.print(notes[i].name());
    }

    // Intervals (GingoInterval objects with full names)
    GingoInterval ivs[7];
    uint8_t ni = chord.intervals(ivs, 7);

    tft.setTextSize(1);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(12, 126);
    tft.print("Intervals: ");

    for (uint8_t i = 0; i < ni; i++) {
        char lbl[8];
        ivs[i].label(lbl, sizeof(lbl));
        uint16_t ivColor = consonanceColor(ivs[i]);
        tft.setTextColor(ivColor, C_BG);
        if (i > 0) tft.print("  ");
        tft.print(lbl);
    }
}

// ---------------------------------------------------------------------------
// Page: Scale Explorer
// ---------------------------------------------------------------------------

void drawScalePage() {
    drawHeader("Scale Explorer", C_SCALE);
    clearContent();
    drawFooter();

    uint8_t idx = itemIdx % SCALE_LIST_SIZE;
    const ScaleEntry& se = SCALE_LIST[idx];
    GingoScale scale(se.tonic, se.type);

    // Scale label + signature
    tft.setTextSize(2);
    tft.setTextColor(C_SCALE, C_BG);
    tft.setCursor(12, 36);
    tft.print(se.label);

    // Signature badge
    int8_t sig = scale.signature();
    tft.setTextSize(1);
    tft.setTextColor(C_ACCENT, C_BG);
    tft.setCursor(12 + strlen(se.label) * 12 + 8, 40);
    drawSignature(tft.getCursorX(), tft.getCursorY(), sig);

    // Mode name + quality
    char modeBuf[22];
    tft.setTextSize(1);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(12, 54);
    tft.print("mode: ");
    tft.setTextColor(C_TEXT, C_BG);
    tft.print(scale.modeName(modeBuf, sizeof(modeBuf)));
    tft.setTextColor(C_DIM, C_BG);
    tft.print("  quality: ");
    tft.setTextColor(C_TEXT, C_BG);
    tft.print(scale.quality());

    // Notes as a row
    GingoNote notes[12];
    uint8_t n = scale.notes(notes, 12);

    tft.setTextSize(2);
    int startX = 12;
    int boxW = (n <= 7) ? 38 : 30;
    int gap = (n <= 7) ? 4 : 3;

    for (uint8_t i = 0; i < n; i++) {
        int bx = startX + i * (boxW + gap);
        int by = 68;

        uint16_t boxColor = (i == 0) ? C_SCALE : C_HEADER;
        uint16_t textColor = (i == 0) ? 0x0000 : C_TEXT;

        tft.fillRoundRect(bx, by, boxW, 26, 4, boxColor);
        tft.setTextColor(textColor, boxColor);
        int nameLen = strlen(notes[i].name());
        tft.setCursor(bx + (boxW / 2) - (nameLen * 6), by + 5);
        tft.print(notes[i].name());

        // Degree number below
        tft.setTextSize(1);
        tft.setTextColor(C_DIM, C_BG);
        tft.setCursor(bx + (boxW / 2) - 3, by + 30);
        tft.print(i + 1);
        tft.setTextSize(2);
    }

    // Brightness bar (7 segments)
    uint8_t bright = scale.brightness();
    tft.setTextSize(1);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(12, 108);
    tft.print("Brightness: ");

    for (uint8_t i = 0; i < 7; i++) {
        int bx = 84 + i * 14;
        if (i < bright) {
            tft.fillRect(bx, 106, 12, 10, C_SCALE);
        } else {
            tft.drawRect(bx, 106, 12, 10, C_DIM);
        }
    }

    tft.setTextColor(C_TEXT, C_BG);
    tft.setCursor(84 + 7 * 14 + 6, 108);
    tft.print(bright);

    // Relative + parallel
    GingoScale rel = scale.relative();
    GingoScale par = scale.parallel();
    char relMode[22], parMode[22];

    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(12, 122);
    tft.print("Relative: ");
    tft.setTextColor(C_ACCENT, C_BG);
    tft.print(rel.tonic().name());
    tft.print(" ");
    tft.print(rel.modeName(relMode, sizeof(relMode)));

    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(12, 134);
    tft.print("Parallel: ");
    tft.setTextColor(C_ACCENT, C_BG);
    tft.print(par.tonic().name());
    tft.print(" ");
    tft.print(par.modeName(parMode, sizeof(parMode)));
}

// ---------------------------------------------------------------------------
// Page: Harmonic Field
// ---------------------------------------------------------------------------

void drawFieldPage() {
    drawHeader("Harmonic Field", C_FIELD);
    clearContent();
    drawFooter();

    uint8_t idx = itemIdx % FIELD_LIST_SIZE;
    const FieldEntry& fe = FIELD_LIST[idx];
    GingoField field(fe.tonic, fe.type);

    // Field label + signature
    tft.setTextSize(2);
    tft.setTextColor(C_FIELD, C_BG);
    tft.setCursor(12, 36);
    tft.print(fe.label);

    int8_t sig = field.signature();
    tft.setTextSize(1);
    tft.setTextColor(C_ACCENT, C_BG);
    tft.setCursor(12 + strlen(fe.label) * 12 + 8, 40);
    drawSignature(tft.getCursorX(), tft.getCursorY(), sig);

    // Triads row
    GingoChord triads[7];
    uint8_t nt = field.chords(triads, 7);

    tft.setTextSize(1);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(12, 56);
    tft.print("Triads:");

    for (uint8_t i = 0; i < nt && i < 7; i++) {
        int bx = 8 + i * 44;
        int by = 68;

        // Color by function
        uint8_t func = field.function(i + 1);
        uint16_t funcColor;
        switch (func) {
            case FUNC_TONIC:       funcColor = C_TONIC;    break;
            case FUNC_SUBDOMINANT: funcColor = C_SUBDOM;   break;
            case FUNC_DOMINANT:    funcColor = C_DOMINANT;  break;
            default:               funcColor = C_DIM;       break;
        }

        // Chord box
        tft.fillRoundRect(bx, by, 42, 20, 3, C_HEADER);
        tft.setTextSize(1);
        tft.setTextColor(C_TEXT, C_HEADER);
        int nameLen = strlen(triads[i].name());
        tft.setCursor(bx + 21 - (nameLen * 3), by + 6);
        tft.print(triads[i].name());

        // Role text below chord
        char roleBuf[12];
        field.role(i + 1, roleBuf, sizeof(roleBuf));
        tft.setTextColor(funcColor, C_BG);
        int roleLen = strlen(roleBuf);
        tft.setCursor(bx + 21 - (roleLen * 3), by + 23);
        tft.print(roleBuf);

        // Function dot
        tft.fillCircle(bx + 21, by - 4, 3, funcColor);
    }

    // Sevenths row
    GingoChord sevs[7];
    uint8_t ns = field.sevenths(sevs, 7);

    tft.setTextSize(1);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(12, 100);
    tft.print("Sevenths:");

    for (uint8_t i = 0; i < ns && i < 7; i++) {
        int bx = 8 + i * 44;
        int by = 112;
        tft.fillRoundRect(bx, by, 42, 20, 3, C_HEADER);
        tft.setTextSize(1);
        tft.setTextColor(C_ACCENT, C_HEADER);
        int nameLen = strlen(sevs[i].name());
        int cx = bx + 21 - (nameLen * 3);
        if (cx < bx + 1) cx = bx + 1;
        tft.setCursor(cx, by + 6);
        tft.print(sevs[i].name());
    }

    // Legend
    int ly = SCR_H - 32;
    tft.setTextSize(1);
    tft.fillCircle(12, ly + 3, 3, C_TONIC);
    tft.setTextColor(C_TEXT, C_BG);
    tft.setCursor(20, ly);
    tft.print("T");
    tft.fillCircle(42, ly + 3, 3, C_SUBDOM);
    tft.setCursor(50, ly);
    tft.print("S");
    tft.fillCircle(72, ly + 3, 3, C_DOMINANT);
    tft.setCursor(80, ly);
    tft.print("D");
}

// ---------------------------------------------------------------------------
// Page: Fretboard Explorer
// ---------------------------------------------------------------------------

// Fretboard geometry constants
#define FB_X        36
#define FB_Y        64
#define FB_FRETS    5
#define FRET_W      52
#define STRING_GAP  10
#define FB_STRINGS  6

/// Draw the fretboard grid (strings, frets, nut, labels).
void drawFretGrid(uint8_t baseFret) {
    uint8_t totalStrings = FB_STRINGS;
    int fbW = FB_FRETS * FRET_W;
    int fbH = (totalStrings - 1) * STRING_GAP;

    // Strings (horizontal) — top to bottom: E2 A2 D3 G3 B3 E4
    static const char* STRING_NAMES[6] = {"E", "A", "D", "G", "B", "E"};
    for (uint8_t s = 0; s < totalStrings; s++) {
        int y = FB_Y + s * STRING_GAP;
        tft.drawFastHLine(FB_X, y, fbW, C_STRING);

        // String name label
        tft.setTextSize(1);
        tft.setTextColor(C_DIM, C_BG);
        tft.setCursor(FB_X - 14, y - 3);
        tft.print(STRING_NAMES[s]);
    }

    // Nut (thick line at fret 0)
    if (baseFret == 0) {
        tft.fillRect(FB_X - 2, FB_Y, 4, fbH, C_NUT);
    }

    // Frets (vertical)
    for (uint8_t f = 1; f <= FB_FRETS; f++) {
        int x = FB_X + f * FRET_W;
        tft.drawFastVLine(x, FB_Y, fbH, C_FRET_LINE);
    }

    // Fret numbers below
    tft.setTextSize(1);
    tft.setTextColor(C_DIM, C_BG);
    for (uint8_t f = 0; f < FB_FRETS; f++) {
        int x = FB_X + f * FRET_W + FRET_W / 2;
        int num = baseFret + f + (baseFret == 0 ? 1 : 1);
        tft.setCursor(x - 3, FB_Y + fbH + 6);
        tft.print(num);
    }
}

void drawFretboardPage() {
    drawHeader("Fretboard", C_FRETBOARD);
    clearContent();
    drawFooter();

    uint8_t idx = itemIdx % FRET_TOTAL_SIZE;
    bool isScale = (idx >= FRET_CHORD_SIZE);

    GingoFretboard fb = GingoFretboard::violao();

    if (!isScale) {
        // Chord mode
        GingoChord chord(FRET_CHORD_LIST[idx]);
        GingoFingering fg;
        bool ok = fb.fingering(chord, 0, fg);

        // Title
        tft.setTextSize(2);
        tft.setTextColor(C_FRETBOARD, C_BG);
        tft.setCursor(12, 36);
        tft.print(chord.name());

        tft.setTextSize(1);
        tft.setTextColor(C_DIM, C_BG);
        tft.setCursor(12, 54);
        if (ok) {
            tft.print("score: ");
            tft.setTextColor(C_TEXT, C_BG);
            tft.print(fg.score);
            tft.setTextColor(C_DIM, C_BG);
            tft.print("  notes: ");
            tft.setTextColor(C_TEXT, C_BG);
            tft.print(fg.numNotes);
        } else {
            tft.print("(no fingering found)");
        }

        // Draw grid
        drawFretGrid(0);

        // Draw fingering positions
        if (ok) {
            for (uint8_t s = 0; s < fg.numStrings; s++) {
                int y = FB_Y + s * STRING_GAP;

                if (fg.strings[s].action == STRING_OPEN) {
                    // "O" left of nut
                    tft.setTextSize(1);
                    tft.setTextColor(C_OPEN_STR, C_BG);
                    tft.setCursor(FB_X - 8, y - 3);
                    tft.print("O");
                } else if (fg.strings[s].action == STRING_MUTED) {
                    // "X" left of nut
                    tft.setTextSize(1);
                    tft.setTextColor(C_MUTED_STR, C_BG);
                    tft.setCursor(FB_X - 8, y - 3);
                    tft.print("X");
                } else if (fg.strings[s].action == STRING_FRETTED) {
                    uint8_t f = fg.strings[s].fret;
                    if (f >= 1 && f <= FB_FRETS) {
                        int x = FB_X + (f - 1) * FRET_W + FRET_W / 2;
                        tft.fillCircle(x, y, 4, C_DOT);
                    }
                }
            }
        }

        // Mode indicator
        tft.setTextSize(1);
        tft.setTextColor(C_FRETBOARD, C_BG);
        tft.setCursor(SCR_W - 48, 36);
        tft.print("CHORD");
    } else {
        // Scale mode
        uint8_t scaleIdx = idx - FRET_CHORD_SIZE;
        const FretScaleEntry& se = FRET_SCALE_LIST[scaleIdx];
        GingoScale scale(se.tonic, se.type);

        // Title
        tft.setTextSize(2);
        tft.setTextColor(C_FRETBOARD, C_BG);
        tft.setCursor(12, 36);
        tft.print(se.label);

        // Positions count
        GingoFretPos positions[48];
        uint8_t npos = fb.scalePositions(scale, positions, 48, 0, FB_FRETS);

        tft.setTextSize(1);
        tft.setTextColor(C_DIM, C_BG);
        tft.setCursor(12, 54);
        tft.print(npos);
        tft.print(" positions");

        // Draw grid
        drawFretGrid(0);

        // Draw scale positions
        uint8_t tonicSemi = GingoNote(se.tonic).semitone();
        for (uint8_t i = 0; i < npos; i++) {
            uint8_t s = positions[i].string;
            uint8_t f = positions[i].fret;
            int y = FB_Y + s * STRING_GAP;

            bool isTonic = (positions[i].midi % 12 == tonicSemi);

            if (f == 0) {
                // Open string: small circle left of nut
                tft.setTextSize(1);
                tft.setTextColor(isTonic ? C_FRETBOARD : C_OPEN_STR, C_BG);
                tft.setCursor(FB_X - 8, y - 3);
                tft.print("O");
            } else if (f <= FB_FRETS) {
                int x = FB_X + (f - 1) * FRET_W + FRET_W / 2;
                uint16_t dotColor = isTonic ? C_FRETBOARD : C_DOT;
                tft.fillCircle(x, y, isTonic ? 5 : 3, dotColor);
            }
        }

        // Mode indicator
        tft.setTextSize(1);
        tft.setTextColor(C_SCALE, C_BG);
        tft.setCursor(SCR_W - 48, 36);
        tft.print("SCALE");
    }
}

// ---------------------------------------------------------------------------
// Page: Sequence Explorer
// ---------------------------------------------------------------------------

/// Build a preset sequence.
void buildPresetSequence(uint8_t presetIdx, GingoSequence& seq) {
    seq.clear();

    GingoDuration quarter("quarter");
    GingoDuration half("half");
    GingoDuration eighth("eighth");

    switch (presetIdx) {
        case 0:  // I-IV-V-I in C
            seq.add(GingoEvent::chordEvent(GingoChord("CM"), quarter));
            seq.add(GingoEvent::chordEvent(GingoChord("FM"), quarter));
            seq.add(GingoEvent::chordEvent(GingoChord("GM"), quarter));
            seq.add(GingoEvent::chordEvent(GingoChord("CM"), quarter));
            break;
        case 1:  // ii-V-I Jazz
            seq.add(GingoEvent::chordEvent(GingoChord("Dm7"), half));
            seq.add(GingoEvent::chordEvent(GingoChord("G7"), quarter));
            seq.add(GingoEvent::chordEvent(GingoChord("C7M"), quarter));
            break;
        case 2:  // Simple Melody
            seq.add(GingoEvent::noteEvent(GingoNote("C"), quarter, 4));
            seq.add(GingoEvent::noteEvent(GingoNote("D"), quarter, 4));
            seq.add(GingoEvent::noteEvent(GingoNote("E"), quarter, 4));
            seq.add(GingoEvent::noteEvent(GingoNote("F"), quarter, 4));
            seq.add(GingoEvent::noteEvent(GingoNote("G"), quarter, 4));
            break;
        case 3:  // Rests & Notes
            seq.add(GingoEvent::noteEvent(GingoNote("C"), eighth, 4));
            seq.add(GingoEvent::rest(eighth));
            seq.add(GingoEvent::noteEvent(GingoNote("E"), eighth, 4));
            seq.add(GingoEvent::rest(eighth));
            seq.add(GingoEvent::noteEvent(GingoNote("G"), eighth, 4));
            break;
        case 4:  // Bossa Pattern
            seq.add(GingoEvent::chordEvent(GingoChord("CM"), quarter));
            seq.add(GingoEvent::chordEvent(GingoChord("Dm7"), quarter));
            seq.add(GingoEvent::chordEvent(GingoChord("G7"), quarter));
            seq.add(GingoEvent::chordEvent(GingoChord("CM"), quarter));
            break;
    }
}

void drawSequencePage() {
    drawHeader("Sequence", C_SEQUENCE);
    clearContent();
    drawFooter();

    uint8_t idx = itemIdx % SEQ_PRESETS_SIZE;
    const SeqPreset& sp = SEQ_PRESETS[idx];

    GingoTempo tempo(sp.bpm);
    GingoTimeSig timeSig(sp.beatsPerBar, sp.beatUnit);
    GingoSequence seq(tempo, timeSig);
    buildPresetSequence(idx, seq);

    // Preset name
    tft.setTextSize(2);
    tft.setTextColor(C_SEQUENCE, C_BG);
    tft.setCursor(12, 36);
    tft.print(sp.name);

    // Metadata line
    tft.setTextSize(1);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(12, 56);
    tft.print(sp.bpm);
    tft.print(" BPM  ");
    tft.print(sp.beatsPerBar);
    tft.print("/");
    tft.print(sp.beatUnit);

    char classBuf[12];
    timeSig.classification(classBuf, sizeof(classBuf));
    tft.print(" ");
    tft.print(classBuf);

    // Stats
    float totalB = seq.totalBeats();
    float totalS = seq.totalSeconds();
    float bars = seq.barCount();

    tft.setTextColor(C_TEXT, C_BG);
    tft.setCursor(180, 56);
    tft.print(totalB, 1);
    tft.print(" beats  ");
    tft.print(bars, 1);
    tft.print(" bars");

    // Timeline visualization
    int tlY = 76;
    int tlH = 36;
    int tlX = 12;
    int tlW = SCR_W - 24;

    // Beat grid lines
    if (totalB > 0) {
        float pixelsPerBeat = (float)tlW / totalB;
        for (uint8_t b = 0; b <= (uint8_t)totalB; b++) {
            int x = tlX + (int)(b * pixelsPerBeat);
            tft.drawFastVLine(x, tlY, tlH + 14, C_BEAT_LINE);

            // Beat number below
            if (b < (uint8_t)totalB) {
                tft.setTextSize(1);
                tft.setTextColor(C_DIM, C_BG);
                tft.setCursor(x + 2, tlY + tlH + 16);
                tft.print(b + 1);
            }
        }
    }

    // Event blocks
    float xOffset = 0;
    for (uint8_t i = 0; i < seq.size(); i++) {
        const GingoEvent& evt = seq.at(i);
        float beats = evt.duration().beats();
        int blockX = tlX + (int)(xOffset / totalB * tlW);
        int blockW = (int)(beats / totalB * tlW) - 2;
        if (blockW < 6) blockW = 6;

        uint16_t blockColor;
        switch (evt.type()) {
            case EVENT_NOTE:  blockColor = C_EVT_NOTE;  break;
            case EVENT_CHORD: blockColor = C_EVT_CHORD; break;
            default:          blockColor = C_EVT_REST;   break;
        }

        tft.fillRoundRect(blockX, tlY + 2, blockW, tlH - 4, 4, blockColor);

        // Label inside block
        tft.setTextSize(1);
        tft.setTextColor(0x0000, blockColor);
        int labelX = blockX + 3;
        int labelY = tlY + tlH / 2 - 3;

        if (evt.type() == EVENT_NOTE) {
            tft.setCursor(labelX, labelY);
            tft.print(evt.note().name());
        } else if (evt.type() == EVENT_CHORD) {
            tft.setCursor(labelX, labelY);
            tft.print(evt.chord().name());
        } else {
            tft.setCursor(labelX, labelY);
            tft.print("rest");
        }

        xOffset += beats;
    }

    // Bottom stats
    tft.setTextSize(1);
    tft.setTextColor(C_DIM, C_BG);
    tft.setCursor(12, SCR_H - 34);
    tft.print("Events: ");
    tft.setTextColor(C_TEXT, C_BG);
    tft.print(seq.size());
    tft.setTextColor(C_DIM, C_BG);
    tft.print("  Duration: ");
    tft.setTextColor(C_TEXT, C_BG);
    tft.print(totalB, 1);
    tft.print(" beats  ");
    tft.print(totalS, 1);
    tft.print("s");
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

void setup() {
    // Buttons
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);

    // Backlight
    pinMode(TFT_BL_PIN, OUTPUT);
    digitalWrite(TFT_BL_PIN, HIGH);

    // Battery power support (GPIO 15 enables power)
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);

    // Display init
    tft.init();
    tft.setRotation(1);  // landscape
    tft.fillScreen(C_BG);

    Serial.begin(115200);
    Serial.println("Gingoduino T-Display-S3 Explorer");
}

void loop() {
    unsigned long now = millis();

    // LEFT button — switch page
    if (digitalRead(BTN_LEFT) == LOW && (now - lastBtnLeft) > DEBOUNCE_MS) {
        lastBtnLeft = now;
        currentPage = (Page)(((uint8_t)currentPage + 1) % PAGE_COUNT);
        itemIdx = 0;
        needRedraw = true;
    }

    // RIGHT button — next item
    if (digitalRead(BTN_RIGHT) == LOW && (now - lastBtnRight) > DEBOUNCE_MS) {
        lastBtnRight = now;
        itemIdx++;
        needRedraw = true;
    }

    // Redraw
    if (needRedraw) {
        needRedraw = false;
        switch (currentPage) {
            case PAGE_NOTE:      drawNotePage();       break;
            case PAGE_INTERVAL:  drawIntervalPage();   break;
            case PAGE_CHORD:     drawChordPage();      break;
            case PAGE_SCALE:     drawScalePage();      break;
            case PAGE_FIELD:     drawFieldPage();      break;
            case PAGE_FRETBOARD: drawFretboardPage();  break;
            case PAGE_SEQUENCE:  drawSequencePage();    break;
            default: break;
        }
    }

    delay(10);
}
