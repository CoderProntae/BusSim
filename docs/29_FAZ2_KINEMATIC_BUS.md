# Faz 2.2 — Kinematik Otobüs Prototipi

Bu teslimde placeholder otobüs ilk kez araç state'ine göre hareket eder.

## Eklenen davranış

`VehicleController` artık yalnızca throttle/brake/steering state'i tutmuyor; basit kinematik hareket de üretiyor:

```text
speedMetersPerSecond
positionX
positionZ
headingRadians
```

Bu fizik simülasyonu değildir. Raycast teker, süspansiyon, motor torku ve lastik modeli sonraki Faz 2 adımlarında gelecek. Bu adımın amacı input → vehicle state → world transform → renderer zincirini görsel olarak doğrulamaktır.

## Geçici sürüş kontrolü

| Dokunma | Davranış |
|---|---|
| Alt sağ | Gaz / hızlanma |
| Alt sol | Fren |
| Üst/orta sol-sağ | Direksiyon |

Tek parmak debug input olduğu için aynı anda gaz + direksiyon henüz yok. Bu, Faz 2.3 mobil sürüş HUD input bölgelerinde düzeltilecek.

## World bağlantısı

World artık `VehicleState` alır:

```text
VehicleController::fixedUpdate
→ VehicleState
→ World::fixedUpdate(vehicleState)
→ BusPlaceholder Transform
→ RenderProxy
→ Renderer
```

Otobüs mesh'i local origin etrafına merkezlendi; world transform ile konum/heading uygulanıyor.

## Kamera

Debug kamera artık aracı takip eden basit sabit takip kamerası gibi davranır:

```text
eye = bus position - forward * distance + height
 target = bus position + forward * lead
```

Bu final sürüş kamerası değildir, ama ilk hareket testini görünür yapar.

## Telefon kabul testi

Bu adım görsel test gerektirir.

1. APK açılır.
2. Yol ve turuncu placeholder otobüs görünür.
3. Alt sağa basılı tutunca otobüs ileri hareket etmeye başlar.
4. Alt sola basılı tutunca otobüs yavaşlar/frenler.
5. Otobüs biraz hızlandıktan sonra üst/orta sol veya sağ tarafa basılı tutunca yönü değişir.
6. Tek parmak debug input nedeniyle gaz ve direksiyon aynı anda beklenmez.
7. Pause/resume çökmez.

## Sonraki teslim

Faz 2.3 hedefi:

1. Gerçek mobil sürüş HUD input bölgeleri.
2. Aynı anda gaz + direksiyon desteği.
3. Safe-area ve sol/sağ el yerleşimi için temel.
