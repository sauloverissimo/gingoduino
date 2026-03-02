// Gingoduino — Music Theory Library for Embedded Systems
// GingoMIDI2: UMP Flex Data generator + MIDI-CI capability announcement.
//
// Generates Universal MIDI Packet (UMP) Flex Data messages from gingoduino
// theory objects. Output is bit-for-bit compatible with cmidi2 and
// AM_MIDI2.0Lib because both follow the same MIDI 2.0 spec bit layout.
//
// UMP Flex Data (Message Type 0xD) — Word 0 layout:
//   bits 31-28: MT = 0xD
//   bits 27-24: Group (0-15)
//   bits 23-22: Format = 0b00 (complete in one UMP)
//   bits 21-20: Addressing = 0b01 (channel-addressed)
//   bits 19-16: Channel (0-15)
//   bits 15-8:  Status Bank
//   bits  7-0:  Status
//
// Chord type values match MIDI 2.0 spec and cmidi2 enums exactly:
//   CMIDI2_UMP_CHORD_TYPE_MAJOR=1, MINOR=7, DOMINANT=13, etc.
//
// Reference: MIDI 2.0 UMP spec v1.1.2, cmidi2.h (atsushieno/cmidi2)
//
// SPDX-License-Identifier: MIT

#ifndef GINGO_MIDI2_H
#define GINGO_MIDI2_H

#include "gingoduino_config.h"

#if GINGODUINO_HAS_MIDI2

#include "gingoduino_types.h"
#include "GingoNote.h"
#include "GingoChord.h"
#include "GingoScale.h"
#include "GingoNoteContext.h"
#include "GingoMonitor.h"

namespace gingoduino {

// ===========================================================================
// GingoUMP — 128-bit Universal MIDI Packet (4 x 32-bit words)
// ===========================================================================

/// A 128-bit Universal MIDI Packet.
///
/// Stores four 32-bit UMP words in host byte order. Use toBytesBE() /
/// byteCount() to serialize to big-endian bytes for wire transmission.
///
/// Compatible with:
///   • cmidi2: two uint64_t pairs (words[0]<<32|words[1], words[2]<<32|words[3])
///   • AM_MIDI2.0Lib UMP struct: copy words[0..3] into UMP.word0..word3
///   • tusb_ump: tud_midi2_send(ump.words, ump.wordCount)
struct GingoUMP {
    uint32_t words[4];
    uint8_t  wordCount;  ///< Number of valid words (4 for all Flex Data msgs)

    GingoUMP() : wordCount(0) {
        words[0] = words[1] = words[2] = words[3] = 0;
    }

    /// Total byte count (wordCount * 4).
    uint8_t byteCount() const { return wordCount * 4; }

    /// Serialize word[i] as 4 big-endian bytes into dst.
    /// Returns number of bytes written (always 4 per word).
    uint8_t writeWordBE(uint8_t wordIdx, uint8_t* dst) const {
        if (wordIdx >= wordCount) return 0;
        uint32_t w = words[wordIdx];
        dst[0] = (uint8_t)(w >> 24);
        dst[1] = (uint8_t)(w >> 16);
        dst[2] = (uint8_t)(w >>  8);
        dst[3] = (uint8_t)(w      );
        return 4;
    }

    /// Serialize all words as big-endian bytes into buf (must hold byteCount bytes).
    /// Returns total bytes written.
    uint8_t toBytesBE(uint8_t* buf, uint8_t maxLen) const {
        uint8_t written = 0;
        for (uint8_t i = 0; i < wordCount && written + 4 <= maxLen; i++) {
            written += writeWordBE(i, buf + written);
        }
        return written;
    }
};

// ===========================================================================
// GingoMIDI2 — UMP Flex Data factory (static methods, header-only)
// ===========================================================================

/// Generates MIDI 2.0 UMP Flex Data messages from gingoduino objects.
///
/// All methods are static — no instance required.
///
/// Examples:
///   GingoChord chord("Am7");
///   GingoUMP ump = GingoMIDI2::chordName(chord);
///   // ump.words[0] = 0xD010_0006 (Flex Data, channel 0, chord name)
///   // ump.words[1] = tonic=A, type=minor7, no bass/alterations
///
///   GingoScale scale("G", SCALE_MAJOR);
///   GingoUMP keySig = GingoMIDI2::keySignature(scale);
class GingoMIDI2 {
public:
    // -----------------------------------------------------------------------
    // Flex Data Chord Name (StatusBank=0x00, Status=0x06)
    // Compatible with cmidi2_ump_flex_data_set_chord_name()
    // -----------------------------------------------------------------------

