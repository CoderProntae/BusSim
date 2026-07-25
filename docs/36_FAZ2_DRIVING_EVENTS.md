# Faz 2.9 — Sürüş Telemetry / Event Katmanı

Bu teslim, yakıt/hasar/odometre/yol dışı sinyallerini ileride ekonomi ve sefer sistemlerinin dinleyebileceği event/telemetry katmanına ayırır.

## Eklenen dosyalar

```text
app/src/main/cpp/include/roadforge/vehicle/VehicleEvents.hpp
app/src/main/cpp/src/vehicle/VehicleEvents.cpp
```

## DrivingTelemetrySnapshot

Anlık sürüş özeti:

```text
speedKmh
engineRpm
selectedGear
fuelLiters
fuel01
damage01
odometerMeters
tripSeconds
groundedWheelCount
suspensionCompression
longitudinalSlip
lateralSlip
offRoad
```

Bu snapshot ileride HUD, sefer sistemi ve debug overlay tarafından tüketilecek.

## VehicleEvent

İlk event tipleri:

```text
OffRoadEntered
OffRoadExited
LowFuel
DamageIncreased
OdometerMilestone
```

Bu event'ler şimdilik logcat'e basılır. Faz 3/Faz 4 içinde sefer değerlendirme, yolcu memnuniyeti ve ekonomi/ledger sistemleri bu sinyalleri dinleyecek.

## VehicleEventCollector

`VehicleState` değerlerini izler ve state geçişlerinden event üretir:

- Yol dışına çıkış/giriş
- Düşük yakıt
- Hasar artışı
- Her 100 metrede odometre milestone

## Kullanıcı testi gerekli mi?

Görsel değişiklik beklenmez. CI build başarısı yeterlidir.

## Sonraki teslim

Faz 2.10 hedefi:

1. Sürüş stabilizasyonu.
2. Hız/turn/fren tuning.
3. Faz 2 kapanış APK'sı.
4. Faz 3 sefer vertical slice hazırlığı.
