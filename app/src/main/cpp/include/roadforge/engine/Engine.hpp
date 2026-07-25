#pragma once

#include "roadforge/core/FrameStats.hpp"
#include "roadforge/core/SimulationClock.hpp"
#include "roadforge/input/InputSystem.hpp"
#include "roadforge/renderer/VulkanRenderer.hpp"
#include "roadforge/vehicle/VehicleController.hpp"
#include "roadforge/world/World.hpp"

#include <android/native_window.h>

#include <cstdint>
#include <mutex>
#include <vector>

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
    void onTouchState(int32_t action, int32_t pointerCount, const float* xs, const float* ys);
    void frame(int64_t frameTimeNanos);

private:
    void releaseWindowLocked();

    std::mutex mutex_;
    renderer::VulkanRenderer renderer_;
    core::SimulationClock simulationClock_;
    core::FrameStats frameStats_;
    input::InputSystem inputSystem_;
    world::World world_;
    vehicle::VehicleController vehicleController_;
    std::vector<world::RenderProxy> worldRenderProxies_;
    std::vector<renderer::DebugRenderProxy> debugRenderProxies_;
    ANativeWindow* window_ = nullptr;
    bool paused_ = true;
    int32_t surfaceWidth_ = 1;
    int32_t surfaceHeight_ = 1;
    int64_t previousFrameTimeNanos_ = 0;
    double appTimeSeconds_ = 0.0;
    uint64_t fixedUpdateCounter_ = 0;
    uint64_t droppedTimeEvents_ = 0;
};

} // namespace roadforge::engine
