#pragma once

#include "roadforge/math/Mat4.hpp"
#include "roadforge/math/Vec.hpp"

#include <array>
#include <cmath>

namespace roadforge::math {

struct Plane final {
    Vec3 normal{};
    float distance = 0.0F;
};

struct Frustum final {
    enum PlaneIndex : int {
        Left = 0,
        Right,
        Bottom,
        Top,
        Near,
        Far,
        Count,
    };

    std::array<Plane, Count> planes{};
};

inline Plane normalizePlane(const Plane& plane) {
    const float len = length(plane.normal);
    if (len <= 0.000001F) {
        return plane;
    }
    return {
        { plane.normal.x / len, plane.normal.y / len, plane.normal.z / len },
        plane.distance / len,
    };
}

inline Frustum extractFrustum(const Mat4& viewProjection) {
    const auto& m = viewProjection.m;
    Frustum frustum{};

    frustum.planes[Frustum::Left] = normalizePlane({ { m[3] + m[0], m[7] + m[4], m[11] + m[8] }, m[15] + m[12] });
    frustum.planes[Frustum::Right] = normalizePlane({ { m[3] - m[0], m[7] - m[4], m[11] - m[8] }, m[15] - m[12] });
    frustum.planes[Frustum::Bottom] = normalizePlane({ { m[3] + m[1], m[7] + m[5], m[11] + m[9] }, m[15] + m[13] });
    frustum.planes[Frustum::Top] = normalizePlane({ { m[3] - m[1], m[7] - m[5], m[11] - m[9] }, m[15] - m[13] });
    frustum.planes[Frustum::Near] = normalizePlane({ { m[2], m[6], m[10] }, m[14] });
    frustum.planes[Frustum::Far] = normalizePlane({ { m[3] - m[2], m[7] - m[6], m[11] - m[10] }, m[15] - m[14] });

    return frustum;
}

inline bool sphereInsideFrustum(const Frustum& frustum, const Vec3& center, float radius) {
    for (const Plane& plane : frustum.planes) {
        if (dot(plane.normal, center) + plane.distance < -radius) {
            return false;
        }
    }
    return true;
}

} // namespace roadforge::math
