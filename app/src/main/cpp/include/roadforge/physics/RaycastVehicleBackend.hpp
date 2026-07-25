#pragma once

#include "roadforge/physics/IPhysicsWorld.hpp"
#include "roadforge/physics/IVehiclePhysicsBackend.hpp"

namespace roadforge::physics {

[[nodiscard]] VehiclePhysicsConfig makeDefaultBusPhysicsConfig();

/**
 * First vehicle physics backend prototype.
 *
 * It does not replace the current kinematic movement yet. It uses the abstract
 * IPhysicsWorld raycast API to sample wheel-ground contacts and emits telemetry
 * that later suspension/force integration will consume.
 */
class RaycastVehicleBackend final : public IVehiclePhysicsBackend {
public:
    explicit RaycastVehicleBackend(IPhysicsWorld& physicsWorld);

    void reset(const VehiclePhysicsConfig& config, const math::Transform& initialTransform) override;
    void step(const vehicle::VehicleCommand& command,
              double fixedDeltaSeconds,
              math::Transform& inOutTransform,
              vehicle::VehicleState& inOutState,
              VehiclePhysicsTelemetry& outTelemetry) override;

private:
    IPhysicsWorld& physicsWorld_;
    VehiclePhysicsConfig config_{};
    BodyHandle body_{};
};

} // namespace roadforge::physics
