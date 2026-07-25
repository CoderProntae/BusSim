#pragma once

#include "roadforge/math/Transform.hpp"
#include "roadforge/math/Vec.hpp"

#include <cstdint>

namespace roadforge::physics {

struct BodyHandle final {
    uint32_t index = UINT32_MAX;
    uint32_t generation = 0;

    [[nodiscard]] bool valid() const { return index != UINT32_MAX; }
};

struct RigidBodyDesc final {
    math::Transform transform{};
    math::Vec3 halfExtents{ 0.5F, 0.5F, 0.5F };
    float massKg = 1.0F;
    bool dynamic = true;
};

struct RaycastRequest final {
    math::Vec3 origin{};
    math::Vec3 direction{ 0.0F, -1.0F, 0.0F };
    float maxDistance = 1.0F;
};

struct RaycastHit final {
    bool hit = false;
    BodyHandle body{};
    math::Vec3 position{};
    math::Vec3 normal{ 0.0F, 1.0F, 0.0F };
    float distance = 0.0F;
    float fraction = 0.0F;
};

} // namespace roadforge::physics
