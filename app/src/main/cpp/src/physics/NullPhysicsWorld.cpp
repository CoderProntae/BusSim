#include "roadforge/physics/NullPhysicsWorld.hpp"

#include <algorithm>
#include <cmath>

namespace roadforge::physics {

void NullPhysicsWorld::reset() {
    slots_.clear();
    freeList_.clear();
}

void NullPhysicsWorld::step(double fixedDeltaSeconds) {
    (void)fixedDeltaSeconds;
}

BodyHandle NullPhysicsWorld::createRigidBody(const RigidBodyDesc& desc) {
    if (!freeList_.empty()) {
        const uint32_t index = freeList_.back();
        freeList_.pop_back();
        slots_[index].alive = true;
        slots_[index].desc = desc;
        return { index, slots_[index].generation };
    }

    const uint32_t index = static_cast<uint32_t>(slots_.size());
    Slot slot{};
    slot.alive = true;
    slot.desc = desc;
    slots_.push_back(slot);
    return { index, slot.generation };
}

void NullPhysicsWorld::destroyRigidBody(BodyHandle body) {
    if (!isAlive(body)) {
        return;
    }

    Slot& slot = slots_[body.index];
    slot.alive = false;
    ++slot.generation;
    slot.desc = {};
    freeList_.push_back(body.index);
}

bool NullPhysicsWorld::isAlive(BodyHandle body) const {
    return indexInRange(body) && slots_[body.index].alive && slots_[body.index].generation == body.generation;
}

void NullPhysicsWorld::setTransform(BodyHandle body, const math::Transform& transform) {
    if (!isAlive(body)) {
        return;
    }
    slots_[body.index].desc.transform = transform;
}

math::Transform NullPhysicsWorld::getTransform(BodyHandle body) const {
    if (!isAlive(body)) {
        return {};
    }
    return slots_[body.index].desc.transform;
}

RaycastHit NullPhysicsWorld::raycast(const RaycastRequest& request) const {
    RaycastHit hit{};
    if (request.maxDistance <= 0.0F) {
        return hit;
    }

    const math::Vec3 direction = math::normalize(request.direction);
    if (std::fabs(direction.y) <= 0.00001F) {
        return hit;
    }

    // Infinite ground plane at y=0. Only report hits in front of the ray.
    const float t = -request.origin.y / direction.y;
    if (t < 0.0F || t > request.maxDistance) {
        return hit;
    }

    hit.hit = true;
    hit.position = {
        request.origin.x + (direction.x * t),
        0.0F,
        request.origin.z + (direction.z * t),
    };
    hit.normal = { 0.0F, 1.0F, 0.0F };
    hit.distance = t;
    hit.fraction = std::clamp(t / request.maxDistance, 0.0F, 1.0F);
    return hit;
}

bool NullPhysicsWorld::indexInRange(BodyHandle body) const {
    return body.index < slots_.size();
}

} // namespace roadforge::physics
