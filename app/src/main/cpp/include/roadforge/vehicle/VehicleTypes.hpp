#pragma once

#include <cstdint>

namespace roadforge::vehicle {

enum class GearMode : uint8_t {
    Park = 0,
    Reverse,
    Neutral,
    Drive,
};

struct VehicleCommand final {
    float throttle = 0.0F;   // 0..1
    float brake = 0.0F;      // 0..1
    float steering = 0.0F;   // -1..1
    float retarder = 0.0F;   // 0..1
    bool handbrake = false;
    GearMode gearMode = GearMode::Drive;
};

struct VehicleState final {
    float throttle = 0.0F;
    float brake = 0.0F;
    float steering = 0.0F;
    float retarder = 0.0F;
    bool handbrake = false;
    GearMode gearMode = GearMode::Drive;

    float speedMetersPerSecond = 0.0F;
    float engineRpm = 650.0F;
    int32_t selectedGear = 1;
    float steeringWheelDegrees = 0.0F;
    float engineTorqueNm = 0.0F;
    float driveForce = 0.0F;
    float brakeForce = 0.0F;

    float positionX = 0.0F;
    float positionZ = 3.7F;
    float headingRadians = 0.0F;
    float odometerMeters = 0.0F;
    float tripSeconds = 0.0F;

    float fuelLiters = 180.0F;
    float fuelCapacityLiters = 180.0F;
    float damage01 = 0.0F;
    float offRoad01 = 0.0F;

    uint32_t groundedWheelCount = 0;
    float averageSuspensionCompression = 0.0F;
    float longitudinalSlip = 0.0F;
    float lateralSlip = 0.0F;
};

} // namespace roadforge::vehicle
