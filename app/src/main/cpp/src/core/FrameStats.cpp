#include "roadforge/core/FrameStats.hpp"

#include <algorithm>

namespace roadforge::core {

void FrameStats::reset() {
    totalFrames_ = 0;
    droppedTimeEvents_ = 0;
    averageFrameSeconds_ = 1.0 / 60.0;
    averageFixedSteps_ = 1.0;
}

void FrameStats::recordFrame(double frameDeltaSeconds, uint32_t fixedSteps, bool droppedExcessTime) {
    const double safeDelta = std::clamp(frameDeltaSeconds, 0.0, 0.25);
    const double safeSteps = static_cast<double>(fixedSteps);

    if (totalFrames_ == 0) {
        averageFrameSeconds_ = safeDelta > 0.0 ? safeDelta : (1.0 / 60.0);
        averageFixedSteps_ = safeSteps;
    } else {
        averageFrameSeconds_ = (averageFrameSeconds_ * (1.0 - kSmoothing)) + (safeDelta * kSmoothing);
        averageFixedSteps_ = (averageFixedSteps_ * (1.0 - kSmoothing)) + (safeSteps * kSmoothing);
    }

    ++totalFrames_;
    if (droppedExcessTime) {
        ++droppedTimeEvents_;
    }
}

FrameStatsSnapshot FrameStats::snapshot() const {
    FrameStatsSnapshot result{};
    result.totalFrames = totalFrames_;
    result.droppedTimeEvents = droppedTimeEvents_;
    result.averageFrameMs = averageFrameSeconds_ * 1000.0;
    result.estimatedFps = averageFrameSeconds_ > 0.000001 ? (1.0 / averageFrameSeconds_) : 0.0;
    result.averageFixedSteps = averageFixedSteps_;
    return result;
}

} // namespace roadforge::core
