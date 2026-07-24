#pragma once

#include <cstdint>

namespace roadforge::core {

/**
 * Fixed-step simulation clock.
 *
 * Rendering may arrive at variable cadence, but vehicle physics, traffic and
 * economy-sensitive gameplay systems must advance in deterministic 60 Hz ticks.
 * This class converts variable frame delta into zero or more fixed simulation
 * ticks plus a render interpolation alpha.
 */
class SimulationClock final {
public:
    struct AdvanceResult final {
        uint32_t fixedSteps = 0;
        double fixedDeltaSeconds = 1.0 / 60.0;
        double interpolationAlpha = 0.0;
        double totalSimulatedSeconds = 0.0;
        uint64_t tickIndex = 0;
        bool droppedExcessTime = false;
    };

    explicit SimulationClock(double fixedDeltaSeconds = 1.0 / 60.0, uint32_t maxStepsPerFrame = 5);

    void reset();
    [[nodiscard]] AdvanceResult advance(double frameDeltaSeconds);

    [[nodiscard]] double fixedDeltaSeconds() const { return fixedDeltaSeconds_; }
    [[nodiscard]] uint64_t tickIndex() const { return tickIndex_; }
    [[nodiscard]] double totalSimulatedSeconds() const { return totalSimulatedSeconds_; }
    [[nodiscard]] double interpolationAlpha() const;

private:
    double fixedDeltaSeconds_ = 1.0 / 60.0;
    double accumulatorSeconds_ = 0.0;
    double totalSimulatedSeconds_ = 0.0;
    uint64_t tickIndex_ = 0;
    uint32_t maxStepsPerFrame_ = 5;
};

} // namespace roadforge::core