    /// Generate a Flex Data Chord Name UMP message.
    /// @param chord    Source chord (root + type used for tonic and chord type).
    /// @param group    MIDI 2.0 group (0-15, default 0).
    /// @param channel  MIDI 2.0 channel (0-15, default 0).
    static GingoUMP chordName(const GingoChord& chord,
                               uint8_t group = 0, uint8_t channel = 0) {
        GingoUMP ump;
        ump.wordCount = 4;
        ump.words[0] = makeWord0_(0x00, 0x06, group, channel);

        // Tonic encoding: note letter (A=1..G=7) + accidental
        uint8_t tonicLetter, tonicAcc;
        noteToMIDI2Tonic_(chord.root(), tonicLetter, tonicAcc);

        uint8_t chordType = chordTypeForName_(chord.type());

        // Word 1: sharpsFlats[31:28] + chordTonic[27:24] + chordType[23:16]
        //         alter1Type[15:12] + alter1Degree[11:8] + alter2Type[7:4] + alter2Degree[3:0]
        ump.words[1] = ((uint32_t)tonicAcc   << 28)
                     | ((uint32_t)tonicLetter << 24)
                     | ((uint32_t)chordType   << 16);
        // Words 2-3: no alterations, no bass note override
        ump.words[2] = 0;
        ump.words[3] = 0;
        return ump;
    }

    // -----------------------------------------------------------------------
    // Flex Data Key Signature (StatusBank=0x00, Status=0x05)
    // -----------------------------------------------------------------------

    /// Generate a Flex Data Key Signature UMP message.
    /// @param scale    Source scale (tonic + key signature used).
    /// @param group    MIDI 2.0 group (0-15, default 0).
    /// @param channel  MIDI 2.0 channel (0-15, default 0).
    static GingoUMP keySignature(const GingoScale& scale,
                                  uint8_t group = 0, uint8_t channel = 0) {
        GingoUMP ump;
        ump.wordCount = 4;
        ump.words[0] = makeWord0_(0x00, 0x05, group, channel);

        uint8_t tonicLetter, tonicAcc;
        noteToMIDI2Tonic_(scale.tonic(), tonicLetter, tonicAcc);

        // Scale type for key sig: 0=major, 1=minor (natural), 2=harmonic, 3=melodic
        uint8_t scaleMode = scaleTypeToMIDI2Mode_(scale.parent());

        // Word 1: tonicSharpsFlats[31:28] + tonicNote[27:24] + scaleType[23:16] + reserved[15:0]
        ump.words[1] = ((uint32_t)tonicAcc    << 28)
                     | ((uint32_t)tonicLetter  << 24)
                     | ((uint32_t)scaleMode    << 16);
        ump.words[2] = 0;
        ump.words[3] = 0;
        return ump;
    }

    // -----------------------------------------------------------------------
    // Per-note context as Assignable Per-Note Controller (MIDI 2.0 Type 0x4)
    // Encodes scale degree + harmonic function as 32-bit per-note RCC value.
    // -----------------------------------------------------------------------

