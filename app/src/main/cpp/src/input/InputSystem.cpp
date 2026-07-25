#include "roadforge/input/InputSystem.hpp"

#include <algorithm>
#include <cmath>

namespace roadforge::input {

namespace {
constexpr int32_t kActionDown = 0;
constexpr int32_t kActionUp = 1;
constexpr int32_t kActionMove = 2;
constexpr int32_t kActionCancel = 3;
constexpr int32_t kActionPointerDown = 5;
constexpr int32_t kActionPointerUp = 6;

float clampUnit(float value) {
    return std::clamp(value, 0.0F, 1.0F);
}
} // namespace

void InputSystem::reset() {
    snapshot_ = {};
}

void InputSystem::setSurfaceSize(int32_t width, int32_t height) {
    surfaceWidth_ = std::max(1, width);
    surfaceHeight_ = std::max(1, height);
}

void InputSystem::handleTouch(int32_t androidAction, float x, float y, int32_t pointerCount) {
    if (pointerCount <= 0) {
        handleTouchState(androidAction, 0, nullptr, nullptr);
        return;
    }

    const float xs[] = { x };
    const float ys[] = { y };
    handleTouchState(androidAction, 1, xs, ys);
    snapshot_.pointerCount = pointerCount;
}

void InputSystem::handleTouchState(int32_t androidAction, int32_t pointerCount, const float* xs, const float* ys) {
    ++snapshot_.sequence;

    const bool touchEnds = androidAction == kActionUp || androidAction == kActionCancel;
    if (touchEnds || pointerCount <= 0 || xs == nullptr || ys == nullptr) {
        snapshot_.primaryTouchDown = false;
        snapshot_.pointerCount = 0;
        snapshot_.steering = 0.0F;
        snapshot_.throttle = 0.0F;
        snapshot_.brake = 0.0F;
        return;
    }

    // ACTION_POINTER_UP still contains the pointer that is being lifted in the
    // current MotionEvent. The next event will remove it; for this bootstrap
    // controller accepting the transient extra pointer is harmless.
    const bool touchBeginsOrMoves = androidAction == kActionDown
        || androidAction == kActionPointerDown
        || androidAction == kActionPointerUp
        || androidAction == kActionMove;
    if (!touchBeginsOrMoves) {
        return;
    }

    snapshot_.primaryTouchDown = true;
    snapshot_.pointerCount = pointerCount;
    snapshot_.normalizedX = normalizeX(xs[0]);
    snapshot_.normalizedY = normalizeY(ys[0]);
    rebuildAxesFromTouchState(pointerCount, xs, ys);
}

float InputSystem::normalizeX(float x) const {
    return clampUnit(x / static_cast<float>(surfaceWidth_));
}

float InputSystem::normalizeY(float y) const {
    return clampUnit(y / static_cast<float>(surfaceHeight_));
}

void InputSystem::rebuildAxesFromTouch() {
    const float x = snapshot_.normalizedX * static_cast<float>(surfaceWidth_);
    const float y = snapshot_.normalizedY * static_cast<float>(surfaceHeight_);
    const float xs[] = { x };
    const float ys[] = { y };
    rebuildAxesFromTouchState(1, xs, ys);
}

void InputSystem::rebuildAxesFromTouchState(int32_t pointerCount, const float* xs, const float* ys) {
    float steering = 0.0F;
    bool steeringFound = false;
    float throttle = 0.0F;
    float brake = 0.0F;

    for (int32_t i = 0; i < pointerCount; ++i) {
        const float nx = normalizeX(xs[i]);
        const float ny = normalizeY(ys[i]);

        // Phase 2.3 temporary mobile driving zones:
        // left 50% = steering strip, bottom-right split = brake/throttle.
        // This allows two fingers: left thumb steers while right thumb holds gas.
        if (nx < 0.50F) {
            const float candidateSteering = std::clamp((nx / 0.50F) * 2.0F - 1.0F, -1.0F, 1.0F);
            if (!steeringFound || std::fabs(candidateSteering) > std::fabs(steering)) {
                steering = candidateSteering;
                steeringFound = true;
            }
            continue;
        }

        const bool lowerHalf = ny >= 0.50F;
        if (lowerHalf) {
            if (nx >= 0.75F) {
                throttle = 1.0F;
            } else {
                brake = 1.0F;
            }
        }
    }

    snapshot_.steering = steeringFound ? steering : 0.0F;
    snapshot_.throttle = throttle;
    snapshot_.brake = brake;
}

} // namespace roadforge::input
