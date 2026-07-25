#include "roadforge/world/World.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace roadforge::world {

void World::reset() {
    slots_.clear();
    freeList_.clear();
    transforms_.clear();
    meshes_.clear();
    debugRoadEntity_ = {};
    debugBusEntity_ = {};
    debugRoadTransformCache_ = {};
    debugCamera_ = {};
    cameraLateralOffset_ = 0.0F;
    cameraDistance_ = 7.65F;
    cameraHeight_ = 1.65F;
    simulationSeconds_ = 0.0;
}

Entity World::createEntity() {
    if (!freeList_.empty()) {
        const uint32_t index = freeList_.back();
        freeList_.pop_back();
        slots_[index].alive = true;
        return { index, slots_[index].generation };
    }

    const uint32_t index = static_cast<uint32_t>(slots_.size());
    slots_.push_back({ 1, true });
    transforms_.emplace_back(std::nullopt);
    meshes_.emplace_back(std::nullopt);
    return { index, 1 };
}

void World::destroyEntity(Entity entity) {
    if (!alive(entity)) {
        return;
    }

    Slot& slot = slots_[entity.index];
    slot.alive = false;
    ++slot.generation;
    transforms_[entity.index].reset();
    meshes_[entity.index].reset();
    freeList_.push_back(entity.index);

    if (debugRoadEntity_ == entity) {
        debugRoadEntity_ = {};
        debugRoadTransformCache_ = {};
    }
    if (debugBusEntity_ == entity) {
        debugBusEntity_ = {};
    }
}

bool World::alive(Entity entity) const {
    return indexInRange(entity) && slots_[entity.index].alive && slots_[entity.index].generation == entity.generation;
}

TransformComponent& World::addTransform(Entity entity, const math::Transform& transformValue) {
    if (!alive(entity)) {
        entity = createEntity();
    }
    transforms_[entity.index] = TransformComponent{ transformValue };
    return transforms_[entity.index].value();
}

MeshComponent& World::addMesh(Entity entity, const MeshComponent& meshValue) {
    if (!alive(entity)) {
        entity = createEntity();
    }
    meshes_[entity.index] = meshValue;
    return meshes_[entity.index].value();
}

TransformComponent* World::transform(Entity entity) {
    if (!alive(entity) || !transforms_[entity.index].has_value()) {
        return nullptr;
    }
    return &transforms_[entity.index].value();
}

const TransformComponent* World::transform(Entity entity) const {
    if (!alive(entity) || !transforms_[entity.index].has_value()) {
        return nullptr;
    }
    return &transforms_[entity.index].value();
}

MeshComponent* World::mesh(Entity entity) {
    if (!alive(entity) || !meshes_[entity.index].has_value()) {
        return nullptr;
    }
    return &meshes_[entity.index].value();
}

const MeshComponent* World::mesh(Entity entity) const {
    if (!alive(entity) || !meshes_[entity.index].has_value()) {
        return nullptr;
    }
    return &meshes_[entity.index].value();
}

Entity World::createDebugRoadEntity() {
    if (alive(debugRoadEntity_)) {
        return debugRoadEntity_;
    }

    debugRoadEntity_ = createEntity();
    math::Transform transformValue{};
    transformValue.position = { 0.0F, 0.0F, 0.0F };
    transformValue.rotation = math::quatIdentity();
    transformValue.scale = { 1.0F, 1.0F, 1.0F };
    addTransform(debugRoadEntity_, transformValue);
    addMesh(debugRoadEntity_, MeshComponent{ MeshKind::DebugRoadSurface, 1000.0F, true });
    debugRoadTransformCache_ = transformValue;
    createDebugBusEntity();
    return debugRoadEntity_;
}

Entity World::createDebugBusEntity() {
    if (alive(debugBusEntity_)) {
        return debugBusEntity_;
    }

    debugBusEntity_ = createEntity();
    math::Transform transformValue{};
    transformValue.position = { 0.0F, 0.0F, 0.0F };
    transformValue.rotation = math::quatIdentity();
    transformValue.scale = { 1.0F, 1.0F, 1.0F };
    addTransform(debugBusEntity_, transformValue);
    addMesh(debugBusEntity_, MeshComponent{ MeshKind::BusPlaceholder, 3.0F, true });
    return debugBusEntity_;
}

void World::collectRenderProxies(std::vector<RenderProxy>& out) const {
    out.clear();
    for (std::size_t index = 0; index < slots_.size(); ++index) {
        if (!slots_[index].alive || !transforms_[index].has_value() || !meshes_[index].has_value()) {
            continue;
        }

        const MeshComponent& meshComponent = meshes_[index].value();
        if (!meshComponent.visible) {
            continue;
        }

        out.push_back(RenderProxy{
            transforms_[index].value().transform,
            meshComponent.meshKind,
            meshComponent.boundingRadius,
            meshComponent.visible,
        });
    }
}

void World::fixedUpdate(double fixedDeltaSeconds, const CameraControlInput& cameraInput) {
    const double safeDelta = std::clamp(fixedDeltaSeconds, 0.0, 0.1);
    simulationSeconds_ += safeDelta;

    if (cameraInput.active) {
        const float dt = static_cast<float>(safeDelta);
        // Debug camera is now a simple track/dolly camera, not an orbit camera.
        // Left/right moves the camera position sideways with the target, so the
        // diagnostic square should slide on screen instead of appearing to spin.
        cameraLateralOffset_ += cameraInput.steering * dt * 3.0F;
        cameraLateralOffset_ = std::clamp(cameraLateralOffset_, -4.0F, 4.0F);
        cameraDistance_ += (cameraInput.brake - cameraInput.throttle) * dt * 3.5F;
        cameraDistance_ = std::clamp(cameraDistance_, 3.25F, 10.5F);
    }

    debugCamera_.target = { cameraLateralOffset_, 0.0F, 3.0F };
    debugCamera_.up = { 0.0F, 1.0F, 0.0F };
    debugCamera_.fovYRadians = 60.0F * 0.01745329252F;
    debugCamera_.eye = {
        cameraLateralOffset_,
        cameraHeight_,
        debugCamera_.target.z - cameraDistance_,
    };

    TransformComponent* roadTransform = transform(debugRoadEntity_);
    if (roadTransform == nullptr) {
        return;
    }

    // Keep the diagnostic square itself stable. When the perspective changes, it is the camera moving, not the road/square spinning.
    roadTransform->transform.rotation = math::quatIdentity();
    debugRoadTransformCache_ = roadTransform->transform;
}

bool World::indexInRange(Entity entity) const {
    return entity.index < slots_.size();
}

} // namespace roadforge::world
