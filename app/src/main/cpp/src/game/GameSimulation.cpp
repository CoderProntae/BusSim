#include "roadforge/game/GameSimulation.hpp"

#include <algorithm>
#include <cmath>

namespace roadforge::game {

namespace {
constexpr float kMetersToKm = 0.001F;
constexpr float kFuelPriceCentsPerLiter = 4500.0F;
constexpr float kServiceCostCentsPerWear01 = 250000.0F;
constexpr float kDamageCostCentsPerDamage01 = 450000.0F;

uint32_t lcg(uint32_t& seed) {
    seed = (1664525u * seed) + 1013904223u;
    return seed;
}

float random01(uint32_t& seed) {
    return static_cast<float>((lcg(seed) >> 8) & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
}
} // namespace

void GameSimulation::resetNewGame() {
    snapshot_ = {};
    nextTripId_ = 1;
    weatherSeed_ = 0xC0FFEEu;
    lastTripOdometerMeters_ = 0.0F;
    seedInitialContent();
    (void)startTrip(1, 1, 65000);
}

void GameSimulation::fixedUpdate(double fixedDeltaSeconds, const vehicle::VehicleState& vehicleState) {
    snapshot_.worldTimeSeconds += std::clamp(fixedDeltaSeconds, 0.0, 0.1);
    updateWeather(fixedDeltaSeconds);
    updateTraffic(fixedDeltaSeconds);
    updateActiveTrip(fixedDeltaSeconds, vehicleState);
}

bool GameSimulation::hasActiveTrip() const {
    return snapshot_.activeTrip.state != TripState::Idle && snapshot_.activeTrip.state != TripState::Completed;
}

bool GameSimulation::startTrip(uint32_t routeId, uint32_t busId, int32_t ticketPriceCents) {
    const RouteRecord* route = findRoute(routeId);
    const BusRecord* bus = findBus(busId);
    if (route == nullptr || bus == nullptr || !route->unlocked || !bus->owned || hasActiveTrip()) {
        return false;
    }

    TripRecord trip{};
    trip.id = nextTripId_++;
    trip.routeId = routeId;
    trip.busId = busId;
    trip.state = TripState::Boarding;
    trip.ticketPriceCents = ticketPriceCents;
    trip.passengerCount = deterministicPassengerCount(*route, ticketPriceCents);

    uint32_t seed = routeId * 977u + busId * 131u + static_cast<uint32_t>(ticketPriceCents);
    uint32_t remaining = trip.passengerCount;
    while (remaining > 0) {
        PassengerGroup group{};
        group.seed = lcg(seed);
        group.count = std::min<uint32_t>(remaining, 1u + (lcg(seed) % 4u));
        group.priceSensitivity01 = 0.25F + (random01(seed) * 0.65F);
        group.comfortExpectation01 = 0.45F + (random01(seed) * 0.45F);
        group.satisfaction01 = 0.75F;
        trip.passengers.push_back(group);
        remaining -= group.count;
    }

    snapshot_.activeTrip = trip;
    lastTripOdometerMeters_ = 0.0F;
    return true;
}

void GameSimulation::forceCompleteActiveTrip() {
    if (hasActiveTrip()) {
        snapshot_.activeTrip.progress01 = 1.0F;
        snapshot_.activeTrip.state = TripState::Settlement;
        settleTrip();
    }
}

RouteRecord* GameSimulation::findRoute(uint32_t routeId) {
    for (RouteRecord& route : snapshot_.routes) {
        if (route.id == routeId) {
            return &route;
        }
    }
    return nullptr;
}

const RouteRecord* GameSimulation::findRoute(uint32_t routeId) const {
    for (const RouteRecord& route : snapshot_.routes) {
        if (route.id == routeId) {
            return &route;
        }
    }
    return nullptr;
}

BusRecord* GameSimulation::findBus(uint32_t busId) {
    for (BusRecord& bus : snapshot_.company.buses) {
        if (bus.id == busId) {
            return &bus;
        }
    }
    return nullptr;
}

const BusRecord* GameSimulation::findBus(uint32_t busId) const {
    for (const BusRecord& bus : snapshot_.company.buses) {
        if (bus.id == busId) {
            return &bus;
        }
    }
    return nullptr;
}

void GameSimulation::seedInitialContent() {
    snapshot_.company.cashCents = 25000000;
    snapshot_.company.reputation01 = 0.55F;
    snapshot_.company.buses.push_back(BusRecord{ 1, "RoadForge Intercity Coach V1", 12500000, 46, 0.0F, 180.0F, 180.0F, true });
    snapshot_.company.unlockedRouteIds.push_back(1);

    snapshot_.routes.push_back(RouteRecord{ 1, "Ankara Terminal", "Eskisehir Terminal", 233.0F, 185.0F, 0.72F, 45000, true });
    snapshot_.routes.push_back(RouteRecord{ 2, "Ankara Terminal", "Konya Terminal", 262.0F, 210.0F, 0.64F, 52000, false });

    snapshot_.weather.kind = WeatherKind::Clear;
    snapshot_.weather.rain01 = 0.0F;
    snapshot_.weather.fog01 = 0.0F;
    snapshot_.weather.gripMultiplier = 1.0F;
    snapshot_.traffic.density01 = 0.35F;
    snapshot_.traffic.simulatedVehicleCount = 18;
}

void GameSimulation::updateActiveTrip(double fixedDeltaSeconds, const vehicle::VehicleState& vehicleState) {
    if (!hasActiveTrip()) {
        return;
    }

    TripRecord& trip = snapshot_.activeTrip;
    if (trip.state == TripState::Boarding && snapshot_.worldTimeSeconds > 3.0) {
        trip.state = TripState::Departing;
    } else if (trip.state == TripState::Departing && vehicleState.speedMetersPerSecond > 1.5F) {
        trip.state = TripState::EnRoute;
    }

    const float currentOdometer = vehicleState.odometerMeters;
    const float deltaMeters = std::max(0.0F, currentOdometer - lastTripOdometerMeters_);
    lastTripOdometerMeters_ = currentOdometer;
    trip.drivenKm += deltaMeters * kMetersToKm;

    const RouteRecord* route = findRoute(trip.routeId);
    if (route != nullptr && route->distanceKm > 0.0F) {
        trip.progress01 = std::clamp(trip.drivenKm / route->distanceKm, 0.0F, 1.0F);
    }

    const float comfortPenalty = (vehicleState.lateralSlip * 0.08F) + (vehicleState.longitudinalSlip < -0.2F ? 0.03F : 0.0F) + (vehicleState.damage01 * 0.2F);
    trip.comfort01 = std::clamp(trip.comfort01 - (comfortPenalty * static_cast<float>(fixedDeltaSeconds) * 0.05F), 0.0F, 1.0F);
    trip.passengerSatisfaction01 = std::clamp((trip.comfort01 * 0.65F) + (snapshot_.company.reputation01 * 0.35F), 0.0F, 1.0F);

    if (trip.progress01 >= 1.0F) {
        trip.state = TripState::Settlement;
        settleTrip();
    }
}

void GameSimulation::updateWeather(double fixedDeltaSeconds) {
    snapshot_.weather.nextChangeSeconds -= fixedDeltaSeconds;
    if (snapshot_.weather.nextChangeSeconds > 0.0) {
        return;
    }

    const float roll = random01(weatherSeed_);
    if (roll < 0.55F) {
        snapshot_.weather.kind = WeatherKind::Clear;
        snapshot_.weather.rain01 = 0.0F;
        snapshot_.weather.fog01 = 0.0F;
        snapshot_.weather.gripMultiplier = 1.0F;
    } else if (roll < 0.78F) {
        snapshot_.weather.kind = WeatherKind::Cloudy;
        snapshot_.weather.rain01 = 0.0F;
        snapshot_.weather.fog01 = 0.05F;
        snapshot_.weather.gripMultiplier = 0.96F;
    } else if (roll < 0.93F) {
        snapshot_.weather.kind = WeatherKind::Rain;
        snapshot_.weather.rain01 = 0.65F;
        snapshot_.weather.fog01 = 0.12F;
        snapshot_.weather.gripMultiplier = 0.82F;
    } else {
        snapshot_.weather.kind = WeatherKind::Fog;
        snapshot_.weather.rain01 = 0.0F;
        snapshot_.weather.fog01 = 0.75F;
        snapshot_.weather.gripMultiplier = 0.9F;
    }
    snapshot_.weather.nextChangeSeconds = 180.0 + (random01(weatherSeed_) * 240.0);
}

void GameSimulation::updateTraffic(double fixedDeltaSeconds) {
    const float wave = 0.5F + 0.5F * std::sin(static_cast<float>(snapshot_.worldTimeSeconds * 0.01));
    snapshot_.traffic.density01 = std::clamp(0.25F + wave * 0.45F, 0.0F, 1.0F);
    snapshot_.traffic.simulatedVehicleCount = 10U + static_cast<uint32_t>(snapshot_.traffic.density01 * 36.0F);
    snapshot_.traffic.nearVehicleCount = static_cast<uint32_t>(snapshot_.traffic.density01 * 8.0F);
    (void)fixedDeltaSeconds;
}

void GameSimulation::settleTrip() {
    TripRecord& trip = snapshot_.activeTrip;
    const RouteRecord* route = findRoute(trip.routeId);
    BusRecord* bus = findBus(trip.busId);
    if (route == nullptr || bus == nullptr) {
        trip.state = TripState::Completed;
        return;
    }

    const int32_t ticketIncome = static_cast<int32_t>(trip.passengerCount) * trip.ticketPriceCents;
    const int32_t fuelCost = static_cast<int32_t>(trip.drivenKm * 0.28F * kFuelPriceCentsPerLiter);
    const int32_t serviceCost = static_cast<int32_t>(bus->wear01 * kServiceCostCentsPerWear01);
    const int32_t damageCost = static_cast<int32_t>((1.0F - trip.comfort01) * kDamageCostCentsPerDamage01);
    addLedger(LedgerCategory::TicketIncome, 0, ticketIncome, trip.id);
    addLedger(LedgerCategory::FuelCost, fuelCost, 0, trip.id);
    addLedger(LedgerCategory::ServiceCost, serviceCost, 0, trip.id);
    addLedger(LedgerCategory::DamageCost, damageCost, 0, trip.id);

    snapshot_.company.reputation01 = std::clamp((snapshot_.company.reputation01 * 0.92F) + (trip.passengerSatisfaction01 * 0.08F), 0.0F, 1.0F);
    bus->wear01 = std::clamp(bus->wear01 + (trip.drivenKm * 0.00008F) + ((1.0F - trip.comfort01) * 0.02F), 0.0F, 1.0F);
    bus->fuelLiters = std::max(0.0F, bus->fuelLiters - (trip.drivenKm * 0.28F));
    trip.state = TripState::Completed;
}

void GameSimulation::addLedger(LedgerCategory category, int32_t debitCents, int32_t creditCents, uint32_t referenceId) {
    snapshot_.company.cashCents += creditCents;
    snapshot_.company.cashCents -= debitCents;
    snapshot_.company.ledger.push_back(LedgerEntry{ snapshot_.worldTimeSeconds, category, debitCents, creditCents, referenceId });
}

uint32_t GameSimulation::deterministicPassengerCount(const RouteRecord& route, int32_t ticketPriceCents) const {
    const float referencePrice = 65000.0F;
    const float priceFactor = std::clamp(referencePrice / std::max(1.0F, static_cast<float>(ticketPriceCents)), 0.35F, 1.25F);
    const float demand = std::clamp(route.baseDemand01 * priceFactor * snapshot_.company.reputation01, 0.05F, 1.0F);
    const uint32_t capacity = snapshot_.company.buses.empty() ? 42U : snapshot_.company.buses.front().capacity;
    return std::clamp(static_cast<uint32_t>(static_cast<float>(capacity) * demand), 4U, capacity);
}

} // namespace roadforge::game
