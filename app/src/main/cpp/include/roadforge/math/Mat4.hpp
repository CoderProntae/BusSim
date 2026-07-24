#pragma once

#include "roadforge/math/Vec.hpp"

#include <array>
#include <cmath>

namespace roadforge::math {

struct Mat4 final {
    std::array<float, 16> m{}; // Column-major, GLSL-compatible.

    [[nodiscard]] const float* data() const { return m.data(); }
    [[nodiscard]] float* data() { return m.data(); }
};

inline float dot(const Vec3& a, const Vec3& b) {
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

inline Vec3 subtract(const Vec3& a, const Vec3& b) {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        (a.y * b.z) - (a.z * b.y),
        (a.z * b.x) - (a.x * b.z),
        (a.x * b.y) - (a.y * b.x),
    };
}

inline Mat4 identity() {
    Mat4 result{};
    result.m[0] = 1.0F;
    result.m[5] = 1.0F;
    result.m[10] = 1.0F;
    result.m[15] = 1.0F;
    return result;
}

inline Mat4 multiply(const Mat4& a, const Mat4& b) {
    Mat4 result{};
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            float value = 0.0F;
            for (int k = 0; k < 4; ++k) {
                value += a.m[(k * 4) + row] * b.m[(col * 4) + k];
            }
            result.m[(col * 4) + row] = value;
        }
    }
    return result;
}

inline Mat4 perspectiveVulkanLH(float fovYRadians, float aspect, float nearPlane, float farPlane) {
    const float safeAspect = aspect > 0.0001F ? aspect : 1.0F;
    const float f = 1.0F / std::tan(fovYRadians * 0.5F);

    Mat4 result{};
    result.m[0] = f / safeAspect;
    result.m[5] = -f; // Vulkan clip-space Y is inverted compared to OpenGL-style math.
    result.m[10] = farPlane / (farPlane - nearPlane);
    result.m[11] = 1.0F;
    result.m[14] = -(nearPlane * farPlane) / (farPlane - nearPlane);
    return result;
}

inline Mat4 lookAtLH(const Vec3& eye, const Vec3& target, const Vec3& up) {
    const Vec3 zAxis = normalize(subtract(target, eye));
    const Vec3 xAxis = normalize(cross(up, zAxis));
    const Vec3 yAxis = cross(zAxis, xAxis);

    Mat4 result = identity();
    result.m[0] = xAxis.x;
    result.m[1] = yAxis.x;
    result.m[2] = zAxis.x;

    result.m[4] = xAxis.y;
    result.m[5] = yAxis.y;
    result.m[6] = zAxis.y;

    result.m[8] = xAxis.z;
    result.m[9] = yAxis.z;
    result.m[10] = zAxis.z;

    result.m[12] = -dot(xAxis, eye);
    result.m[13] = -dot(yAxis, eye);
    result.m[14] = -dot(zAxis, eye);
    return result;
}

} // namespace roadforge::math
