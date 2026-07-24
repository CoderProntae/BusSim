#pragma once

#include <algorithm>
#include <cmath>

namespace roadforge::math {

struct Vec2 final {
    float x = 0.0F;
    float y = 0.0F;
};

struct Vec3 final {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
};

struct Vec4 final {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
    float w = 0.0F;
};

inline Vec4 lerp(const Vec4& a, const Vec4& b, float t) {
    const float u = std::clamp(t, 0.0F, 1.0F);
    return {
        a.x * (1.0F - u) + b.x * u,
        a.y * (1.0F - u) + b.y * u,
        a.z * (1.0F - u) + b.z * u,
        a.w * (1.0F - u) + b.w * u,
    };
}

inline float length(const Vec3& v) {
    return std::sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

inline Vec3 normalize(const Vec3& v) {
    const float len = length(v);
    if (len <= 0.00001F) {
        return {};
    }
    return { v.x / len, v.y / len, v.z / len };
}

} // namespace roadforge::math
