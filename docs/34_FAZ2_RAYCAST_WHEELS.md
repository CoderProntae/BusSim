# Faz 2.7 — Raycast Teker / Süspansiyon Telemetry Prototipi

Bu teslim, fizik adapter sınırını ilk kez araç sistemiyle bağlar. Henüz tam rigid-body sürüş fiziği değildir; ama araç teker noktaları üzerinden zemin raycast'i yapıp telemetry üretir.

## Eklenen dosyalar

```text
app/src/main/cpp/include/roadforge/physics/RaycastVehicleBackend.hpp
app/src/main/cpp/src/physics/RaycastVehicleBackend.cpp
```

## Yeni akış

```text
VehicleController kinematic state
→ vehicle transform
→ RaycastVehicleBackend::step
→ NullPhysicsWorld ground-plane raycast
→ VehiclePhysicsTelemetry
→ VehicleState telemetry alanları
→ World transform/render
```

## Teker/süspansiyon prototipi

Varsayılan placeholder bus config:

- 4 teker
- ön tekerler steering flag
- arka tekerler driven flag
- teker radius / rest suspension length
- spring/damper değerleri ileride kullanılmak üzere config içinde

Şimdilik üretilen telemetry:

```text
groundedWheelCount
averageSuspensionCompression
longitudinalSlip
lateralSlip
```

Bu telemetry henüz kuvvet uygulamıyor. Faz 2.8+ içinde motor/fren/şanzıman ve daha sonra raycast suspension kuvvetlerine bağlanacak.

## Kullanıcı testi gerekli mi?

Görsel değişiklik beklenmez. Build başarısı yeterlidir.

## Sonraki teslim

Faz 2.8 hedefi:

1. Motor/şanzıman/fren state modelini genişletmek.
2. Kinematik hızlanmayı daha otobüs hissine yakın yapmak.
3. Retarder ve fren ayrımını hazırlamak.
