// Gingoduino — Music Theory Library for Embedded Systems
// GingoChordComparison: multidimensional chord pair analysis.
//
// Compares two chords in absolute (context-free) terms across 17 dimensions:
// note overlap, root geometry, quality match, set-theory relationships,
// voice leading, Neo-Riemannian transformations, and Forte interval vectors.
//
// All computation is integer-only, zero-heap, and safe on ESP32 and above.
// Requires Tier 3 (GINGODUINO_HAS_COMPARISON).
//
// Theoretical references:
//   Neo-Riemannian:   Cohn (2012), "Audacious Euphony"
//   Interval vectors: Forte (1973), "The Structure of Atonal Music"
//   Transposition:    Lewin (1987), "Generalized Musical Intervals"
//   Voice leading:    Tymoczko (2011), "A Geometry of Music"
//
// SPDX-License-Identifier: MIT

#ifndef GINGO_CHORD_COMPARISON_H
#define GINGO_CHORD_COMPARISON_H

#include "gingoduino_config.h"

#if GINGODUINO_HAS_COMPARISON

#include "gingoduino_types.h"
#include "GingoNote.h"
#include "GingoChord.h"

namespace gingoduino {

// ===========================================================================
// Enums
// ===========================================================================

/// Subset relationship between two chords' pitch class sets.
enum ChordSubsetRelation : uint8_t {
    CHORD_SUBSET_NONE   = 0,  ///< Neither is a subset of the other
    CHORD_SUBSET_A_IN_B = 1,  ///< All notes of A are present in B (A ⊆ B)
    CHORD_SUBSET_B_IN_A = 2,  ///< All notes of B are present in A (B ⊆ A)
    CHORD_SUBSET_EQUAL  = 3   ///< Same pitch class sets (implies enharmonic)
};

/// Neo-Riemannian transformation between two triads.
///
/// Single operations (Cohn 2012):
///   P (Parallel): same root, toggle major ↔ minor.
///   L (Leading-tone exchange): move one note by a semitone.
///   R (Relative): share two common tones with the relative triad.
///
/// Two-step compositions (read left-to-right):
///   NEO_RP = R then P, NEO_LP = L then P, etc.
///
/// Only applies to triads (size == 3) of type "M" or "m".
/// Returns NEO_NONE for non-triads, augmented, diminished, etc.
enum NeoRiemannianTransform : uint8_t {
    NEO_NONE = 0,
    // Single operations
    NEO_P  = 1,  ///< Parallel  (CM ↔ Cm)
    NEO_L  = 2,  ///< Leading-tone (CM ↔ Em)
    NEO_R  = 3,  ///< Relative  (CM ↔ Am)
    // Two-step compositions
    NEO_RP = 4,  ///< R then P  (CM → AM)
    NEO_RL = 5,  ///< R then L  (CM → FM)
    NEO_LP = 6,  ///< L then P  (CM → EM)
    NEO_LR = 7,  ///< L then R  (CM → GM)
    NEO_PR = 8,  ///< P then R  (CM → EbM)
    NEO_PL = 9   ///< P then L  (CM → AbM)
};

// ===========================================================================
// GingoChordComparison
// ===========================================================================

/// Multidimensional comparison of two chords (context-free).
///
/// Usage:
///   GingoChord cm("CM"), am("Am");
///   GingoChordComparison cmp = GingoChordComparison::compute(cm, am);
///
///   cmp.common_count              // 2 (C and E shared)
///   cmp.root_distance             // 3
///   cmp.transformation            // NEO_R
///   cmp.interval_vector_a[0..5]   // {0,0,1,1,1,0} (Forte for triads)
///   cmp.same_interval_vector      // true  (CM and Am share the same vector)
///   cmp.voice_leading             // 2 (minimal semitone movement)
struct GingoChordComparison {

    // ── Note overlap ─────────────────────────────────────────────────────────

    /// Pitch classes present in both chords (12-bit bitmask, bit i = semitone i).
    uint16_t common_pc;

    /// Pitch classes present only in chord A.
    uint16_t exclusive_a_pc;

    /// Pitch classes present only in chord B.
    uint16_t exclusive_b_pc;

    /// Number of shared pitch classes (popcount of common_pc).
    uint8_t  common_count;

    // ── Root geometry ─────────────────────────────────────────────────────────

    /// Shortest arc distance between roots on the chromatic circle (0-6).
    uint8_t  root_distance;

    /// Signed root interval (B_root - A_root), normalized to -6..+6.
    /// Positive = B's root is higher (ascending interval).
    int8_t   root_direction;

    // ── Quality match ─────────────────────────────────────────────────────────

    /// True if both chords have the same type string (e.g. both "m7").
    bool     same_quality;

    /// True if both chords have the same number of notes.
    bool     same_size;

    /// Bitmask of interval semitones-from-root present in both chords.
    /// Bit i = semitone interval i (0-11) appears in both chords' structure.
    /// Example: CM and Am both have intervals 0 (root) and 7 (fifth) → bit 0 and bit 7.
    uint16_t common_interval_mask;

    // ── Set theory ────────────────────────────────────────────────────────────

    /// True if both chords have identical pitch class sets.
    bool     enharmonic;

    /// Subset relationship (see ChordSubsetRelation enum).
    uint8_t  subset;

    /// True if both chords share the same pitch class set but have different roots.
    bool     inversion;

    /// Transposition index T_n (Lewin 1987): the n such that rotating A's pitch
    /// class set by n semitones yields B's pitch class set. Range 0-11.
    /// -1 if no transposition relationship exists.
    int8_t   transposition;

    // ── Voice leading ─────────────────────────────────────────────────────────

    /// Minimum total semitone movement for optimal note pairing (Tymoczko 2011).
    /// Uses chromatic distance (shortest arc on the circle of semitones).
    /// -1 if the chords have different sizes.
    int8_t   voice_leading;

    // ── Neo-Riemannian ────────────────────────────────────────────────────────

    /// Neo-Riemannian transformation connecting A to B (see NeoRiemannianTransform).
    /// NEO_NONE if chords are not triads, or no transformation found within 2 steps.
    uint8_t  transformation;

    // ── Forte interval vectors ─────────────────────────────────────────────────

    /// Forte interval-class vector for chord A.
    /// Index i counts pairs of notes separated by interval class (i+1) (ic1-ic6).
    /// Example: major triad {C,E,G} → {0,0,1,1,1,0}.
    uint8_t  interval_vector_a[6];

    /// Forte interval-class vector for chord B.
    uint8_t  interval_vector_b[6];

    /// True if both interval vectors are equal (Z-relation candidate).
    /// Example: CM and Am both have {0,0,1,1,1,0}.
    bool     same_interval_vector;

    // ── Factory ────────────────────────────────────────────────────────────────

    /// Compute the full comparison between chords A and B.
    static GingoChordComparison compute(const GingoChord& a, const GingoChord& b);

    /// Human-readable name for a NeoRiemannianTransform enum value.
    /// Returns "P", "L", "R", "RP", "RL", "LP", "LR", "PR", "PL", or "".
    static const char* transformationName(uint8_t t);
};

} // namespace gingoduino

#endif // GINGODUINO_HAS_COMPARISON
#endif // GINGO_CHORD_COMPARISON_H
