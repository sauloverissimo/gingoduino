// Gingoduino — Music Theory Library for Embedded Systems
// Implementation of GingoSequence.
//
// SPDX-License-Identifier: MIT

#include "GingoSequence.h"

#if GINGODUINO_HAS_SEQUENCE

namespace gingoduino {

GingoSequence::GingoSequence(const GingoTempo& tempo, const GingoTimeSig& timeSig)
    : count_(0), tempo_(tempo), timeSig_(timeSig)
{}

bool GingoSequence::add(const GingoEvent& event) {
    if (count_ >= GINGODUINO_MAX_EVENTS) return false;
    events_[count_++] = event;
    return true;
}

bool GingoSequence::remove(uint8_t index) {
    if (index >= count_) return false;
    for (uint8_t i = index; i < count_ - 1; i++) {
        events_[i] = events_[i + 1];
    }
    count_--;
    return true;
}

void GingoSequence::clear() {
    count_ = 0;
}

const GingoEvent& GingoSequence::at(uint8_t index) const {
    static const GingoEvent fallback;
    if (index >= count_) return fallback;
    return events_[index];
}

float GingoSequence::totalBeats() const {
    float total = 0.0f;
    for (uint8_t i = 0; i < count_; i++) {
        total += events_[i].duration().beats();
    }
    return total;
}

float GingoSequence::totalSeconds() const {
    float total = 0.0f;
    for (uint8_t i = 0; i < count_; i++) {
        total += tempo_.seconds(events_[i].duration());
    }
    return total;
}

float GingoSequence::barCount() const {
    float beatsPerBar = timeSig_.barDuration().beats();
    if (beatsPerBar <= 0.0f) return 0.0f;
    return totalBeats() / beatsPerBar;
}

void GingoSequence::transpose(int8_t semitones) {
    for (uint8_t i = 0; i < count_; i++) {
        events_[i] = events_[i].transpose(semitones);
    }
}

} // namespace gingoduino

#endif // GINGODUINO_HAS_SEQUENCE
