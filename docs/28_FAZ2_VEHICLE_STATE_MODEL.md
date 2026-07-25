# Faz 2.1 — Vehicle Input / State Modeli

Faz 2 sürüş vertical slice başlangıcıdır. Bu teslim fizik/sürüş hareketi üretmez; önce araç sisteminin komut ve durum sınırını kurar.

## Eklenen dosyalar

```text
app/src/main/cpp/include/roadforge/vehicle/VehicleTypes.hpp
app/src/main/cpp/include/roadforge/vehicle/VehicleController.hpp
app/src/main/cpp/src/vehicle/VehicleController.cpp
```

## VehicleCommand

İlk araç komut modeli:

```text
VehicleCommand {
  throttle,
  brake,
  steering,
  retarder,
  handbrake,
  gearMode
}
```

Bu model ileride mobil HUD, replay sistemi, AI sürücü ve fizik backend arasında ortak sınır olacak.

## VehicleState

İlk araç state snapshot modeli:

```text
VehicleState {
  throttle,
  brake,
  steering,
  retarder,
  handbrake,
  gearMode,
  speedMetersPerSecond,
  engineRpm,
  selectedGear,
  steeringWheelDegrees
}
```

Şimdilik hız fiziksel olarak üretilmiyor; `engineRpm` ve `steeringWheelDegrees` preview/debug değerleri olarak güncelleniyor.

## VehicleController

`VehicleController` görevleri:

- `InputSnapshot` → `VehicleCommand` map eder.
- Komut değerlerini clamp eder.
- Pedal/direksiyon değerlerini fixed tick içinde yumuşatır.
- `VehicleState` snapshot üretir.

Geçici input ayrımı:

| Dokunma | Vehicle command |
|---|---|
| Alt sağ | Throttle |
| Alt sol | Brake |
| Üst/orta sol-sağ | Steering |

Pedal aktifken steering sıfırlanır. Böylece eski debug kamera karışıklığı tekrar etmez.

## Engine bağlantısı

Yeni akış:

```text
InputSystem snapshot
→ VehicleController::commandFromInput
→ VehicleController::setCommand
→ fixed 60 Hz VehicleController::fixedUpdate
→ VehicleState snapshot
```

Her 240 frame'de logcat'e frame stats ile birlikte vehicle preview state de basılır.

## Kullanıcı testi gerekli mi?

Görsel değişiklik beklenmez. Bu adım için APK indirip test etmek şart değildir; CI build başarısı yeterli kabul edilir.

## Faz 2 tahmini

Faz 2 toplam tahmini **10–12 ara adım** sürecek.

Sıradaki adımlar:

1. Faz 2.2 — Kinematik otobüs prototipi: placeholder otobüs hızlanır/frenler/döner.
2. Faz 2.3 — Mobil sürüş HUD input bölgeleri.
3. Faz 2.4 — Takip/kabin kamera prototipi.
4. Faz 2.5 — Test pisti/yol segmentleri.
5. Faz 2.6+ — Fizik adapter, raycast süspansiyon, motor/şanzıman/fren modeli.
