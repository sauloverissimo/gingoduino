// Gingoduino — Music Theory Library for Embedded Systems
// GingoMIDI1: output adapter implementations.
//
// SPDX-License-Identifier: MIT

#include "gingoduino_config.h"

#if GINGODUINO_HAS_MIDI1

#include "GingoMIDI1.h"
#include "GingoEvent.h"

#if GINGODUINO_HAS_SEQUENCE
#include "GingoSequence.h"
#endif

namespace gingoduino {
namespace GingoMIDI1 {

uint8_t fromEvent(const GingoEvent& event, uint8_t* buf, uint8_t maxLen) {
    if (!buf) return 0;
    if (event.type() == EVENT_REST) return 0;
    if (maxLen < 6) return 0;

    uint8_t noteNum = event.midiNumber();
    uint8_t ch = event.midiChannel() & 0x0F;

    buf[0] = 0x90 | ch;
    buf[1] = noteNum;
    buf[2] = event.velocity();

    buf[3] = 0x80 | ch;
    buf[4] = noteNum;
    buf[5] = 0;

    return 6;
}

#if GINGODUINO_HAS_SEQUENCE
uint16_t fromSequence(const GingoSequence& seq, uint8_t* buf,
                      uint16_t maxLen, uint8_t channel) {
    if (!buf || maxLen == 0) return 0;

    uint16_t offset = 0;
    for (uint8_t i = 0; i < seq.size(); i++) {
        const GingoEvent& src = seq.at(i);
        uint8_t needed = (src.type() == EVENT_REST) ? 0 : 6;
        if (offset + needed > maxLen) break;

        if (channel != 0) {
            GingoEvent overridden = src;
            overridden.setMidiChannel(channel);
            offset += fromEvent(overridden, buf + offset,
                                (uint8_t)(maxLen - offset > 255 ? 255 : maxLen - offset));
        } else {
            offset += fromEvent(src, buf + offset,
                                (uint8_t)(maxLen - offset > 255 ? 255 : maxLen - offset));
        }
    }
    return offset;
}
#endif

} // namespace GingoMIDI1
} // namespace gingoduino

#endif // GINGODUINO_HAS_MIDI1
