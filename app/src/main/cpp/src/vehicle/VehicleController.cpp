#include "roadforge/vehicle/VehicleController.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace roadforge::vehicle {

namespace {
constexpr float kIdleRpm = 650.0F;
constexpr float kMaxRpm = 2600.0F;
constexpr float kMaxSteeringWheelDegrees = 540.0F;
constexpr float kPedalSlewPerSecond = 8.0F;
constexpr float kSteerSlewPerSecond = 6.0F;
constexpr float kMaxPreviewSpeedMetersPerSecond = 22.0F; // ~79 km/h, enough for a mobile vertical slice.
constexpr float kBaseDriveAcceleration = 2.35F;
constexpr float kBrakeDeceleration = 7.25F;
constexpr float kRetarderDeceleration = 2.1F;
constexpr float kRollingDrag = 0.18F;
constexpr float kAeroDrag = 0.018F;
constexpr float kMaxTurnRateRadiansPerSecond = 0.58F;
constexpr float kFuelIdleLitersPerSecond = 0.00018F;
constexpr float kFuelLoadLitersPerSecond = 0.0022F;

constexpr std::array<float, 6> kGearSpeedThresholds = {
    0.0F, 4.0F, 8.0F, 12.5F, 16.5F, 20.5F,
};

float roadGripAt(float x, float z) {
    const bool onMainRoad = std::fabs(x) <= 2.95F && z >= -8.5F && z <= 82.5F;
    const bool onCrossRoad = std::fabs(x) <= 14.5F && z >= 24.5F && z <= 31.0F;
    const bool onTerminalPad = x >= 5.5F && x <= 15.5F && z >= 35.5F && z <= 49.5F;
    if (onMainRoad || onCrossRoad || onTerminalPad) {
        return 1.0F;
    }
    return 0.48F;
}

int32_t chooseAutomaticGear(float speedMetersPerSecond, GearMode mode) {
    if (mode == GearMode::Reverse) {
        return -1;
    }
    if (mode != GearMode::Drive) {
        return 0;
    }

    int32_t gear = 1;
    for (int32_t i = 1; i < static_cast<int32_t>(kGearSpeedThresholds.size()); ++i) {
        if (speedMetersPerSecond >= kGearSpeedThresholds[static_cast<size_t>(i)]) {
            gear = i + 1;
        }
    }
    return std::clamp(gear, 1, 6);
}

float gearAccelerationScale(int32_t gear) {
    switch (gear) {
        case 1: return 1.00F;
        case 2: return 0.82F;
        case 3: return 0.66F;
        case 4: return 0.52F;
        case 5: return 0.42F;
        case 6: return 0.34F;
        default: return 0.0F;
    }
}

} // namespace

void VehicleController::reset() {
    command_ = {};
    state_ = {};
    state_.positionX = 0.0F;
    state_.positionZ = 3.7F;
    state_.headingRadians = 0.0F;
    state_.fuelCapacityLiters = 180.0F;
    state_.fuelLiters = state_.fuelCapacityLiters;
}

