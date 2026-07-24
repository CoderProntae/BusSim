#pragma once

#include "roadforge/math/Transform.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace roadforge::world {

struct Entity final {
    uint32_t index = UINT32_MAX;
    uint32_t generation = 0;

    [[nodiscard]] bool valid() const { return index != UINT32_MAX; }
};

inline bool operator==(const Entity& a, const Entity& b) {
    return a.index == b.index && a.generation == b.generation;
}

enum class MeshKind : uint8_t {
    DebugRoadPlate = 0,
};

struct TransformComponent final {
    math::Transform transform{};
};

struct MeshComponent final {
    MeshKind meshKind = MeshKind::DebugRoadPlate;
    float boundingRadius = 1.0F;
    bool visible = true;
};

struct CameraControlInput final {
    float steering = 0.0F;
    float throttle = 0.0F;
    float brake = 0.0F;
    bool active = false;
};

struct DebugCamera final {
    math::Vec3 eye{ 0.0F, 1.65F, -4.25F };
    math::Vec3 target{ 0.0F, 0.0F, 3.4F };
    math::Vec3 up{ 0.0F, 1.0F, 0.0F };
    float fovYRadians = 60.0F * 0.01745329252F;
};

/**
 * Minimal sparse-slot world used as the bridge toward the future ECS.
 *
 * This is intentionally small: entity lifetime + transform/mesh components are
 * enough to move the debug render object out of the renderer and into a world
 * representation. Later phases can replace the storage with a full sparse-set
 * ECS without changing high-level system ownership rules.
 */
class World final {
public:
    void reset();

    [[nodiscard]] Entity createEntity();
    void destroyEntity(Entity entity);
    [[nodiscard]] bool alive(Entity entity) const;

    TransformComponent& addTransform(Entity entity, const math::Transform& transform = {});
    MeshComponent& addMesh(Entity entity, const MeshComponent& mesh = {});

    [[nodiscard]] TransformComponent* transform(Entity entity);
    [[nodiscard]] const TransformComponent* transform(Entity entity) const;
    [[nodiscard]] MeshComponent* mesh(Entity entity);
    [[nodiscard]] const MeshComponent* mesh(Entity entity) const;

    Entity createDebugRoadEntity();
    void fixedUpdate(double fixedDeltaSeconds, const CameraControlInput& cameraInput = {});

    [[nodiscard]] const math::Transform& debugRoadTransform() const { return debugRoadTransformCache_; }
    [[nodiscard]] Entity debugRoadEntity() const { return debugRoadEntity_; }
    [[nodiscard]] const DebugCamera& debugCamera() const { return debugCamera_; }

private:
    struct Slot final {
        uint32_t generation = 1;
        bool alive = false;
    };

    [[nodiscard]] bool indexInRange(Entity entity) const;

    std::vector<Slot> slots_;
    std::vector<uint32_t> freeList_;
    std::vector<std::optional<TransformComponent>> transforms_;
    std::vector<std::optional<MeshComponent>> meshes_;

    Entity debugRoadEntity_{};
    math::Transform debugRoadTransformCache_{};
    DebugCamera debugCamera_{};
    float cameraOrbitYawRadians_ = 0.0F;
    float cameraDistance_ = 7.65F;
    float cameraHeight_ = 1.65F;
    double simulationSeconds_ = 0.0;
};

} // namespace roadforge::world
