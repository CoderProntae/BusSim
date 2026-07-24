# Faz 1.9 — Debug Kamera Controller Bağlantısı

Bu teslim, kamerayı renderer içinde sabit değer olmaktan çıkarıp World tarafından yönetilen ilk debug kamera state'ine taşır.

## Eklenen davranış

`World` artık şu kamera state'ini tutar:

```text
DebugCamera { eye, target, up, fovYRadians }
```

Engine her fixed simulation tick içinde input snapshot'tan küçük bir `CameraControlInput` üretir:

```text
CameraControlInput { steering, throttle, brake, active }
```

Akış:

```text
InputSystem snapshot
→ Engine CameraControlInput
→ World::fixedUpdate
→ DebugCamera update
→ Renderer::setDebugCamera
→ view/projection matrix
→ Vulkan draw
```

## Geçici debug kontrol şeması

Bu hâlâ nihai oyun kontrolü değildir; sadece kamera/world/render hattını doğrulamak için kullanılır.

| Input | Debug kamera etkisi |
|---|---|
| Sol / sağ dokunma | Kamerayı yol etrafında yavaşça orbit eder |
| Alt sağ | Kamerayı yola yaklaştırır |
| Alt sol | Kamerayı yoldan uzaklaştırır |

Diagnostic renkler hâlâ aktiftir. Bu yüzden kısa tıklamada önce renk görürsün; kamera hareketi daha çok basılı tutunca fark edilir.

## Renderer değişikliği

Renderer artık hardcoded kamera kullanmıyor. Kamera verisi şu API ile giriyor:

```cpp
setDebugCamera(eye, target, up, fovYRadians)
```

## Telefon kabul testi

1. APK açılır.
2. Perspektif yol plakası görünür.
3. Alt sağ/alt sol/sol/sağ diagnostic renkleri çalışır.
4. Sol veya sağ tarafa 1-2 saniye basılı tutunca yol plakasının perspektifi çok hafif değişmelidir.
5. Alt sağa basılı tutunca kamera biraz yaklaşır, alt sola basılı tutunca biraz uzaklaşır.
6. Pause/resume çökmez.

## Sonraki teslim

Faz 1.10 hedefi:

1. Debug overlay için basit görsel frame stats yüzeyi.
2. Sonrasında placeholder otobüs mesh ve yol parçası hazırlığı.
