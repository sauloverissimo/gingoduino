// Gingoduino — Music Theory Library for Embedded Systems
// GingoChordComparison: implementation.
//
// SPDX-License-Identifier: MIT

#include "GingoChordComparison.h"

#if GINGODUINO_HAS_COMPARISON

#include <stdint.h>
#include <string.h>

namespace gingoduino {

// ===========================================================================
// Internal helpers
// ===========================================================================

/// Build a 12-bit pitch class bitmask from a chord's notes.
static uint16_t pcMaskFromChord_(const GingoChord& chord) {
    GingoNote notes[GINGODUINO_MAX_CHORD_NOTES];
    uint8_t n = chord.notes(notes, GINGODUINO_MAX_CHORD_NOTES);
    uint16_t mask = 0;
    for (uint8_t i = 0; i < n; i++) {
        mask |= (uint16_t)(1u << (notes[i].semitone() % 12));
    }
    return mask;
}

/// Build a 12-bit bitmask of interval semitones from root.
/// Bit i = semitone interval i (0-11) is present in chord's structure.
static uint16_t intervalMaskFromChord_(const GingoChord& chord) {
    GingoNote notes[GINGODUINO_MAX_CHORD_NOTES];
    uint8_t n = chord.notes(notes, GINGODUINO_MAX_CHORD_NOTES);
    uint8_t root_pc = (uint8_t)(chord.root().semitone() % 12);
    uint16_t mask = 0;
    for (uint8_t i = 0; i < n; i++) {
        uint8_t interval = (uint8_t)((notes[i].semitone() % 12 - root_pc + 12) % 12);
        mask |= (uint16_t)(1u << interval);
    }
    return mask;
}

/// Count bits in a 12-bit bitmask.
static uint8_t popcount12_(uint16_t mask) {
    uint8_t count = 0;
    mask &= 0x0FFF;
    while (mask) { count += (mask & 1); mask >>= 1; }
    return count;
}

/// Chromatic distance: shortest arc on the circle of semitones (0-6).
static uint8_t chromaticDist_(uint8_t a, uint8_t b) {
    uint8_t d = (uint8_t)((b + 12 - a) % 12);
    return (d > 6) ? (uint8_t)(12 - d) : d;
}

/// Rotate a 12-bit pitch class bitmask by n semitones upward.
/// Bit i → bit (i+n)%12.
static uint16_t rotatePc_(uint16_t mask, uint8_t n) {
    uint16_t result = 0;
    n = n % 12;
    for (uint8_t i = 0; i < 12; i++) {
        if (mask & (uint16_t)(1u << i)) {
            result |= (uint16_t)(1u << ((i + n) % 12));
        }
    }
    return result;
}

/// Compute the Forte interval-class vector for a pitch class bitmask.
/// Output: iv[0..5] where iv[i] = count of note pairs with interval class (i+1).
/// Interval class ic(d) = min(d, 12-d) for chromatic distance d.
static void computeIntervalVector_(uint16_t pc_mask, uint8_t iv[6]) {
    for (uint8_t i = 0; i < 6; i++) iv[i] = 0;
    for (uint8_t i = 0; i < 12; i++) {
        if (!(pc_mask & (uint16_t)(1u << i))) continue;
        for (uint8_t j = i + 1; j < 12; j++) {
            if (!(pc_mask & (uint16_t)(1u << j))) continue;
            uint8_t d = j - i;
            uint8_t ic = (d > 6) ? (uint8_t)(12 - d) : d;
            if (ic > 0 && ic <= 6) iv[ic - 1]++;
        }
    }
}

// ===========================================================================
// Voice leading (minimum sum of chromatic distances over all pairings)
// ===========================================================================

/// In-place sort of a uint8_t array (insertion sort — stable, tiny N).
static void sortU8_(uint8_t* a, uint8_t n) {
    for (uint8_t i = 1; i < n; i++) {
        uint8_t key = a[i];
        int8_t j = (int8_t)(i - 1);
        while (j >= 0 && a[j] > key) { a[j + 1] = a[j]; j--; }
        a[j + 1] = key;
    }
}

/// Advance to next lexicographic permutation of array a[0..n-1].
/// Returns false if already at last permutation.
static bool nextPerm_(uint8_t* a, uint8_t n) {
    int8_t i = (int8_t)(n - 2);
    while (i >= 0 && a[i] >= a[i + 1]) i--;
    if (i < 0) return false;
    int8_t j = (int8_t)(n - 1);
    while (a[j] <= a[i]) j--;
    uint8_t t = a[i]; a[i] = a[j]; a[j] = t;
    uint8_t lo = (uint8_t)(i + 1), hi = (uint8_t)(n - 1);
    while (lo < hi) { t = a[lo]; a[lo] = a[hi]; a[hi] = t; lo++; hi--; }
    return true;
}

/// Compute minimum voice leading distance between two chords of equal size.
/// Returns -1 if sizes differ.
static int8_t computeVoiceLeading_(const GingoChord& a, const GingoChord& b) {
    uint8_t na = a.size(), nb = b.size();
    if (na != nb || na == 0) return -1;

    GingoNote notesA[GINGODUINO_MAX_CHORD_NOTES];
    GingoNote notesB[GINGODUINO_MAX_CHORD_NOTES];
    a.notes(notesA, GINGODUINO_MAX_CHORD_NOTES);
    b.notes(notesB, GINGODUINO_MAX_CHORD_NOTES);

    // Extract pitch classes
    uint8_t pcA[GINGODUINO_MAX_CHORD_NOTES];
    uint8_t pcB[GINGODUINO_MAX_CHORD_NOTES];
    for (uint8_t i = 0; i < na; i++) {
        pcA[i] = (uint8_t)(notesA[i].semitone() % 12);
        pcB[i] = (uint8_t)(notesB[i].semitone() % 12);
    }

    // Sort B to start from lexicographically first permutation
    sortU8_(pcB, na);

    int16_t minSum = 127;
    do {
        int16_t sum = 0;
        for (uint8_t i = 0; i < na; i++) {
            sum += chromaticDist_(pcA[i], pcB[i]);
        }
        if (sum < minSum) {
            minSum = sum;
            if (minSum == 0) break;  // can't do better
        }
    } while (nextPerm_(pcB, na));

    return (int8_t)minSum;
}

// ===========================================================================
// Neo-Riemannian detection
// ===========================================================================

/// True if chord is a pure major or minor triad.
static bool isTriad_(const GingoChord& chord, bool& isMajor) {
    if (chord.size() != 3) return false;
    const char* t = chord.type();
    if (strcmp(t, "M") == 0) { isMajor = true;  return true; }
    if (strcmp(t, "m") == 0) { isMajor = false; return true; }
    return false;
}

/// Apply a single Neo-Riemannian operation to (root, isMajor).
/// root is modified in-place; isMajor toggles.
/// Only handles NEO_P, NEO_L, NEO_R.
static void applyNeoStep_(uint8_t& root, bool& isMajor, uint8_t op) {
    switch (op) {
        case NEO_P:
            isMajor = !isMajor;
            break;
        case NEO_L:
            if (isMajor) { root = (uint8_t)((root + 4) % 12); isMajor = false; }
            else          { root = (uint8_t)((root + 8) % 12); isMajor = true;  }
            break;
        case NEO_R:
            if (isMajor) { root = (uint8_t)((root + 9) % 12); isMajor = false; }
            else          { root = (uint8_t)((root + 3) % 12); isMajor = true;  }
            break;
        default: break;
    }
}

/// Detect Neo-Riemannian transformation from A to B.
/// Returns NEO_NONE if no 1 or 2-step path is found.
static uint8_t detectNeoRiemannian_(const GingoChord& a, const GingoChord& b) {
    bool aMajor, bMajor;
    if (!isTriad_(a, aMajor) || !isTriad_(b, bMajor)) return NEO_NONE;

    uint8_t aRoot = (uint8_t)(a.root().semitone() % 12);
    uint8_t bRoot = (uint8_t)(b.root().semitone() % 12);

    // Single-step candidates
    static const uint8_t SINGLE[] = { NEO_P, NEO_L, NEO_R };
    for (uint8_t si = 0; si < 3; si++) {
        uint8_t r = aRoot; bool m = aMajor;
        applyNeoStep_(r, m, SINGLE[si]);
        if (r == bRoot && m == bMajor) return SINGLE[si];
    }

    // Two-step candidates: (first, second, enum value)
    struct TwoStep { uint8_t first; uint8_t second; uint8_t result; };
    static const TwoStep TWO[] = {
        { NEO_R, NEO_P, NEO_RP },
        { NEO_R, NEO_L, NEO_RL },
        { NEO_L, NEO_P, NEO_LP },
        { NEO_L, NEO_R, NEO_LR },
        { NEO_P, NEO_R, NEO_PR },
        { NEO_P, NEO_L, NEO_PL },
    };
    for (uint8_t ti = 0; ti < 6; ti++) {
        uint8_t r = aRoot; bool m = aMajor;
        applyNeoStep_(r, m, TWO[ti].first);
        applyNeoStep_(r, m, TWO[ti].second);
        if (r == bRoot && m == bMajor) return TWO[ti].result;
    }

    return NEO_NONE;
}

// ===========================================================================
// GingoChordComparison::compute
// ===========================================================================

GingoChordComparison GingoChordComparison::compute(const GingoChord& a,
                                                    const GingoChord& b) {
    GingoChordComparison c;
    memset(&c, 0, sizeof(c));

    // ── Pitch class sets ─────────────────────────────────────────────────────
    uint16_t pcA = pcMaskFromChord_(a);
    uint16_t pcB = pcMaskFromChord_(b);

    c.common_pc      = pcA & pcB;
    c.exclusive_a_pc = pcA & ~pcB;
    c.exclusive_b_pc = pcB & ~pcA;
    c.common_count   = popcount12_(c.common_pc);

    // ── Root geometry ─────────────────────────────────────────────────────────
    uint8_t ra = (uint8_t)(a.root().semitone() % 12);
    uint8_t rb = (uint8_t)(b.root().semitone() % 12);

    int8_t diff = (int8_t)((int)rb - (int)ra);
    if (diff > 6)  diff = (int8_t)(diff - 12);
    if (diff < -6) diff = (int8_t)(diff + 12);

    c.root_direction = diff;
    c.root_distance  = (uint8_t)(diff < 0 ? -diff : diff);

    // ── Quality match ─────────────────────────────────────────────────────────
    c.same_quality        = (strcmp(a.type(), b.type()) == 0);
    c.same_size           = (a.size() == b.size());
    c.common_interval_mask = intervalMaskFromChord_(a) & intervalMaskFromChord_(b);

    // ── Set theory ────────────────────────────────────────────────────────────
    c.enharmonic = (pcA == pcB);

    if (pcA == pcB) {
        c.subset = CHORD_SUBSET_EQUAL;
    } else if ((pcA & pcB) == pcA) {
        c.subset = CHORD_SUBSET_A_IN_B;
    } else if ((pcA & pcB) == pcB) {
        c.subset = CHORD_SUBSET_B_IN_A;
    } else {
        c.subset = CHORD_SUBSET_NONE;
    }

    c.inversion = (pcA == pcB) && (ra != rb);

    // T_n transposition: find n such that rotate(pcA, n) == pcB
    c.transposition = -1;
    for (uint8_t n = 0; n < 12; n++) {
        if (rotatePc_(pcA, n) == pcB) {
            c.transposition = (int8_t)n;
            break;
        }
    }

    // ── Voice leading ─────────────────────────────────────────────────────────
    c.voice_leading = computeVoiceLeading_(a, b);

    // ── Neo-Riemannian ────────────────────────────────────────────────────────
    c.transformation = detectNeoRiemannian_(a, b);

    // ── Forte interval vectors ─────────────────────────────────────────────────
    computeIntervalVector_(pcA, c.interval_vector_a);
    computeIntervalVector_(pcB, c.interval_vector_b);
    c.same_interval_vector =
        (memcmp(c.interval_vector_a, c.interval_vector_b, 6) == 0);

    return c;
}

// ===========================================================================
// GingoChordComparison::transformationName
// ===========================================================================

const char* GingoChordComparison::transformationName(uint8_t t) {
    switch (t) {
        case NEO_P:  return "P";
        case NEO_L:  return "L";
        case NEO_R:  return "R";
        case NEO_RP: return "RP";
        case NEO_RL: return "RL";
        case NEO_LP: return "LP";
        case NEO_LR: return "LR";
        case NEO_PR: return "PR";
        case NEO_PL: return "PL";
        default:     return "";
    }
}

} // namespace gingoduino

#endif // GINGODUINO_HAS_COMPARISON
