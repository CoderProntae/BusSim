#pragma once

#include <cstdint>

namespace roadforge::input {

/**
 * Engine-facing input snapshot.
 *
 * UI widgets will later emit commands/actions, but the native engine already
 * needs a stable action/axis boundary so vehicle, camera and replay systems do
 * not depend on raw Android MotionEvent constants.
 */
struct InputSnapshot final {
    bool primaryTouchDown = false;
    float normalizedX = 0.0F;
    float normalizedY = 0.0F;
    float steering = 0.0F; // -1 left, +1 right
    float throttle = 0.0F; // 0..1
    float brake = 0.0F;    // 0..1
    int32_t pointerCount = 0;
    uint64_t sequence = 0;
};

class InputSystem final {
public:
    void reset();
    void setSurfaceSize(int32_t width, int32_t height);
    void handleTouch(int32_t androidAction, float x, float y, int32_t pointerCount);
    void handleTouchState(int32_t androidAction, int32_t pointerCount, const float* xs, const float* ys);

    [[nodiscard]] const InputSnapshot& snapshot() const { return snapshot_; }

private:
    [[nodiscard]] float normalizeX(float x) const;
    [[nodiscard]] float normalizeY(float y) const;
    void rebuildAxesFromTouch();
    void rebuildAxesFromTouchState(int32_t pointerCount, const float* xs, const float* ys);

    int32_t surfaceWidth_ = 1;
    int32_t surfaceHeight_ = 1;
    InputSnapshot snapshot_{};
};

} // namespace roadforge::input
