# Faz 1.15 — Faz 1 Kapanış Temizliği ve Faz 2 Hazırlığı

Bu teslim, Faz 1'i kapatmaya hazırlayan küçük temizlik ve planlama adımıdır.

## Yapılan temizlik

Debug overlay denemesi geçici olarak kapalıydı fakat renderer hâlâ overlay vertex/index buffer'ları ayırıyordu. Bu gereksiz kaynak ayrımı kaldırıldı.

Kaldırılan/geçici ertelenen parçalar:

- `createDebugOverlayResources`
- `cleanupDebugOverlayResources`
- `updateDebugOverlayBuffers`
- Overlay frame-in-flight buffer üyeleri

Overlay sistemi tamamen iptal edilmedi; doğru çözüm olarak ileride ayrı bir 2D UI/overlay pipeline ile geri gelecek.

## Faz 1 mevcut sonuç

Faz 1 sonunda elimizde şu çalışan iskelet var:

```text
Android SurfaceView
→ JNI
→ C++ Engine
→ SimulationClock
→ InputSystem
→ World/ECS başlangıcı
→ RenderProxy köprüsü
→ Vulkan renderer
→ Shader pipeline
→ Vertex/index buffer
→ Depth buffer
→ Dynamic debug scene
→ Placeholder 3D yol + otobüs
```

## Faz 1 bilinçli eksikleri

Bunlar Faz 1 kapsamında tamamlama hedefi değildir:

- Üretim kalitesi UI/overlay
- Texture/material/PBR
- Gerçek asset import pipeline
- Fizik
- Sürüş dinamiği
- Ses
- Kayıt sistemi

## Faz 2 tahmini ara adımlar

Faz 2, **sürüş vertical slice** olacak. Tahmini 10–12 ara adım sürecek.

Önerilen sıra:

1. **Faz 2.1 — Vehicle input/state modeli**  
   Gaz, fren, steering, vites, el freni/retarder gibi komut/state sınırı.

2. **Faz 2.2 — Kinematik otobüs prototipi**  
   Fizik motoru olmadan basit hız/yön/hareket; telefonda ilk sürülebilir prototip.

3. **Faz 2.3 — Mobil sürüş HUD input bölgeleri**  
   Sol/sağ direksiyon, gaz/fren butonları, safe-area ve basılı tutma davranışı.

4. **Faz 2.4 — Takip/kabin kamera prototipi**  
   Debug kamera yerine araca bağlı takip kamerası.

5. **Faz 2.5 — Test pisti / yol segmentleri**  
   Düz yol yerine dönüşlü küçük test pisti.

6. **Faz 2.6 — Fizik backend adapter tasarımı**  
   `IPhysicsWorld`, `IVehiclePhysicsBackend` sınırı; Jolt değerlendirmesine hazırlık.

7. **Faz 2.7 — Raycast teker/süspansiyon prototipi**  
   Teker noktaları, spring/damper, zemin teması.

8. **Faz 2.8 — Motor/şanzıman/fren modeli başlangıcı**  
   Tork eğrisi, otomatik vites state machine, fren/retarder ayrımı.

9. **Faz 2.9 — Çarpışma/yakıt/hasar ilk sinyalleri**  
   Kaba collision proxy ve sürüş event'leri.

10. **Faz 2.10 — Sürüş metrikleri ve stabilizasyon**  
   5 dakika sürüş testi, FPS/stabilite, input latency gözlemi.

Gerekirse 2.11–2.12 küçük düzeltme/kalite adımları eklenir.

## Faz 2 kabul hedefi

Faz 2 sonunda hedef:

```text
Telefonda 5 dakika boyunca kararlı şekilde sürülebilen placeholder otobüs.
Basit test pisti.
Gaz/fren/direksiyon hissedilir.
Kamera aracı takip eder.
Pause/resume çökmez.
```
