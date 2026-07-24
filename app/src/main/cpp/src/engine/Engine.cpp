#include "roadforge/engine/Engine.hpp"

#include "roadforge/core/Log.hpp"

#include <algorithm>

namespace roadforge::engine {

namespace {
constexpr double kNanosToSeconds = 1.0 / 1'000'000'000.0;
constexpr double kMaxDeltaSeconds = 1.0 / 15.0;
} // namespace

Engine::Engine() {
    RF_LOGI("Engine created");
}

Engine::~Engine() {
    std::lock_guard<std::mutex> lock(mutex_);
    renderer_.shutdown();
    releaseWindowLocked();
    RF_LOGI("Engine destroyed");
}

void Engine::onSurfaceCreated(ANativeWindow* window) {
    std::lock_guard<std::mutex> lock(mutex_);
    renderer_.shutdown();
    releaseWindowLocked();

    window_ = window;
    if (window_ == nullptr) {
        RF_LOGE("Surface created with null ANativeWindow");
        return;
    }

    surfaceWidth_ = std::max(1, ANativeWindow_getWidth(window_));
    surfaceHeight_ = std::max(1, ANativeWindow_getHeight(window_));
    inputSystem_.setSurfaceSize(surfaceWidth_, surfaceHeight_);
    inputSystem_.reset();
    frameStats_.reset();
    previousFrameTimeNanos_ = 0;
    simulationClock_.reset();
    appTimeSeconds_ = 0.0;
    fixedUpdateCounter_ = 0;
    droppedTimeEvents_ = 0;
    if (!renderer_.initialize(window_)) {
        RF_LOGE("Vulkan renderer initialization failed");
        renderer_.shutdown();
    }
}

void Engine::onSurfaceChanged(int32_t width, int32_t height) {
    std::lock_guard<std::mutex> lock(mutex_);
    surfaceWidth_ = std::max(1, width);
    surfaceHeight_ = std::max(1, height);
    inputSystem_.setSurfaceSize(surfaceWidth_, surfaceHeight_);
    renderer_.onSurfaceChanged(width, height);
}

void Engine::onSurfaceDestroyed() {
    std::lock_guard<std::mutex> lock(mutex_);
    renderer_.shutdown();
    releaseWindowLocked();
    previousFrameTimeNanos_ = 0;
}

void Engine::onPause() {
    std::lock_guard<std::mutex> lock(mutex_);
    paused_ = true;
    previousFrameTimeNanos_ = 0;
    RF_LOGI("Engine paused");
}

void Engine::onResume() {
    std::lock_guard<std::mutex> lock(mutex_);
    paused_ = false;
    previousFrameTimeNanos_ = 0;
    RF_LOGI("Engine resumed");
}

void Engine::onTouch(int32_t action, float x, float y, int32_t pointerCount) {
    std::lock_guard<std::mutex> lock(mutex_);
    inputSystem_.handleTouch(action, x, y, pointerCount);
    const input::InputSnapshot& input = inputSystem_.snapshot();
    RF_LOGD("Touch action=%d x=%.1f y=%.1f pointers=%d steer=%.2f throttle=%.1f brake=%.1f",
            action,
            x,
            y,
            pointerCount,
            input.steering,
            input.throttle,
            input.brake);
    renderer_.setTouchPulse(0.22F);
    renderer_.setInputDebug(input.steering, input.throttle, input.brake, input.primaryTouchDown);
}

void Engine::frame(int64_t frameTimeNanos) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (paused_ || !renderer_.isReady()) {
        previousFrameTimeNanos_ = frameTimeNanos;
        return;
    }

    double deltaSeconds = 1.0 / 60.0;
    if (previousFrameTimeNanos_ > 0) {
        deltaSeconds = static_cast<double>(frameTimeNanos - previousFrameTimeNanos_) * kNanosToSeconds;
        deltaSeconds = std::clamp(deltaSeconds, 0.0, kMaxDeltaSeconds);
    }
    previousFrameTimeNanos_ = frameTimeNanos;
    appTimeSeconds_ += deltaSeconds;

    const core::SimulationClock::AdvanceResult simulationStep = simulationClock_.advance(deltaSeconds);
    for (uint32_t step = 0; step < simulationStep.fixedSteps; ++step) {
        ++fixedUpdateCounter_;
    }

    if (simulationStep.droppedExcessTime) {
        ++droppedTimeEvents_;
        RF_LOGW("Simulation clock dropped excess accumulated time; count=%llu", static_cast<unsigned long long>(droppedTimeEvents_));
    }

    frameStats_.recordFrame(deltaSeconds, simulationStep.fixedSteps, simulationStep.droppedExcessTime);
    const core::FrameStatsSnapshot stats = frameStats_.snapshot();
    if (stats.totalFrames > 0 && (stats.totalFrames % 240U) == 0U) {
        RF_LOGI("FrameStats fps=%.1f frameMs=%.2f fixedSteps=%.2f dropped=%llu",
                stats.estimatedFps,
                stats.averageFrameMs,
                stats.averageFixedSteps,
                static_cast<unsigned long long>(stats.droppedTimeEvents));
    }

    const input::InputSnapshot& input = inputSystem_.snapshot();
    renderer_.setInputDebug(input.steering, input.throttle, input.brake, input.primaryTouchDown);
    renderer_.setSimulationTiming(appTimeSeconds_, fixedUpdateCounter_, simulationStep.interpolationAlpha);
    renderer_.tick(static_cast<float>(deltaSeconds));
    renderer_.drawFrame();
}

void Engine::releaseWindowLocked() {
    if (window_ != nullptr) {
        ANativeWindow_release(window_);
        window_ = nullptr;
    }
}

} // namespace roadforge::engine
