#pragma once

#include "roadforge/vehicle/VehicleTypes.hpp"

#include <cstdint>
#include <vector>

namespace roadforge::vehicle {

enum class VehicleEventType : uint8_t {
    OffRoadEntered = 0,
    OffRoadExited,
    LowFuel,
    DamageIncreased,
    OdometerMilestone,
};

struct VehicleEvent final {
    VehicleEventType type = VehicleEventType::OffRoadEntered;
    float value = 0.0F;
    float odometerMeters = 0.0F;
    float tripSeconds = 0.0F;
};

struct DrivingTelemetrySnapshot final {
    float speedKmh = 0.0F;
    float engineRpm = 0.0F;
    int32_t selectedGear = 0;
    float fuelLiters = 0.0F;
    float fuel01 = 0.0F;
    float damage01 = 0.0F;
    float odometerMeters = 0.0F;
    float tripSeconds = 0.0F;
    uint32_t groundedWheelCount = 0;
    float suspensionCompression = 0.0F;
    float longitudinalSlip = 0.0F;
    float lateralSlip = 0.0F;
    bool offRoad = false;
};

class VehicleEventCollector final {
public:
    void reset();
    void update(const VehicleState& state);

    [[nodiscard]] const std::vector<VehicleEvent>& events() const { return events_; }
    [[nodiscard]] const DrivingTelemetrySnapshot& telemetry() const { return telemetry_; }

private:
    std::vector<VehicleEvent> events_;
    DrivingTelemetrySnapshot telemetry_{};
    bool wasOffRoad_ = false;
    bool lowFuelEmitted_ = false;
    float lastDamage01_ = 0.0F;
    float nextOdometerMilestoneMeters_ = 100.0F;
};

} // namespace roadforge::vehicle
