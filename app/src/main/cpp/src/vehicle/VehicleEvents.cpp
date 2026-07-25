#include "roadforge/vehicle/VehicleEvents.hpp"

#include <algorithm>

namespace roadforge::vehicle {

namespace {
constexpr float kMetersPerSecondToKmh = 3.6F;
constexpr float kLowFuelThreshold01 = 0.15F;
constexpr float kDamageEventDelta = 0.02F;
} // namespace

void VehicleEventCollector::reset() {
    events_.clear();
    telemetry_ = {};
    wasOffRoad_ = false;
    lowFuelEmitted_ = false;
    lastDamage01_ = 0.0F;
    nextOdometerMilestoneMeters_ = 100.0F;
}

void VehicleEventCollector::update(const VehicleState& state) {
    events_.clear();

    telemetry_.speedKmh = state.speedMetersPerSecond * kMetersPerSecondToKmh;
    telemetry_.engineRpm = state.engineRpm;
    telemetry_.selectedGear = state.selectedGear;
    telemetry_.fuelLiters = state.fuelLiters;
    telemetry_.fuel01 = state.fuelCapacityLiters > 0.0F ? std::clamp(state.fuelLiters / state.fuelCapacityLiters, 0.0F, 1.0F) : 0.0F;
    telemetry_.damage01 = std::clamp(state.damage01, 0.0F, 1.0F);
    telemetry_.odometerMeters = state.odometerMeters;
    telemetry_.tripSeconds = state.tripSeconds;
    telemetry_.groundedWheelCount = state.groundedWheelCount;
    telemetry_.suspensionCompression = state.averageSuspensionCompression;
    telemetry_.longitudinalSlip = state.longitudinalSlip;
    telemetry_.lateralSlip = state.lateralSlip;
    telemetry_.offRoad = state.offRoad01 > 0.25F;

    if (telemetry_.offRoad && !wasOffRoad_) {
        events_.push_back({ VehicleEventType::OffRoadEntered, state.offRoad01, state.odometerMeters, state.tripSeconds });
    } else if (!telemetry_.offRoad && wasOffRoad_) {
        events_.push_back({ VehicleEventType::OffRoadExited, 0.0F, state.odometerMeters, state.tripSeconds });
    }
    wasOffRoad_ = telemetry_.offRoad;

    if (!lowFuelEmitted_ && telemetry_.fuel01 <= kLowFuelThreshold01) {
        lowFuelEmitted_ = true;
        events_.push_back({ VehicleEventType::LowFuel, telemetry_.fuel01, state.odometerMeters, state.tripSeconds });
    }

    if (telemetry_.damage01 - lastDamage01_ >= kDamageEventDelta) {
        lastDamage01_ = telemetry_.damage01;
        events_.push_back({ VehicleEventType::DamageIncreased, telemetry_.damage01, state.odometerMeters, state.tripSeconds });
    }

    if (state.odometerMeters >= nextOdometerMilestoneMeters_) {
        events_.push_back({ VehicleEventType::OdometerMilestone, nextOdometerMilestoneMeters_, state.odometerMeters, state.tripSeconds });
        while (state.odometerMeters >= nextOdometerMilestoneMeters_) {
            nextOdometerMilestoneMeters_ += 100.0F;
        }
    }
}

} // namespace roadforge::vehicle
