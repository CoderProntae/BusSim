# Faz 1.2 — Native Input Action/Axis ve Frame Stats

Bu teslim, Android raw touch olaylarını oyun sistemlerinin kullanacağı kararlı native input sınırına çevirir ve temel frame istatistiklerini toplamaya başlar.

## Eklenen dosyalar

```text
app/src/main/cpp/include/roadforge/input/InputSystem.hpp
app/src/main/cpp/src/input/InputSystem.cpp
app/src/main/cpp/include/roadforge/core/FrameStats.hpp
app/src/main/cpp/src/core/FrameStats.cpp
```

## InputSystem

Java katmanı hâlâ yalnızca raw `MotionEvent` bilgisini JNI ile taşır. Oyun tarafındaki anlamlandırma C++ içindedir.

İlk debug mapping:

| Dokunma bölgesi | Native axis/action |
|---|---|
| Ekranın yatay konumu | `steering`: -1 sol, +1 sağ |
| Alt sağ | `throttle = 1` |
| Alt sol | `brake = 1` |
| Dokunma aktif | `primaryTouchDown = true` |

Bu mapping kalıcı HUD tasarımı değildir. Faz 2 araç sürüşüne kadar input sistemi test edilsin diye kondu. Gerçek mobil sürüş panelinde buton/joystick bölgeleri safe-area ve sağ/sol el ayarına göre ayrıca tanımlanacak.

## FrameStats

Toplanan ilk metrikler:

- Toplam frame sayısı
- Ortalama frame süresi
- Tahmini FPS
- Ortalama fixed simulation step sayısı
- Dropped accumulated simulation time sayısı

Şimdilik native log'a her 240 frame'de bir özet basılır. Faz 1 içinde text/debug overlay geldiğinde bu değerler ekranda gösterilecek.

## Renderer debug bağlantısı

Renderer şu debug verilerini alır:

```text
setSimulationTiming(appTimeSeconds, simulationTick, interpolationAlpha)
setInputDebug(steering, throttle, brake, touchActive)
```

Şimdilik yalnızca clear color tonuna çok hafif etki eder:

- Steering sağ/sol: kırmızı/mavi tonu az değişir.
- Throttle: yeşil tonunu az artırır.
- Brake: kırmızı tonunu az artırır.
- Dokunma: önceki turuncu pulse hâlâ çalışır.

## Telefon kabul testi

1. APK açılır.
2. Lacivert Vulkan ekran görünür.
3. Dokununca turuncu tepki hâlâ gelir.
4. Alt sağ / alt sol / sol-sağ dokunuşlarda arka plan tonunda çok hafif fark görülebilir.
5. Pause/resume çökmez.

## Sonraki teslim

Faz 1.3 için hedef:

1. Shader kaynak klasörü ve SPIR-V build adımı.
2. Vulkan pipeline layout / graphics pipeline.
3. Vertex buffer ile ilk üçgen veya quad çizimi.
4. Bu çizimin telefonda doğrulanması.
