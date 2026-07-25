#include "roadforge/engine/Engine.hpp"

#include "roadforge/core/Log.hpp"

#include <algorithm>

namespace roadforge::engine {

namespace {
constexpr double kNanosToSeconds = 1.0 / 1'000'000'000.0;
constexpr double kMaxDeltaSeconds = 1.0 / 15.0;
} // namespace

Engine::Engine()
    : vehiclePhysicsBackend_(physicsWorld_) {
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
    gameSimulation_.resetNewGame();
    world_.reset();
    world_.createDebugRoadEntity();
    vehicleController_.reset();
    physicsWorld_.reset();
    math::Transform initialVehicleTransform{};
    initialVehicleTransform.position = { vehicleController_.state().positionX, 0.0F, vehicleController_.state().positionZ };
    vehiclePhysicsBackend_.reset(physics::makeDefaultBusPhysicsConfig(), initialVehicleTransform);
    vehiclePhysicsTelemetry_ = {};
    vehicleEventCollector_.reset();
    frameStats_.reset();
    previousFrameTimeNanos_ = 0;
    simulationClock_.reset();
    appTimeSeconds_ = 0.0;
    fixedUpdateCounter_ = 0;
    droppedTimeEvents_ = 0;
    cameraToggleWasDown_ = false;
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


void Engine::onTouchState(int32_t action, int32_t pointerCount, const float* xs, const float* ys) {
    std::lock_guard<std::mutex> lock(mutex_);
    inputSystem_.handleTouchState(action, pointerCount, xs, ys);
    const input::InputSnapshot& input = inputSystem_.snapshot();
    RF_LOGD("TouchState action=%d pointers=%d steer=%.2f throttle=%.1f brake=%.1f",
            action,
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

    const input::InputSnapshot& input = inputSystem_.snapshot();
    if (input.cameraToggle && !cameraToggleWasDown_) {
        world_.toggleCameraMode();
        RF_LOGI("Camera mode toggled");
    }
    cameraToggleWasDown_ = input.cameraToggle;

    vehicleController_.setCommand(vehicle::VehicleController::commandFromInput(input));

    const core::SimulationClock::AdvanceResult simulationStep = simulationClock_.advance(deltaSeconds);
    for (uint32_t step = 0; step < simulationStep.fixedSteps; ++step) {
        vehicleController_.fixedUpdate(simulationStep.fixedDeltaSeconds);
        vehicle::VehicleState vehicleState = vehicleController_.state();
        math::Transform vehicleTransform{};
        vehicleTransform.position = { vehicleState.positionX, 0.0F, vehicleState.positionZ };
        vehicleTransform.rotation = math::quatFromAxisAngle({ 0.0F, 1.0F, 0.0F }, vehicleState.headingRadians);
        vehiclePhysicsBackend_.step(vehicleController_.command(), simulationStep.fixedDeltaSeconds, vehicleTransform, vehicleState, vehiclePhysicsTelemetry_);
        physicsWorld_.step(simulationStep.fixedDeltaSeconds);
        vehicleController_.overrideState(vehicleState);
        vehicleEventCollector_.update(vehicleController_.state());
        gameSimulation_.fixedUpdate(simulationStep.fixedDeltaSeconds, vehicleController_.state());
        for (const vehicle::VehicleEvent& event : vehicleEventCollector_.events()) {
            RF_LOGI("VehicleEvent type=%u value=%.2f odometer=%.1f trip=%.1f",
                    static_cast<unsigned>(event.type),
                    event.value,
                    event.odometerMeters,
                    event.tripSeconds);
        }
        world_.fixedUpdate(simulationStep.fixedDeltaSeconds, vehicleController_.state());
        ++fixedUpdateCounter_;
    }

    if (simulationStep.droppedExcessTime) {
        ++droppedTimeEvents_;
        RF_LOGW("Simulation clock dropped excess accumulated time; count=%llu", static_cast<unsigned long long>(droppedTimeEvents_));
    }

    frameStats_.recordFrame(deltaSeconds, simulationStep.fixedSteps, simulationStep.droppedExcessTime);
    const core::FrameStatsSnapshot stats = frameStats_.snapshot();
    if (stats.totalFrames > 0 && (stats.totalFrames % 240U) == 0U) {
        const vehicle::VehicleState& vehicleState = vehicleController_.state();
        RF_LOGI("FrameStats fps=%.1f frameMs=%.2f fixedSteps=%.2f dropped=%llu vehicle throttle=%.2f brake=%.2f steer=%.2f rpm=%.0f",
                stats.estimatedFps,
                stats.averageFrameMs,
                stats.averageFixedSteps,
                static_cast<unsigned long long>(stats.droppedTimeEvents),
                vehicleState.throttle,
                vehicleState.brake,
                vehicleState.steering,
                vehicleState.engineRpm);
        const vehicle::DrivingTelemetrySnapshot& drivingTelemetry = vehicleEventCollector_.telemetry();
        RF_LOGI("VehiclePhysics grounded=%u compression=%.2f longSlip=%.2f latSlip=%.2f speedKmh=%.1f fuel=%.0f%% damage=%.0f%% offRoad=%d",
                drivingTelemetry.groundedWheelCount,
                drivingTelemetry.suspensionCompression,
                drivingTelemetry.longitudinalSlip,
                drivingTelemetry.lateralSlip,
                drivingTelemetry.speedKmh,
                drivingTelemetry.fuel01 * 100.0F,
                drivingTelemetry.damage01 * 100.0F,
                drivingTelemetry.offRoad ? 1 : 0);
        const game::GameSnapshot& gameSnapshot = gameSimulation_.snapshot();
        RF_LOGI("GameSim tripState=%u progress=%.3f passengers=%u cash=%.2f reputation=%.2f weather=%u traffic=%.2f",
                static_cast<unsigned>(gameSnapshot.activeTrip.state),
                gameSnapshot.activeTrip.progress01,
                gameSnapshot.activeTrip.passengerCount,
                static_cast<double>(gameSnapshot.company.cashCents) / 100.0,
                gameSnapshot.company.reputation01,
                static_cast<unsigned>(gameSnapshot.weather.kind),
                gameSnapshot.traffic.density01);
    }

    world_.collectRenderProxies(worldRenderProxies_);
    debugRenderProxies_.clear();
    debugRenderProxies_.reserve(worldRenderProxies_.size());
    for (const world::RenderProxy& proxy : worldRenderProxies_) {
        renderer::DebugMeshKind renderMeshKind = renderer::DebugMeshKind::RoadSurface;
        if (proxy.meshKind == world::MeshKind::BusPlaceholder) {
            renderMeshKind = renderer::DebugMeshKind::BusPlaceholder;
        }
        debugRenderProxies_.push_back(renderer::DebugRenderProxy{
            proxy.transform,
            renderMeshKind,
            proxy.boundingRadius,
            proxy.visible,
        });
    }

    renderer_.setInputDebug(input.steering, input.throttle, input.brake, input.primaryTouchDown);
    renderer_.setFrameStats(stats);
    renderer_.setDebugRenderProxies(debugRenderProxies_);
    renderer_.setSimulationTiming(appTimeSeconds_, fixedUpdateCounter_, simulationStep.interpolationAlpha);
    renderer_.setDebugRoadTransform(world_.debugRoadTransform());
    const world::DebugCamera& camera = world_.debugCamera();
    renderer_.setDebugCamera(camera.eye, camera.target, camera.up, camera.fovYRadians);
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
