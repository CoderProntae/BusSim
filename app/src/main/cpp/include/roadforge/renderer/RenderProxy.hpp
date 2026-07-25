#pragma once

#include "roadforge/math/Transform.hpp"

#include <cstdint>

namespace roadforge::renderer {

enum class DebugMeshKind : uint8_t {
    RoadSurface = 0,
    BusPlaceholder,
};

struct DebugRenderProxy final {
    math::Transform transform{};
    DebugMeshKind meshKind = DebugMeshKind::RoadSurface;
    float boundingRadius = 1.0F;
    bool visible = true;
};

} // namespace roadforge::renderer
