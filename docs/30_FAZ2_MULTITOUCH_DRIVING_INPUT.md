# Faz 2.3 — Çoklu Dokunmalı Mobil Sürüş Input Bölgeleri

Bu teslim, geçici tek parmak debug input yerine aynı anda gaz + direksiyon yapılabilen ilk mobil sürüş input hattını ekler.

## Java → JNI değişikliği

`MainActivity` artık yalnızca action pointer koordinatını değil, aktif tüm pointer koordinatlarını native tarafa gönderir:

```text
MotionEvent pointer list
→ float[] xs / float[] ys
→ nativeTouchState
→ C++ Engine::onTouchState
→ InputSystem::handleTouchState
```

Eski `nativeTouch` yolu uyumluluk için duruyor, ancak ana akış artık `nativeTouchState`.

## Input bölgeleri

Geçici Faz 2.3 mapping:

| Bölge | Davranış |
|---|---|
| Sol %50 ekran | Direksiyon şeridi |
| Sağ alt %25 | Gaz / throttle |
| Orta-sağ alt %25 | Fren / brake |

Bu sayede iki parmakla:

```text
sol başparmak = direksiyon
sağ başparmak = gaz veya fren
```

aynı anda çalışabilir.

## VehicleController değişikliği

Pedal aktifken steering'i sıfırlayan geçici kural kaldırıldı. Artık `InputSnapshot` doğru şekilde ayrı bölgelerden geldiği için steering + throttle aynı anda araç komutuna geçebilir.

## Telefon kabul testi

Bu adım görsel test gerektirir.

1. APK açılır.
2. Otobüs ve yol görünür.
3. Sağ alt bölgeye basılı tut: otobüs hızlanır.
4. Sağ alt gazı basılı tutarken sol yarıda parmağını sola/sağa taşı: otobüs yön değiştirmelidir.
5. Sağ alt gazı bırakıp orta-sağ alt fren bölgesine bas: otobüs yavaşlamalıdır.
6. Pause/resume çökmez.

## Not

Bu hâlâ final HUD değildir. Faz 2.4 ve sonrası için gerçek görsel sürüş kontrolleri, safe-area ve sol/sağ el düzeni ayrıca tasarlanacak.
