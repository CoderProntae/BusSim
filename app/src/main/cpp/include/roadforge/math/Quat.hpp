#pragma once

#include "roadforge/math/Mat4.hpp"
#include "roadforge/math/Vec.hpp"

#include <cmath>

namespace roadforge::math {

struct Quat final {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
    float w = 1.0F;
};

inline Quat quatIdentity() {
    return {};
}

inline Quat quatNormalize(const Quat& q) {
    const float lengthSquared = (q.x * q.x) + (q.y * q.y) + (q.z * q.z) + (q.w * q.w);
    if (lengthSquared <= 0.000001F) {
        return quatIdentity();
    }

    const float invLength = 1.0F / std::sqrt(lengthSquared);
    return { q.x * invLength, q.y * invLength, q.z * invLength, q.w * invLength };
}

inline Quat quatFromAxisAngle(const Vec3& axis, float radians) {
    const Vec3 normalizedAxis = normalize(axis);
    const float halfAngle = radians * 0.5F;
    const float s = std::sin(halfAngle);
    return quatNormalize({ normalizedAxis.x * s, normalizedAxis.y * s, normalizedAxis.z * s, std::cos(halfAngle) });
}

inline Quat quatMultiply(const Quat& a, const Quat& b) {
    return quatNormalize({
        (a.w * b.x) + (a.x * b.w) + (a.y * b.z) - (a.z * b.y),
        (a.w * b.y) - (a.x * b.z) + (a.y * b.w) + (a.z * b.x),
        (a.w * b.z) + (a.x * b.y) - (a.y * b.x) + (a.z * b.w),
        (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z),
    });
}

inline Mat4 quatToMat4(const Quat& input) {
    const Quat q = quatNormalize(input);
    const float xx = q.x * q.x;
    const float yy = q.y * q.y;
    const float zz = q.z * q.z;
    const float xy = q.x * q.y;
    const float xz = q.x * q.z;
    const float yz = q.y * q.z;
    const float wx = q.w * q.x;
    const float wy = q.w * q.y;
    const float wz = q.w * q.z;

    Mat4 result = identity();
    result.m[0] = 1.0F - (2.0F * (yy + zz));
    result.m[1] = 2.0F * (xy + wz);
    result.m[2] = 2.0F * (xz - wy);

    result.m[4] = 2.0F * (xy - wz);
    result.m[5] = 1.0F - (2.0F * (xx + zz));
    result.m[6] = 2.0F * (yz + wx);

    result.m[8] = 2.0F * (xz + wy);
    result.m[9] = 2.0F * (yz - wx);
    result.m[10] = 1.0F - (2.0F * (xx + yy));
    return result;
}

} // namespace roadforge::math