    /// Generate a Per-Note Assignable Controller UMP for harmonic context.
    /// @param midiNoteNum  MIDI note number being annotated (0-127).
    /// @param ctx          Per-note context from GingoField::noteContext().
    /// @param group        MIDI 2.0 group (0-15, default 0).
    /// @param channel      MIDI 2.0 channel (0-15, default 0).
    static GingoUMP perNoteController(uint8_t midiNoteNum,
                                       const GingoNoteContext& ctx,
                                       uint8_t group = 0, uint8_t channel = 0) {
        GingoUMP ump;
        ump.wordCount = 2;  // per-note CC is 64-bit (2 words)

        // Word 0: MT=0x4 | Group | Channel | Note | Index=0 (bank 0, RCC)
        ump.words[0] = ((uint32_t)0x4 << 28)
                     | ((uint32_t)(group   & 0xF) << 24)
                     | ((uint32_t)0x1              << 20)  // opcode: assignable per-note ctrl
                     | ((uint32_t)(channel & 0xF) << 16)
                     | ((uint32_t)midiNoteNum       <<  8)
                     | ((uint32_t)0x00);  // controller index 0 = gingoduino degree/function

        // Word 1: 32-bit controller value
        // Bits 31-24: scale degree (1-7, 0=not in scale)
        // Bits 23-16: harmonic function (0=T, 1=S, 2=D)
        // Bits 15-8:  interval semitones from tonic (0-11)
        // Bits  7-0:  inScale flag (1=in scale, 0=outside)
        ump.words[1] = ((uint32_t)ctx.degree              << 24)
                     | ((uint32_t)(ctx.function & 0xFF)   << 16)
                     | ((uint32_t)ctx.interval.semitones() <<  8)
                     | ((uint32_t)(ctx.inScale ? 1 : 0));
        ump.words[2] = 0;
        ump.words[3] = 0;
        return ump;
    }

    // -----------------------------------------------------------------------
    // UMP input dispatch → GingoMonitor
    // -----------------------------------------------------------------------

    /// Dispatch an incoming UMP packet to a GingoMonitor.
    ///
    /// Handles Note On, Note Off, Sustain Pedal (CC64), and All Notes Off
    /// (CC123) for both MIDI 1.0 (MT=0x2) and MIDI 2.0 (MT=0x4) messages.
    /// All groups and channels are accepted.
    ///
    /// This closes the integration loop with AM_MIDI2.0Lib and cmidi2:
    ///
    ///   // AM_MIDI2.0Lib — feed UMP directly into gingoduino:
    ///   processor.setChannelVoiceMessage([&mon](UMP& u) {
    ///       GingoMIDI2::dispatch(u.getUMPData(), mon);
    ///   });
    ///
    ///   // cmidi2 / tusb_ump — raw word buffer:
    ///   uint32_t* words = ...; // from UMP stream
    ///   GingoMIDI2::dispatch(words, mon);
    ///
    /// @param words  Pointer to the first word of the UMP packet.
    ///               Must point to at least 2 words when MT=0x4.
    /// @param mon    GingoMonitor to receive the routed events.
    /// @returns      true if the packet was handled.
    static bool dispatch(const uint32_t* words, GingoMonitor& mon) {
        uint8_t mt = (uint8_t)((words[0] >> 28) & 0xF);

        // MIDI 1.0 Channel Voice (MT=0x2, 1 word)
        // Word 0: [MT][Group][Status byte][Data1][Data2]
        if (mt == 0x2) {
            uint8_t opcode = (uint8_t)((words[0] >> 20) & 0xF);
            uint8_t data1  = (uint8_t)((words[0] >>  8) & 0x7F);
            uint8_t data2  = (uint8_t)(words[0] & 0x7F);

            if (opcode == 0x9 && data2 > 0) { mon.noteOn(data1, data2);  return true; }
            if (opcode == 0x8 || (opcode == 0x9 && data2 == 0)) { mon.noteOff(data1); return true; }
            // CC (opcode 0xB): data1=CC number, data2=value
            if (opcode == 0xB) {
                if (data1 == 64) { (data2 >= 64) ? mon.sustainOn() : mon.sustainOff(); return true; }
                if (data1 == 123) { mon.reset(); return true; }  // All Notes Off
            }
            return false;
        }

        // MIDI 2.0 Channel Voice (MT=0x4, 2 words)
        // Word 0: [MT][Group][Opcode][Channel][Index][reserved]
        // Word 1: [Value 32-bit]
        if (mt == 0x4) {
            uint8_t opcode = (uint8_t)((words[0] >> 20) & 0xF);
            uint8_t index  = (uint8_t)((words[0] >>  8) & 0x7F);

            // Note On/Off
            if (opcode == 0x9 || opcode == 0x8) {
                uint16_t vel16 = (uint16_t)((words[1] >> 16) & 0xFFFF);
                uint8_t  vel7  = (uint8_t)(vel16 >> 9);
                if (opcode == 0x9 && vel16 > 0) { mon.noteOn(index, vel7);  return true; }
                mon.noteOff(index); return true;
            }
            // CC (opcode 0xB): index=CC number, word1=32-bit value
            if (opcode == 0xB) {
                uint32_t val = words[1];
                if (index == 64) { (val >= 0x80000000U) ? mon.sustainOn() : mon.sustainOff(); return true; }
                if (index == 123) { mon.reset(); return true; }
            }
            return false;
        }

        return false;  // unhandled message type
    }

private:
    // -----------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------

