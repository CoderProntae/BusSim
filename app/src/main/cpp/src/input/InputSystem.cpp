#include "roadforge/input/InputSystem.hpp"

#include <algorithm>

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
    ++snapshot_.sequence;
    snapshot_.pointerCount = std::max(0, pointerCount);

    const bool touchBegins = androidAction == kActionDown || androidAction == kActionPointerDown;
    const bool touchContinues = androidAction == kActionMove;
    const bool touchEnds = androidAction == kActionUp || androidAction == kActionCancel || androidAction == kActionPointerUp;

    if (touchBegins || touchContinues) {
        snapshot_.primaryTouchDown = true;
        snapshot_.normalizedX = normalizeX(x);
        snapshot_.normalizedY = normalizeY(y);
        rebuildAxesFromTouch();
        return;
    }

    if (touchEnds) {
        snapshot_.primaryTouchDown = false;
        snapshot_.normalizedX = normalizeX(x);
        snapshot_.normalizedY = normalizeY(y);
        snapshot_.steering = 0.0F;
        snapshot_.throttle = 0.0F;
        snapshot_.brake = 0.0F;
    }
}

float InputSystem::normalizeX(float x) const {
    return clampUnit(x / static_cast<float>(surfaceWidth_));
}

float InputSystem::normalizeY(float y) const {
    return clampUnit(y / static_cast<float>(surfaceHeight_));
}

void InputSystem::rebuildAxesFromTouch() {
    snapshot_.steering = std::clamp((snapshot_.normalizedX - 0.5F) * 2.0F, -1.0F, 1.0F);

    const bool lowerHalf = snapshot_.normalizedY >= 0.50F;
    const bool rightHalf = snapshot_.normalizedX >= 0.50F;
    snapshot_.throttle = lowerHalf && rightHalf ? 1.0F : 0.0F;
    snapshot_.brake = lowerHalf && !rightHalf ? 1.0F : 0.0F;
}

} // namespace roadforge::input
