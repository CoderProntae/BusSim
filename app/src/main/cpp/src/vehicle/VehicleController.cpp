#include "roadforge/vehicle/VehicleController.hpp"

#include <algorithm>
#include <cmath>

namespace roadforge::vehicle {

namespace {
constexpr float kIdleRpm = 650.0F;
constexpr float kMaxPreviewRpm = 1800.0F;
constexpr float kMaxSteeringWheelDegrees = 540.0F;
constexpr float kPedalSlewPerSecond = 8.0F;
constexpr float kSteerSlewPerSecond = 6.0F;
} // namespace

void VehicleController::reset() {
    command_ = {};
    state_ = {};
}

VehicleCommand VehicleController::commandFromInput(const input::InputSnapshot& input) {
    VehicleCommand command{};
    command.throttle = input.primaryTouchDown ? std::clamp(input.throttle, 0.0F, 1.0F) : 0.0F;
    command.brake = input.primaryTouchDown ? std::clamp(input.brake, 0.0F, 1.0F) : 0.0F;

    const bool pedalInputActive = command.throttle > 0.0F || command.brake > 0.0F;
    command.steering = input.primaryTouchDown && !pedalInputActive ? std::clamp(input.steering, -1.0F, 1.0F) : 0.0F;
    command.retarder = 0.0F;
    command.handbrake = false;
    command.gearMode = GearMode::Drive;
    return command;
}

void VehicleController::setCommand(const VehicleCommand& command) {
    command_ = command;
    command_.throttle = std::clamp(command_.throttle, 0.0F, 1.0F);
    command_.brake = std::clamp(command_.brake, 0.0F, 1.0F);
    command_.steering = std::clamp(command_.steering, -1.0F, 1.0F);
    command_.retarder = std::clamp(command_.retarder, 0.0F, 1.0F);
}

void VehicleController::fixedUpdate(double fixedDeltaSeconds) {
    const float dt = static_cast<float>(std::clamp(fixedDeltaSeconds, 0.0, 0.1));

    state_.throttle = approach(state_.throttle, command_.throttle, kPedalSlewPerSecond * dt);
    state_.brake = approach(state_.brake, command_.brake, kPedalSlewPerSecond * dt);
    state_.steering = approach(state_.steering, command_.steering, kSteerSlewPerSecond * dt);
    state_.retarder = approach(state_.retarder, command_.retarder, kPedalSlewPerSecond * dt);
    state_.handbrake = command_.handbrake;
    state_.gearMode = command_.gearMode;
    state_.selectedGear = command_.gearMode == GearMode::Reverse ? -1 : (command_.gearMode == GearMode::Drive ? 1 : 0);

    // Preview values only. Real longitudinal dynamics start in Phase 2.2+.
    state_.speedMetersPerSecond = 0.0F;
    state_.engineRpm = kIdleRpm + (state_.throttle * (kMaxPreviewRpm - kIdleRpm));
    state_.steeringWheelDegrees = state_.steering * kMaxSteeringWheelDegrees;
}

float VehicleController::approach(float current, float target, float maxDelta) {
    const float delta = target - current;
    if (std::fabs(delta) <= maxDelta) {
        return target;
    }
    return current + (delta > 0.0F ? maxDelta : -maxDelta);
}

} // namespace roadforge::vehicle
