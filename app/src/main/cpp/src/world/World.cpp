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
    debugBusTransformCache_ = {};
    debugCamera_ = {};
    cameraMode_ = CameraMode::Follow;
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
        debugBusTransformCache_ = {};
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
    transformValue.position = { 0.0F, 0.0F, 3.7F };
    transformValue.rotation = math::quatIdentity();
    transformValue.scale = { 1.0F, 1.0F, 1.0F };
    addTransform(debugBusEntity_, transformValue);
    addMesh(debugBusEntity_, MeshComponent{ MeshKind::BusPlaceholder, 3.0F, true });
    debugBusTransformCache_ = transformValue;
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

void World::fixedUpdate(double fixedDeltaSeconds, const vehicle::VehicleState& vehicleState) {
    const double safeDelta = std::clamp(fixedDeltaSeconds, 0.0, 0.1);
    simulationSeconds_ += safeDelta;

    TransformComponent* roadTransform = transform(debugRoadEntity_);
    if (roadTransform != nullptr) {
        roadTransform->transform.rotation = math::quatIdentity();
        debugRoadTransformCache_ = roadTransform->transform;
    }

    TransformComponent* busTransform = transform(debugBusEntity_);
    if (busTransform != nullptr) {
        busTransform->transform.position = { vehicleState.positionX, 0.0F, vehicleState.positionZ };
        busTransform->transform.rotation = math::quatFromAxisAngle({ 0.0F, 1.0F, 0.0F }, vehicleState.headingRadians);
        busTransform->transform.scale = { 1.0F, 1.0F, 1.0F };
        debugBusTransformCache_ = busTransform->transform;
    }

    const float heading = vehicleState.headingRadians;
    const math::Vec3 forward{ std::sin(heading), 0.0F, std::cos(heading) };
    const math::Vec3 right{ std::cos(heading), 0.0F, -std::sin(heading) };
    const math::Vec3 busPosition{ vehicleState.positionX, 0.0F, vehicleState.positionZ };
    debugCamera_.up = { 0.0F, 1.0F, 0.0F };

    if (cameraMode_ == CameraMode::Cabin) {
        debugCamera_.fovYRadians = 72.0F * 0.01745329252F;
        debugCamera_.eye = {
            busPosition.x - (right.x * 0.36F) + (forward.x * 0.62F),
            0.98F,
            busPosition.z - (right.z * 0.36F) + (forward.z * 0.62F),
        };
        debugCamera_.target = {
            debugCamera_.eye.x + (forward.x * 7.0F),
            0.82F,
            debugCamera_.eye.z + (forward.z * 7.0F),
        };
    } else {
        debugCamera_.fovYRadians = 60.0F * 0.01745329252F;
        debugCamera_.target = {
            busPosition.x + (forward.x * 1.8F),
            0.42F,
            busPosition.z + (forward.z * 1.8F),
        };
        debugCamera_.eye = {
            busPosition.x - (forward.x * cameraDistance_),
            cameraHeight_ + 1.05F,
            busPosition.z - (forward.z * cameraDistance_),
        };
    }
}

void World::toggleCameraMode() {
    cameraMode_ = cameraMode_ == CameraMode::Follow ? CameraMode::Cabin : CameraMode::Follow;
}

bool World::indexInRange(Entity entity) const {
    return entity.index < slots_.size();
}

} // namespace roadforge::world
