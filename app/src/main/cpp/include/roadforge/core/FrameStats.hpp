#pragma once

#include <cstdint>

namespace roadforge::core {

struct FrameStatsSnapshot final {
    uint64_t totalFrames = 0;
    uint64_t droppedTimeEvents = 0;
    double averageFrameMs = 0.0;
    double estimatedFps = 0.0;
    double averageFixedSteps = 0.0;
};

class FrameStats final {
public:
    void reset();
    void recordFrame(double frameDeltaSeconds, uint32_t fixedSteps, bool droppedExcessTime);

    [[nodiscard]] FrameStatsSnapshot snapshot() const;

private:
    static constexpr double kSmoothing = 0.08;

    uint64_t totalFrames_ = 0;
    uint64_t droppedTimeEvents_ = 0;
    double averageFrameSeconds_ = 1.0 / 60.0;
    double averageFixedSteps_ = 1.0;
};

} // namespace roadforge::core