VehicleCommand VehicleController::commandFromInput(const input::InputSnapshot& input) {
    VehicleCommand command{};
    const bool cameraToggleOnly = input.cameraToggle && input.throttle <= 0.0F && input.brake <= 0.0F && std::fabs(input.steering) <= 0.001F;
    command.throttle = input.primaryTouchDown && !cameraToggleOnly ? std::clamp(input.throttle, 0.0F, 1.0F) : 0.0F;
    command.brake = input.primaryTouchDown && !cameraToggleOnly ? std::clamp(input.brake, 0.0F, 1.0F) : 0.0F;
    command.steering = input.primaryTouchDown && !cameraToggleOnly ? std::clamp(input.steering, -1.0F, 1.0F) : 0.0F;
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

void VehicleController::overrideState(const VehicleState& state) {
    state_ = state;
}

void VehicleController::fixedUpdate(double fixedDeltaSeconds) {
    const float dt = static_cast<float>(std::clamp(fixedDeltaSeconds, 0.0, 0.1));

    state_.throttle = approach(state_.throttle, command_.throttle, kPedalSlewPerSecond * dt);
    state_.brake = approach(state_.brake, command_.brake, kPedalSlewPerSecond * dt);
    state_.steering = approach(state_.steering, command_.steering, kSteerSlewPerSecond * dt);
    state_.retarder = approach(state_.retarder, command_.retarder, kPedalSlewPerSecond * dt);
    state_.handbrake = command_.handbrake;
    state_.gearMode = command_.gearMode;
    state_.selectedGear = chooseAutomaticGear(state_.speedMetersPerSecond, state_.gearMode);

    const float grip = roadGripAt(state_.positionX, state_.positionZ);
    state_.offRoad01 = 1.0F - grip;

    const float gearScale = gearAccelerationScale(state_.selectedGear);
    const float driveAcceleration = state_.throttle * kBaseDriveAcceleration * gearScale * grip;
    const float brakeAcceleration = state_.brake * kBrakeDeceleration;
    const float retarderAcceleration = state_.retarder * kRetarderDeceleration;
    const float dragAcceleration = (state_.speedMetersPerSecond * kRollingDrag)
        + (state_.speedMetersPerSecond * state_.speedMetersPerSecond * kAeroDrag);
    const float acceleration = driveAcceleration - brakeAcceleration - retarderAcceleration - dragAcceleration;

    state_.speedMetersPerSecond = std::clamp(
        state_.speedMetersPerSecond + (acceleration * dt),
        0.0F,
        kMaxPreviewSpeedMetersPerSecond);

    const float speedRatio = kMaxPreviewSpeedMetersPerSecond > 0.0F
        ? std::clamp(state_.speedMetersPerSecond / kMaxPreviewSpeedMetersPerSecond, 0.0F, 1.0F)
        : 0.0F;
    const float lowSpeedSteerBoost = 0.45F + (0.55F * (1.0F - speedRatio));
    state_.headingRadians += state_.steering * speedRatio * lowSpeedSteerBoost * kMaxTurnRateRadiansPerSecond * dt;

    const float previousX = state_.positionX;
    const float previousZ = state_.positionZ;
    state_.positionX += std::sin(state_.headingRadians) * state_.speedMetersPerSecond * dt;
    state_.positionZ += std::cos(state_.headingRadians) * state_.speedMetersPerSecond * dt;
    const float dx = state_.positionX - previousX;
    const float dz = state_.positionZ - previousZ;
    state_.odometerMeters += std::sqrt((dx * dx) + (dz * dz));
    state_.tripSeconds += dt;

    const float gearBaseRpm = state_.selectedGear > 0 ? (static_cast<float>(state_.selectedGear - 1) * 110.0F) : 0.0F;
    state_.engineRpm = std::clamp(kIdleRpm + gearBaseRpm + (state_.throttle * 850.0F) + (speedRatio * 850.0F), kIdleRpm, kMaxRpm);
    state_.steeringWheelDegrees = state_.steering * kMaxSteeringWheelDegrees;
    state_.engineTorqueNm = state_.throttle * (1450.0F - (speedRatio * 420.0F));
    state_.driveForce = driveAcceleration;
    state_.brakeForce = brakeAcceleration + retarderAcceleration;

    const float fuelUse = (kFuelIdleLitersPerSecond + (state_.throttle * kFuelLoadLitersPerSecond) + (speedRatio * 0.0006F)) * dt;
    state_.fuelLiters = std::max(0.0F, state_.fuelLiters - fuelUse);

    if (state_.offRoad01 > 0.0F && state_.speedMetersPerSecond > 5.0F) {
        state_.damage01 = std::clamp(state_.damage01 + (state_.offRoad01 * state_.speedMetersPerSecond * 0.000035F * dt), 0.0F, 1.0F);
    }
}

float VehicleController::approach(float current, float target, float maxDelta) {
    const float delta = target - current;
    if (std::fabs(delta) <= maxDelta) {
        return target;
    }
    return current + (delta > 0.0F ? maxDelta : -maxDelta);
}

} // namespace roadforge::vehicle
