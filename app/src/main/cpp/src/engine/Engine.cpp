#include "roadforge/engine/Engine.hpp"

#include "roadforge/core/Log.hpp"

#include <algorithm>

namespace roadforge::engine {

namespace {
constexpr float kNanosToSeconds = 1.0F / 1'000'000'000.0F;
constexpr float kMaxDeltaSeconds = 1.0F / 15.0F;
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

    previousFrameTimeNanos_ = 0;
    if (!renderer_.initialize(window_)) {
        RF_LOGE("Vulkan renderer initialization failed");
        renderer_.shutdown();
    }
}

void Engine::onSurfaceChanged(int32_t width, int32_t height) {
    std::lock_guard<std::mutex> lock(mutex_);
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
    RF_LOGD("Touch action=%d x=%.1f y=%.1f pointers=%d", action, x, y, pointerCount);
    renderer_.setTouchPulse(0.22F);
}

void Engine::frame(int64_t frameTimeNanos) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (paused_ || !renderer_.isReady()) {
        previousFrameTimeNanos_ = frameTimeNanos;
        return;
    }

    float deltaSeconds = 1.0F / 60.0F;
    if (previousFrameTimeNanos_ > 0) {
        deltaSeconds = static_cast<float>(frameTimeNanos - previousFrameTimeNanos_) * kNanosToSeconds;
        deltaSeconds = std::clamp(deltaSeconds, 0.0F, kMaxDeltaSeconds);
    }
    previousFrameTimeNanos_ = frameTimeNanos;

    renderer_.tick(deltaSeconds);
    renderer_.drawFrame();
}

void Engine::releaseWindowLocked() {
    if (window_ != nullptr) {
        ANativeWindow_release(window_);
        window_ = nullptr;
    }
}

} // namespace roadforge::engine
