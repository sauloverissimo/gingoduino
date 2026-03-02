// Gingoduino — Zero-Conflict Integration Test
//
// Verifies that Gingoduino and cmidi2 coexist without namespace or macro
// conflicts. Both headers are included in the same translation unit, and
// symbols from both libraries are used side by side.
//
// Build (from repo root, after downloading cmidi2.h):
//
//   mkdir -p extras/tests/vendor
//   curl -sSL https://raw.githubusercontent.com/atsushieno/cmidi2/main/cmidi2.h
//        -o extras/tests/vendor/cmidi2.h
//
//   g++ -std=c++11 -DGINGODUINO_TIER=3 -I. -Iextras/tests/vendor
//       -Wall -Wno-unused-variable -Wno-unused-parameter
//       -o extras/tests/test_integration extras/tests/test_integration.cpp
//
// Expected: compiles with zero errors and zero warnings.
//
// What this proves:
//   - No macro name collision between GINGODUINO_* and CMIDI2_* prefixes
//   - No namespace collision (gingoduino:: vs global cmidi2 C symbols)
//   - Both UMP structs (GingoUMP and cmidi2_ump128_t) can be used in the
//     same scope without ambiguity
//   - Chord type constants have identical numeric values in both libraries
//     (CMIDI2_UMP_CHORD_TYPE_MAJOR == 1, Gingoduino Major type == 1)
//
// SPDX-License-Identifier: MIT

// cmidi2 is a pure-C header — include first to avoid any implicit ordering
// issues with C++ stdlib headers pulled in by Gingoduino's Tier 3 features.
#include "extras/tests/vendor/cmidi2.h"

// Gingoduino full Tier 3 header (includes <functional> for Tier 3 lambdas)
#define GINGODUINO_TIER 3
#include "src/Gingoduino.h"

// Pull in all .cpp files for a single-file build (same pattern as test_native.cpp)
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
#include "src/GingoMonitor.cpp"
#include "src/GingoChordComparison.cpp"

#include <cstdio>
#include <cstring>

// ---------------------------------------------------------------------------
// Test: both UMP structs in scope simultaneously
// ---------------------------------------------------------------------------

static int testUMPCoexistence() {
    int failures = 0;

    using namespace gingoduino;

    // Gingoduino chord
    GingoChord cm("CM");
    GingoUMP gingoUmp = GingoMIDI2::chordName(cm);

    // cmidi2 UMP struct — same 4×uint32_t layout (fields: p1, p2, p3, p4)
    cmidi2_ump128_t cmidi2Ump;
    cmidi2Ump.p1 = gingoUmp.words[0];
    cmidi2Ump.p2 = gingoUmp.words[1];
    cmidi2Ump.p3 = gingoUmp.words[2];
    cmidi2Ump.p4 = gingoUmp.words[3];

    // Verify Message Type bits (bits 31-28) == 0xD (Flex Data)
    uint32_t mt = (cmidi2Ump.p1 >> 28) & 0xF;
    if (mt != (uint32_t)CMIDI2_MESSAGE_TYPE_FLEX_DATA) {
        printf("  FAIL: UMP MT should be 0xD (FLEX_DATA), got 0x%X\n", (unsigned)mt);
        failures++;
    } else {
        printf("  OK:   UMP MT=0xD (FLEX_DATA) — GingoUMP and cmidi2_ump128_t agree\n");
    }

    // Verify chord name status byte (bits 7-0 of p1) == 0x06
    uint32_t status = cmidi2Ump.p1 & 0xFF;
    if (status != (uint32_t)CMIDI2_FLEX_DATA_STATUS_SET_CHORD_NAME) {
        printf("  FAIL: chord name status should be 0x06, got 0x%02X\n", (unsigned)status);
        failures++;
    } else {
        printf("  OK:   chord name status=0x06 — matches CMIDI2_FLEX_DATA_STATUS_SET_CHORD_NAME\n");
    }

    // Verify Gingoduino's Major type value == cmidi2's CMIDI2_UMP_CHORD_TYPE_MAJOR (1)
    // Gingoduino encodes chord type in word1 bits 23-16
    uint32_t gingoChordType = (gingoUmp.words[1] >> 16) & 0xFF;
    if (gingoChordType != (uint32_t)CMIDI2_UMP_CHORD_TYPE_MAJOR) {
        printf("  FAIL: Major chord type should be %d, got %u\n",
               (int)CMIDI2_UMP_CHORD_TYPE_MAJOR, (unsigned)gingoChordType);
        failures++;
    } else {
        printf("  OK:   Major chord type=%d — GingoMIDI2 matches CMIDI2_UMP_CHORD_TYPE_MAJOR\n",
               (int)gingoChordType);
    }

    return failures;
}

