#pragma once

#include "roadforge/math/Transform.hpp"
#include "roadforge/vehicle/VehicleTypes.hpp"

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
    DebugRoadSurface = 0,
    BusPlaceholder,
};

struct TransformComponent final {
    math::Transform transform{};
};

struct MeshComponent final {
    MeshKind meshKind = MeshKind::DebugRoadSurface;
    float boundingRadius = 1.0F;
    bool visible = true;
};

struct CameraControlInput final {
    float steering = 0.0F;
    float throttle = 0.0F;
    float brake = 0.0F;
    bool active = false;
};

struct RenderProxy final {
    math::Transform transform{};
    MeshKind meshKind = MeshKind::DebugRoadSurface;
    float boundingRadius = 1.0F;
    bool visible = true;
};

struct DebugCamera final {
    math::Vec3 eye{ 0.0F, 1.65F, -4.25F };
    math::Vec3 target{ 0.0F, 0.0F, 3.0F };
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
    Entity createDebugBusEntity();
    void collectRenderProxies(std::vector<RenderProxy>& out) const;
    void fixedUpdate(double fixedDeltaSeconds, const vehicle::VehicleState& vehicleState);

    [[nodiscard]] const math::Transform& debugRoadTransform() const { return debugRoadTransformCache_; }
    [[nodiscard]] Entity debugRoadEntity() const { return debugRoadEntity_; }
    [[nodiscard]] Entity debugBusEntity() const { return debugBusEntity_; }
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
    Entity debugBusEntity_{};
    math::Transform debugRoadTransformCache_{};
    math::Transform debugBusTransformCache_{};
    DebugCamera debugCamera_{};
    float cameraLateralOffset_ = 0.0F;
    float cameraDistance_ = 7.65F;
    float cameraHeight_ = 1.65F;
    double simulationSeconds_ = 0.0;
};

} // namespace roadforge::world
