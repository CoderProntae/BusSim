#pragma once

#include "roadforge/physics/IPhysicsWorld.hpp"

#include <optional>
#include <vector>

namespace roadforge::physics {

/**
 * Deterministic no-middleware physics world.
 *
 * It stores transforms for handles and provides a simple y=0 ground-plane
 * raycast. This lets vehicle/raycast code be developed before integrating a
 * full rigid-body library.
 */
class NullPhysicsWorld final : public IPhysicsWorld {
public:
    void reset() override;
    void step(double fixedDeltaSeconds) override;

    [[nodiscard]] BodyHandle createRigidBody(const RigidBodyDesc& desc) override;
    void destroyRigidBody(BodyHandle body) override;
    [[nodiscard]] bool isAlive(BodyHandle body) const override;

    void setTransform(BodyHandle body, const math::Transform& transform) override;
    [[nodiscard]] math::Transform getTransform(BodyHandle body) const override;

    [[nodiscard]] RaycastHit raycast(const RaycastRequest& request) const override;

private:
    struct Slot final {
        uint32_t generation = 1;
        bool alive = false;
        RigidBodyDesc desc{};
    };

    [[nodiscard]] bool indexInRange(BodyHandle body) const;

    std::vector<Slot> slots_;
    std::vector<uint32_t> freeList_;
};

} // namespace roadforge::physics
