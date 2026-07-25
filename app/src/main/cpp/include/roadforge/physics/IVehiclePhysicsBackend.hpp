#pragma once

#include "roadforge/math/Transform.hpp"
#include "roadforge/vehicle/VehicleTypes.hpp"

#include <array>
#include <cstdint>

namespace roadforge::physics {

struct WheelSpec final {
    math::Vec3 localPosition{};
    float radiusMeters = 0.52F;
    float suspensionRestLengthMeters = 0.36F;
    float springStrength = 65000.0F;
    float damperStrength = 4500.0F;
    bool driven = false;
    bool steering = false;
};

struct VehiclePhysicsConfig final {
    float massKg = 12500.0F;
    math::Vec3 centerOfMassOffset{ 0.0F, -0.45F, 0.15F };
    float wheelBaseMeters = 6.2F;
    float trackWidthMeters = 2.45F;
    std::array<WheelSpec, 6> wheels{};
    uint32_t wheelCount = 4;
};

struct VehiclePhysicsTelemetry final {
    float speedMetersPerSecond = 0.0F;
    float longitudinalSlip = 0.0F;
    float lateralSlip = 0.0F;
    float averageSuspensionCompression = 0.0F;
    uint32_t groundedWheelCount = 0;
};

/**
 * Adapter boundary between high-level vehicle gameplay and concrete physics.
 *
 * Vehicle gameplay owns commands/state. Physics backend owns contact, suspension
 * and body integration details. This keeps Jolt/custom physics replaceable.
 */
class IVehiclePhysicsBackend {
public:
    virtual ~IVehiclePhysicsBackend() = default;

    virtual void reset(const VehiclePhysicsConfig& config, const math::Transform& initialTransform) = 0;
    virtual void step(const vehicle::VehicleCommand& command,
                      double fixedDeltaSeconds,
                      math::Transform& inOutTransform,
                      vehicle::VehicleState& inOutState,
                      VehiclePhysicsTelemetry& outTelemetry) = 0;
};

} // namespace roadforge::physics
