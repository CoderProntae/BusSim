#include "roadforge/physics/RaycastVehicleBackend.hpp"

#include <algorithm>
#include <cmath>

namespace roadforge::physics {

namespace {

math::Vec3 rotateYaw(const math::Vec3& v, float headingRadians) {
    const float s = std::sin(headingRadians);
    const float c = std::cos(headingRadians);
    return {
        (v.x * c) + (v.z * s),
        v.y,
        (-v.x * s) + (v.z * c),
    };
}

} // namespace

VehiclePhysicsConfig makeDefaultBusPhysicsConfig() {
    VehiclePhysicsConfig config{};
    config.massKg = 12500.0F;
    config.centerOfMassOffset = { 0.0F, -0.45F, 0.15F };
    config.wheelBaseMeters = 5.8F;
    config.trackWidthMeters = 2.35F;
    config.wheelCount = 4;

    const float halfTrack = config.trackWidthMeters * 0.5F;
    const float frontZ = -1.95F;
    const float rearZ = 1.95F;
    const float wheelY = 0.72F;

    config.wheels[0] = WheelSpec{ { -halfTrack, wheelY, frontZ }, 0.52F, 0.38F, 65000.0F, 4500.0F, false, true };
    config.wheels[1] = WheelSpec{ { halfTrack, wheelY, frontZ }, 0.52F, 0.38F, 65000.0F, 4500.0F, false, true };
    config.wheels[2] = WheelSpec{ { -halfTrack, wheelY, rearZ }, 0.52F, 0.38F, 72000.0F, 5200.0F, true, false };
    config.wheels[3] = WheelSpec{ { halfTrack, wheelY, rearZ }, 0.52F, 0.38F, 72000.0F, 5200.0F, true, false };
    return config;
}

RaycastVehicleBackend::RaycastVehicleBackend(IPhysicsWorld& physicsWorld)
    : physicsWorld_(physicsWorld) {}

void RaycastVehicleBackend::reset(const VehiclePhysicsConfig& config, const math::Transform& initialTransform) {
    config_ = config;
    if (body_.valid() && physicsWorld_.isAlive(body_)) {
        physicsWorld_.destroyRigidBody(body_);
    }

    RigidBodyDesc desc{};
    desc.transform = initialTransform;
    desc.halfExtents = { 0.95F, 0.55F, 1.25F };
    desc.massKg = config_.massKg;
    desc.dynamic = true;
    body_ = physicsWorld_.createRigidBody(desc);
}

void RaycastVehicleBackend::step(const vehicle::VehicleCommand& command,
                                 double fixedDeltaSeconds,
                                 math::Transform& inOutTransform,
                                 vehicle::VehicleState& inOutState,
                                 VehiclePhysicsTelemetry& outTelemetry) {
    (void)fixedDeltaSeconds;
    outTelemetry = {};
    outTelemetry.speedMetersPerSecond = inOutState.speedMetersPerSecond;

    if (!body_.valid() || !physicsWorld_.isAlive(body_)) {
        reset(config_, inOutTransform);
    }

    physicsWorld_.setTransform(body_, inOutTransform);

    float compressionSum = 0.0F;
    uint32_t grounded = 0;
    const uint32_t wheelCount = std::min(config_.wheelCount, static_cast<uint32_t>(config_.wheels.size()));
    for (uint32_t i = 0; i < wheelCount; ++i) {
        const WheelSpec& wheel = config_.wheels[i];
        const math::Vec3 local = rotateYaw(wheel.localPosition, inOutState.headingRadians);
        const math::Vec3 origin{
            inOutTransform.position.x + local.x,
            inOutTransform.position.y + local.y,
            inOutTransform.position.z + local.z,
        };

        const float maxDistance = wheel.suspensionRestLengthMeters + wheel.radiusMeters;
        const RaycastHit hit = physicsWorld_.raycast(RaycastRequest{ origin, { 0.0F, -1.0F, 0.0F }, maxDistance });
        if (!hit.hit) {
            continue;
        }

        ++grounded;
        const float compression = std::clamp((maxDistance - hit.distance) / std::max(0.001F, wheel.suspensionRestLengthMeters), 0.0F, 1.0F);
        compressionSum += compression;
    }

    outTelemetry.groundedWheelCount = grounded;
    outTelemetry.averageSuspensionCompression = grounded > 0 ? compressionSum / static_cast<float>(grounded) : 0.0F;
    outTelemetry.longitudinalSlip = std::clamp(command.throttle - command.brake, -1.0F, 1.0F) * (1.0F - outTelemetry.averageSuspensionCompression * 0.25F);
    outTelemetry.lateralSlip = std::fabs(command.steering) * std::clamp(inOutState.speedMetersPerSecond / 12.0F, 0.0F, 1.0F);

    inOutState.groundedWheelCount = outTelemetry.groundedWheelCount;
    inOutState.averageSuspensionCompression = outTelemetry.averageSuspensionCompression;
    inOutState.longitudinalSlip = outTelemetry.longitudinalSlip;
    inOutState.lateralSlip = outTelemetry.lateralSlip;
}

} // namespace roadforge::physics
