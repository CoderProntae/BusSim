#pragma once

#include "roadforge/input/InputSystem.hpp"
#include "roadforge/vehicle/VehicleTypes.hpp"

namespace roadforge::vehicle {

/**
 * First vehicle command/state boundary for Phase 2.
 *
 * This is intentionally not a full physics vehicle yet. It gives the engine a
 * deterministic, testable place to translate input into vehicle commands and to
 * expose a stable vehicle state snapshot before the kinematic/physics layers are
 * added in the next Phase 2 steps.
 */
class VehicleController final {
public:
    void reset();

    [[nodiscard]] static VehicleCommand commandFromInput(const input::InputSnapshot& input);

    void setCommand(const VehicleCommand& command);
    void fixedUpdate(double fixedDeltaSeconds);

    [[nodiscard]] const VehicleCommand& command() const { return command_; }
    [[nodiscard]] const VehicleState& state() const { return state_; }

private:
    static float approach(float current, float target, float maxDelta);

    VehicleCommand command_{};
    VehicleState state_{};
};

} // namespace roadforge::vehicle