    /// Build Flex Data Word 0.
    static uint32_t makeWord0_(uint8_t statusBank, uint8_t status,
                                uint8_t group, uint8_t channel) {
        return ((uint32_t)0xD         << 28)   // MT = Flex Data
             | ((uint32_t)(group   & 0xF) << 24)
             | ((uint32_t)0x0       << 22)   // Format = complete in one UMP
             | ((uint32_t)0x1       << 20)   // Addressing = channel
             | ((uint32_t)(channel & 0xF) << 16)
             | ((uint32_t)statusBank <<  8)
             | ((uint32_t)status);
    }

    /// Convert a GingoNote to cmidi2 tonic encoding.
    /// letter: A=1, B=2, C=3, D=4, E=5, F=6, G=7
    /// acc: natural=0, sharp=1, double_sharp=2, flat=0xF, double_flat=0xE
    static void noteToMIDI2Tonic_(const GingoNote& note,
                                   uint8_t& letter, uint8_t& acc) {
        const char* name = note.name();
        switch (name[0]) {
            case 'A': letter = 1; break;
            case 'B': letter = 2; break;
            case 'C': letter = 3; break;
            case 'D': letter = 4; break;
            case 'E': letter = 5; break;
            case 'F': letter = 6; break;
            case 'G': letter = 7; break;
            default:  letter = 0; break;
        }
        if      (name[1] == '#' && name[2] == '#') acc = 2;   // double sharp
        else if (name[1] == '#')                    acc = 1;   // sharp
        else if (name[1] == 'b' && name[2] == 'b') acc = 0xE; // double flat
        else if (name[1] == 'b')                    acc = 0xF; // flat
        else                                        acc = 0;   // natural
    }

    /// Map gingoduino chord type string to MIDI 2.0 chord type value.
    /// Returns 0 (Unknown) for types without a direct MIDI 2.0 mapping.
    /// Values match CMIDI2_UMP_CHORD_TYPE_* in cmidi2.h exactly.
    static uint8_t chordTypeForName_(const char* type) {
        if (!type || !type[0]) return 1;  // empty = Major

        // Struct for lookup table (sorted by name for readability)
        struct Entry { const char* name; uint8_t midi2Type; };
        static const Entry TABLE[] = {
            // Major family
            { "M",        1  },  // Major triad
            { "6",        2  },  // Major 6th
            { "7M",       3  },  // Major 7th (maj7)
            { "M9",       4  },  // Major 9th
            { "maj13",    6  },  // Major 13th
            // Minor family
            { "m",        7  },  // Minor triad
            { "m6",       8  },  // Minor 6th
            { "m7",       9  },  // Minor 7th
            { "m9",       10 },  // Minor 9th
            { "m11",      11 },  // Minor 11th
            { "m13",      12 },  // Minor 13th
            // Dominant family
            { "7",        13 },  // Dominant 7th
            { "9",        14 },  // Dominant 9th
            { "11",       15 },  // Dominant 11th
            { "13",       16 },  // Dominant 13th
            // Augmented
            { "aug",      17 },  // Augmented triad
            { "7#5",      18 },  // Augmented 7th
            { "7+5",      18 },  // Augmented 7th (alt notation)
            { "M7#5",     18 },  // Major 7th augmented
            // Diminished
            { "dim",      19 },  // Diminished triad
            { "dim7",     20 },  // Diminished 7th
            { "m7(b5)",   21 },  // Half-diminished (m7♭5)
            // Special
            { "mM7",      22 },  // Minor-Major 7th
            { "5",        24 },  // Power chord (no third)
            { "sus2",     25 },  // Suspended 2nd
            { "sus4",     26 },  // Suspended 4th
            { "sus",      26 },  // Suspended 4th (alt notation)
            { "sus7",     27 },  // 7th suspended 4th
        };

        static const uint8_t TABLE_SIZE =
            static_cast<uint8_t>(sizeof(TABLE) / sizeof(TABLE[0]));

        for (uint8_t i = 0; i < TABLE_SIZE; i++) {
            if (strcmp(type, TABLE[i].name) == 0) return TABLE[i].midi2Type;
        }
        return 0;  // Unknown — no direct MIDI 2.0 chord type
    }

