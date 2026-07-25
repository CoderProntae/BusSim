#pragma once

#include "roadforge/game/GameTypes.hpp"

#include <cstdint>

namespace roadforge::game {

/**
 * Offline single-player game simulation facade.
 *
 * This sits above vehicle/world/renderer. It owns company, route, trip,
 * passenger, weather, traffic and ledger state. Current implementation is a
 * complete local MVP simulation model; later phases can connect these values to
 * UI screens and persistence without changing gameplay ownership.
 */
class GameSimulation final {
public:
    void resetNewGame();
    void fixedUpdate(double fixedDeltaSeconds, const vehicle::VehicleState& vehicleState);

    [[nodiscard]] bool hasActiveTrip() const;
    [[nodiscard]] const GameSnapshot& snapshot() const { return snapshot_; }

    bool startTrip(uint32_t routeId, uint32_t busId, int32_t ticketPriceCents);
    void forceCompleteActiveTrip();

private:
    [[nodiscard]] RouteRecord* findRoute(uint32_t routeId);
    [[nodiscard]] const RouteRecord* findRoute(uint32_t routeId) const;
    [[nodiscard]] BusRecord* findBus(uint32_t busId);
    [[nodiscard]] const BusRecord* findBus(uint32_t busId) const;

    void seedInitialContent();
    void updateActiveTrip(double fixedDeltaSeconds, const vehicle::VehicleState& vehicleState);
    void updateWeather(double fixedDeltaSeconds);
    void updateTraffic(double fixedDeltaSeconds);
    void settleTrip();
    void addLedger(LedgerCategory category, int32_t debitCents, int32_t creditCents, uint32_t referenceId);
    [[nodiscard]] uint32_t deterministicPassengerCount(const RouteRecord& route, int32_t ticketPriceCents) const;

    GameSnapshot snapshot_{};
    uint32_t nextTripId_ = 1;
    uint32_t weatherSeed_ = 0xC0FFEEu;
    float lastTripOdometerMeters_ = 0.0F;
};

} // namespace roadforge::game