// ---------------------------------------------------------------------------
// Test: keySignature UMP agrees with cmidi2 status constant
// ---------------------------------------------------------------------------

static int testKeySignatureStatus() {
    int failures = 0;

    using namespace gingoduino;

    GingoScale cMajor("C", SCALE_MAJOR);
    GingoUMP gingoUmp = GingoMIDI2::keySignature(cMajor);

    uint32_t status = gingoUmp.words[0] & 0xFF;
    if (status != (uint32_t)CMIDI2_FLEX_DATA_STATUS_SET_KEY_SIGNATURE) {
        printf("  FAIL: keySignature status should be 0x05, got 0x%02X\n", (unsigned)status);
        failures++;
    } else {
        printf("  OK:   keySignature status=0x05 — matches CMIDI2_FLEX_DATA_STATUS_SET_KEY_SIGNATURE\n");
    }

    return failures;
}

// ---------------------------------------------------------------------------
// Test: note letter encoding matches cmidi2 enums
// ---------------------------------------------------------------------------

static int testNoteLetterEncoding() {
    int failures = 0;

    using namespace gingoduino;

    // GingoMIDI2 encodes C=3 in tonic letter (bits 27-24 of word1).
    // cmidi2 defines CMIDI2_UMP_CHORD_NAME_C = 3.
    GingoChord cm("CM");
    GingoUMP ump = GingoMIDI2::chordName(cm);
    uint32_t letter = (ump.words[1] >> 24) & 0xF;

    if (letter != (uint32_t)CMIDI2_UMP_CHORD_NAME_C) {
        printf("  FAIL: C tonic letter should be %d, got %u\n",
               (int)CMIDI2_UMP_CHORD_NAME_C, (unsigned)letter);
        failures++;
    } else {
        printf("  OK:   C tonic letter=%d — GingoMIDI2 matches CMIDI2_UMP_CHORD_NAME_C\n",
               (int)letter);
    }

    // Natural accidental: both should encode as 0
    uint32_t acc = (ump.words[1] >> 28) & 0xF;
    if (acc != (uint32_t)CMIDI2_UMP_CHORD_NAME_NATURAL) {
        printf("  FAIL: natural accidental should be 0, got %u\n", (unsigned)acc);
        failures++;
    } else {
        printf("  OK:   natural accidental=0 — matches CMIDI2_UMP_CHORD_NAME_NATURAL\n");
    }

    return failures;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main() {
    printf("Gingoduino Integration Test (Gingoduino + cmidi2)\n");
    printf("==================================================\n\n");

    int failures = 0;

    printf("=== UMP coexistence ===\n");
    failures += testUMPCoexistence();

    printf("\n=== keySignature status ===\n");
    failures += testKeySignatureStatus();

    printf("\n=== Note letter encoding ===\n");
    failures += testNoteLetterEncoding();

    printf("\n==================================================\n");
    if (failures == 0) {
        printf("All integration checks passed. Zero conflicts detected.\n");
    } else {
        printf("FAILED: %d check(s) did not pass.\n", failures);
    }
    return failures > 0 ? 1 : 0;
}
