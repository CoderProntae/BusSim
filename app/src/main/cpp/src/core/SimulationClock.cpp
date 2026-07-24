#include "roadforge/core/SimulationClock.hpp"

#include <algorithm>
#include <cmath>

namespace roadforge::core {

namespace {
constexpr double kMinFixedDeltaSeconds = 1.0 / 240.0;
constexpr double kMaxFixedDeltaSeconds = 1.0 / 15.0;
constexpr double kMaxAcceptedFrameDeltaSeconds = 0.25;
} // namespace

SimulationClock::SimulationClock(double fixedDeltaSeconds, uint32_t maxStepsPerFrame)
    : fixedDeltaSeconds_(std::clamp(fixedDeltaSeconds, kMinFixedDeltaSeconds, kMaxFixedDeltaSeconds)),
      maxStepsPerFrame_(std::max(1U, maxStepsPerFrame)) {}

void SimulationClock::reset() {
    accumulatorSeconds_ = 0.0;
    totalSimulatedSeconds_ = 0.0;
    tickIndex_ = 0;
}

SimulationClock::AdvanceResult SimulationClock::advance(double frameDeltaSeconds) {
    const double safeDelta = std::clamp(frameDeltaSeconds, 0.0, kMaxAcceptedFrameDeltaSeconds);
    accumulatorSeconds_ += safeDelta;

    uint32_t steps = 0;
    while (accumulatorSeconds_ >= fixedDeltaSeconds_ && steps < maxStepsPerFrame_) {
        accumulatorSeconds_ -= fixedDeltaSeconds_;
        totalSimulatedSeconds_ += fixedDeltaSeconds_;
        ++tickIndex_;
        ++steps;
    }

    bool dropped = false;
    if (accumulatorSeconds_ >= fixedDeltaSeconds_) {
        accumulatorSeconds_ = std::fmod(accumulatorSeconds_, fixedDeltaSeconds_);
        dropped = true;
    }

    AdvanceResult result{};
    result.fixedSteps = steps;
    result.fixedDeltaSeconds = fixedDeltaSeconds_;
    result.interpolationAlpha = interpolationAlpha();
    result.totalSimulatedSeconds = totalSimulatedSeconds_;
    result.tickIndex = tickIndex_;
    result.droppedExcessTime = dropped;
    return result;
}

double SimulationClock::interpolationAlpha() const {
    if (fixedDeltaSeconds_ <= 0.0) {
        return 0.0;
    }
    return std::clamp(accumulatorSeconds_ / fixedDeltaSeconds_, 0.0, 1.0);
}

} // namespace roadforge::core
