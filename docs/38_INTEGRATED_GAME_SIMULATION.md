# Integrated Game Simulation Layer

This milestone adds the first integrated offline game simulation layer above the driving/vehicle stack.

## Scope

The native game now has data models and runtime simulation for:

- Company cash and reputation
- Owned buses
- Routes and unlock state
- Active trip lifecycle
- Passenger groups and satisfaction
- Ledger entries
- Weather state
- Lightweight traffic density state
- Trip progress driven by vehicle odometer

## Runtime flow

```text
VehicleController / VehicleState
→ VehicleEventCollector
→ GameSimulation::fixedUpdate
→ Trip progress / passenger satisfaction / weather / traffic / ledger
```

The system is offline and deterministic enough for local testing. It is not final UI/UX; it is the gameplay backbone that the future menu/HUD/save systems will consume.

## Default new-game state

- Company starts with one owned intercity coach.
- Ankara → Eskisehir route is unlocked.
- A default trip starts automatically for bootstrap testing.
- Passenger count is generated deterministically from route demand, ticket price and company reputation.

## Why this matters

This moves the project from only a driving renderer prototype toward an actual game loop:

```text
Drive vehicle
→ accumulate trip progress
→ affect comfort/satisfaction
→ settle ledger/cash/reputation
```

Future work will connect these systems to a real UI and local save file.