    /// Map gingoduino ScaleType to MIDI 2.0 mode byte for key signature.
    static uint8_t scaleTypeToMIDI2Mode_(ScaleType st) {
        switch (st) {
            case SCALE_MAJOR:          return 0;
            case SCALE_NATURAL_MINOR:  return 1;
            case SCALE_HARMONIC_MINOR: return 2;
            case SCALE_MELODIC_MINOR:  return 3;
            default:                   return 0;
        }
    }
};

// ===========================================================================
// GingoMIDICI — MIDI-CI capability announcement (SysEx byte generators)
// ===========================================================================

/// Generates MIDI-CI SysEx message bytes for device discovery and
/// capability announcement. Output is spec-compliant MIDI-CI v2 SysEx.
///
/// MIDI-CI messages are transport-independent — the bytes can be sent
/// via any SysEx-capable transport (USB MIDI, BLE MIDI, DIN MIDI).
///
/// The gingoduino Profile ID (non-commercial, not registered):
///   [0x7D, 0x47, 0x49, 0x4E, 0x47] = non-commercial + "GING" (ASCII)
///
/// Usage with AM_MIDI2.0Lib:
///   uint8_t buf[64];
///   uint8_t len = GingoMIDICI::profileInquiryReply(buf, sizeof(buf));
///   ump_processor.sendSysEx(buf, len);
namespace GingoMIDICI {

/// Gingoduino Profile ID for MIDI-CI.
/// Non-commercial bank (0x7D) + "GING" ASCII (educational use).
static const uint8_t PROFILE_ID[5] = { 0x7D, 0x47, 0x49, 0x4E, 0x47 };

/// Default source MUID (28-bit, 7-bit bytes).
/// Users should replace this with a device-specific value.
static const uint8_t DEFAULT_MUID[4] = { 0x47, 0x49, 0x4E, 0x47 };  // "GING"

/// Broadcast destination MUID (all devices).
static const uint8_t MUID_BROADCAST[4] = { 0x7F, 0x7F, 0x7F, 0x7F };

/// MIDI-CI SysEx header.
static const uint8_t MIDI_CI_SYSEX_ID = 0x0D;

/// Write a 28-bit MUID as 4 x 7-bit bytes into dst[0..3].
inline void writeMUID(uint8_t* dst, const uint8_t src[4]) {
    dst[0] = src[0] & 0x7F;
    dst[1] = src[1] & 0x7F;
    dst[2] = src[2] & 0x7F;
    dst[3] = src[3] & 0x7F;
}

/// Generate a MIDI-CI Discovery Request (sub-ID 0x70).
/// Used to discover MIDI-CI capable devices on the network.
/// @param buf     Output buffer.
/// @param maxLen  Buffer capacity.
/// @param srcMUID Source MUID (4 x 7-bit bytes), or nullptr for default.
/// @returns       Number of bytes written, 0 if buffer too small.
inline uint8_t discoveryRequest(uint8_t* buf, uint8_t maxLen,
                                 const uint8_t* srcMUID = nullptr) {
    // F0 7E 7F 0D 70 02 <src_muid[4]> <dest_muid[4]> <mfr_id[3]> <fam[2]> <model[2]> <ver[4]> <cicat> <maxsysex[4]> F7
    // = 1+1+1+1+1+1+4+4+3+2+2+4+1+4+1 = 31 bytes
    static const uint8_t MSG_LEN = 31;
    if (maxLen < MSG_LEN) return 0;

    uint8_t i = 0;
    buf[i++] = 0xF0;  // SysEx start
    buf[i++] = 0x7E;  // Universal SysEx
    buf[i++] = 0x7F;  // Device ID: all
    buf[i++] = MIDI_CI_SYSEX_ID;  // MIDI-CI
    buf[i++] = 0x70;  // Sub-ID: Discovery Request
    buf[i++] = 0x02;  // MIDI-CI version 2
    // Source MUID (4 bytes)
    const uint8_t* src = srcMUID ? srcMUID : DEFAULT_MUID;
    writeMUID(buf + i, src); i += 4;
    // Destination MUID: broadcast
    writeMUID(buf + i, MUID_BROADCAST); i += 4;
    // Manufacturer ID (non-commercial)
    buf[i++] = 0x7D; buf[i++] = 0x00; buf[i++] = 0x00;
    // Device family + model (gingoduino = 0x0001, 0x0001)
    buf[i++] = 0x01; buf[i++] = 0x00;  // family
    buf[i++] = 0x01; buf[i++] = 0x00;  // model
    // Version (0.3.0.0 — matches library.properties)
    buf[i++] = 0x00; buf[i++] = 0x03; buf[i++] = 0x00; buf[i++] = 0x00;
    // MIDI-CI category support: 0x0E (profiles + property exchange + process inquiry)
    buf[i++] = 0x0E;
    // Max SysEx size: 128 bytes
    buf[i++] = 0x00; buf[i++] = 0x01; buf[i++] = 0x00; buf[i++] = 0x00;
    buf[i++] = 0xF7;  // SysEx end
    return i;
}

/// Generate a MIDI-CI Profile Inquiry Reply (sub-ID 0x22).
/// Announces the gingoduino profile (chord detection + harmonic analysis).
/// @param buf      Output buffer.
/// @param maxLen   Buffer capacity.
/// @param channel  MIDI channel (0-15, or 0x7F for all channels).
/// @param srcMUID  Source MUID (4 x 7-bit bytes), or nullptr for default.
/// @returns        Number of bytes written, 0 if buffer too small.
inline uint8_t profileInquiryReply(uint8_t* buf, uint8_t maxLen,
                                    uint8_t channel = 0x7F,
                                    const uint8_t* srcMUID = nullptr) {
    // F0 7E <ch> 0D 0x22 02 <src[4]> <dest[4]> <ch> <num_en=1> <profile_id[5]> <num_dis=0> F7
    // = 1+1+1+1+1+1+4+4+1+1+5+1+1 = 23 bytes
    static const uint8_t MSG_LEN = 23;
    if (maxLen < MSG_LEN) return 0;

    uint8_t i = 0;
    buf[i++] = 0xF0;
    buf[i++] = 0x7E;
    buf[i++] = channel & 0x7F;
    buf[i++] = MIDI_CI_SYSEX_ID;
    buf[i++] = 0x22;  // Sub-ID: Profile Inquiry Reply (enabled profiles)
    buf[i++] = 0x02;  // MIDI-CI version 2
    const uint8_t* src = srcMUID ? srcMUID : DEFAULT_MUID;
    writeMUID(buf + i, src); i += 4;
    writeMUID(buf + i, MUID_BROADCAST); i += 4;
    buf[i++] = channel & 0x7F;  // channel
    buf[i++] = 0x01;  // number of enabled profiles
    buf[i++] = PROFILE_ID[0];
    buf[i++] = PROFILE_ID[1];
    buf[i++] = PROFILE_ID[2];
    buf[i++] = PROFILE_ID[3];
    buf[i++] = PROFILE_ID[4];
    buf[i++] = 0x00;  // number of disabled profiles = 0
    buf[i++] = 0xF7;
    return i;
}

/// Generate a JSON capabilities string for MIDI-CI Property Exchange.
/// Compatible with the "ResourceList" property defined in the MIDI-CI spec.
/// @param buf     Output buffer.
/// @param maxLen  Buffer capacity.
/// @returns       Number of bytes written (not null-terminated).
inline uint8_t capabilitiesJSON(char* buf, uint8_t maxLen) {
    static const char JSON[] =
        "{\"name\":\"gingoduino\","
        "\"version\":\"0.3.0\","
        "\"scales\":[\"major\",\"minor\",\"modes\"],"
        "\"chords\":[\"triad\",\"seventh\",\"ninth\"],"
        "\"features\":[\"chord_detect\",\"key_sig\","
                      "\"harmonic_func\",\"per_note\","
                      "\"field_deduce\"]}";

    uint8_t len = 0;
    const char* src = JSON;
    while (*src && len < maxLen - 1) {
        buf[len++] = *src++;
    }
    buf[len] = '\0';
    return len;
}

} // namespace GingoMIDICI

} // namespace gingoduino

#endif // GINGODUINO_HAS_MIDI2
#endif // GINGO_MIDI2_H
