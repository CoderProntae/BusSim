#pragma once

#include "roadforge/math/Transform.hpp"
#include "roadforge/physics/PhysicsTypes.hpp"

namespace roadforge::physics {

/**
 * Physics world boundary used by gameplay and vehicle systems.
 *
 * The engine must not depend directly on a concrete physics middleware. A Jolt
 * backend, a custom raycast backend, or a no-op test backend must be swappable
 * behind this interface.
 */
class IPhysicsWorld {
public:
    virtual ~IPhysicsWorld() = default;

    virtual void reset() = 0;
    virtual void step(double fixedDeltaSeconds) = 0;

    [[nodiscard]] virtual BodyHandle createRigidBody(const RigidBodyDesc& desc) = 0;
    virtual void destroyRigidBody(BodyHandle body) = 0;
    [[nodiscard]] virtual bool isAlive(BodyHandle body) const = 0;

    virtual void setTransform(BodyHandle body, const math::Transform& transform) = 0;
    [[nodiscard]] virtual math::Transform getTransform(BodyHandle body) const = 0;

    [[nodiscard]] virtual RaycastHit raycast(const RaycastRequest& request) const = 0;
};

} // namespace roadforge::physics
