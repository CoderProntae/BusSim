#pragma once

#include "roadforge/renderer/VulkanRenderer.hpp"

#include <android/native_window.h>

#include <cstdint>
#include <mutex>

namespace roadforge::engine {

class Engine final {
public:
    Engine();
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    void onSurfaceCreated(ANativeWindow* window);
    void onSurfaceChanged(int32_t width, int32_t height);
    void onSurfaceDestroyed();
    void onPause();
    void onResume();
    void onTouch(int32_t action, float x, float y, int32_t pointerCount);
    void frame(int64_t frameTimeNanos);

private:
    void releaseWindowLocked();

    std::mutex mutex_;
    renderer::VulkanRenderer renderer_;
    ANativeWindow* window_ = nullptr;
    bool paused_ = true;
    int64_t previousFrameTimeNanos_ = 0;
};

} // namespace roadforge::engine
