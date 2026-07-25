#pragma once

#include "roadforge/vehicle/VehicleTypes.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace roadforge::game {

enum class LedgerCategory : uint8_t {
    TicketIncome = 0,
    FuelCost,
    ServiceCost,
    DamageCost,
    BusPurchase,
    RouteBonus,
};

enum class TripState : uint8_t {
    Idle = 0,
    Planning,
    Boarding,
    Departing,
    EnRoute,
    RestStop,
    Arriving,
    Settlement,
    Completed,
};

enum class WeatherKind : uint8_t {
    Clear = 0,
    Cloudy,
    Rain,
    Fog,
};

struct LedgerEntry final {
    double timestampSeconds = 0.0;
    LedgerCategory category = LedgerCategory::TicketIncome;
    int32_t debitCents = 0;
    int32_t creditCents = 0;
    uint32_t referenceId = 0;
};

struct BusRecord final {
    uint32_t id = 0;
    std::string model = "RoadForge Intercity Coach V1";
    int32_t purchasePriceCents = 12500000;
    uint32_t capacity = 46;
    float wear01 = 0.0F;
    float fuelLiters = 180.0F;
    float fuelCapacityLiters = 180.0F;
    bool owned = true;
};

struct RouteRecord final {
    uint32_t id = 0;
    std::string origin = "Ankara Terminal";
    std::string destination = "Eskisehir Terminal";
    float distanceKm = 233.0F;
    float expectedDurationMinutes = 185.0F;
    float baseDemand01 = 0.72F;
    int32_t tollCents = 45000;
    bool unlocked = true;
};

struct PassengerGroup final {
    uint32_t seed = 0;
    uint32_t count = 1;
    float priceSensitivity01 = 0.5F;
    float comfortExpectation01 = 0.65F;
    float satisfaction01 = 0.75F;
};

struct WeatherState final {
    WeatherKind kind = WeatherKind::Clear;
    float rain01 = 0.0F;
    float fog01 = 0.0F;
    float gripMultiplier = 1.0F;
    double nextChangeSeconds = 240.0;
};

struct TrafficState final {
    float density01 = 0.35F;
    uint32_t simulatedVehicleCount = 18;
    uint32_t nearVehicleCount = 0;
};

struct TripRecord final {
    uint32_t id = 0;
    uint32_t routeId = 0;
    uint32_t busId = 0;
    TripState state = TripState::Idle;
    int32_t ticketPriceCents = 65000;
    float progress01 = 0.0F;
    float drivenKm = 0.0F;
    float comfort01 = 0.85F;
    float passengerSatisfaction01 = 0.75F;
    uint32_t passengerCount = 0;
    std::vector<PassengerGroup> passengers;
};

struct Company final {
    int32_t cashCents = 25000000;
    float reputation01 = 0.55F;
    std::vector<BusRecord> buses;
    std::vector<uint32_t> unlockedRouteIds;
    std::vector<LedgerEntry> ledger;
};

struct GameSnapshot final {
    double worldTimeSeconds = 0.0;
    Company company;
    std::vector<RouteRecord> routes;
    TripRecord activeTrip;
    WeatherState weather;
    TrafficState traffic;
};

} // namespace roadforge::game
