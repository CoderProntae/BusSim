#pragma once

#include "roadforge/math/Mat4.hpp"
#include "roadforge/math/Quat.hpp"
#include "roadforge/math/Vec.hpp"

namespace roadforge::math {

struct Transform final {
    Vec3 position{ 0.0F, 0.0F, 0.0F };
    Quat rotation{};
    Vec3 scale{ 1.0F, 1.0F, 1.0F };
};

inline Mat4 translationMatrix(const Vec3& translation) {
    Mat4 result = identity();
    result.m[12] = translation.x;
    result.m[13] = translation.y;
    result.m[14] = translation.z;
    return result;
}

inline Mat4 scaleMatrix(const Vec3& scale) {
    Mat4 result = identity();
    result.m[0] = scale.x;
    result.m[5] = scale.y;
    result.m[10] = scale.z;
    return result;
}

inline Mat4 transformToMat4(const Transform& transform) {
    const Mat4 translation = translationMatrix(transform.position);
    const Mat4 rotation = quatToMat4(transform.rotation);
    const Mat4 scale = scaleMatrix(transform.scale);
    return multiply(translation, multiply(rotation, scale));
}

} // namespace roadforge::math
