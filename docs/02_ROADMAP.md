# RoadForge Bus Sim — Yol Haritası

## Faz 0 — Native Android/Vulkan bootstrap

Durum: **Tamamlandı.**

Kapsam:

- Android/JNI/C++/Vulkan başlangıç hattı
- GitHub Actions ile APK alma yolu
- Gerçek cihaz APK testi ve hataların giderilmesi

Kabul kriteri:

- Uygulama açılır.
- Vulkan clear screen görünür.
- Dokunmaya renk tepkisi verir.
- Pause/resume'da çökmez.

## Faz 1 — Motor çekirdeği + 3D debug scene

Durum: **Kapanış aşamasında.**

Tamamlanan ana parçalar:

- Android Choreographer frame loop
- Native C++ Engine host
- 60 Hz `SimulationClock`
- InputSystem ve diagnostic input renkleri
- FrameStats toplama
- Math: `Vec`, `Mat4`, `Quat`, `Transform`, `Frustum`
- World/ECS başlangıcı
- World → RenderProxy köprüsü
- Vulkan shader pipeline
- Vertex/index buffer
- Depth buffer
- RAII buffer/image kaynakları
- Dynamic debug scene
- Placeholder 3D yol + otobüs
- GitHub Actions debug APK akışı

Bilinçli ertelenen Faz 1 parçaları:

- Üretim kalitesi UI/overlay: ayrı 2D UI pipeline ile dönecek.
- Texture/material/PBR: Faz 2 sonrası görsel kalite adımlarında genişletilecek.
- Gerçek asset import: önce static mesh veri formatı ve sonra loader.

Faz 1 kapanış hedefi:

- 3D yol + placeholder otobüs stabil çalışır.
- APK CI başarılıdır.
- Pause/resume stabil kalır.
- Faz 2 sürüş vertical slice için world/render/input sınırları hazırdır.

## Faz 2 — Sürüş vertical slice

Tahmini süre: **10–12 ara adım.**

Önerilen sıra:

1. **Faz 2.1 — Vehicle input/state modeli**
   - Gaz, fren, steering, vites, retarder/el freni komutları.
   - UI input ile araç sistemi arasında command sınırı.

2. **Faz 2.2 — Kinematik otobüs prototipi**
   - Fizik motoru olmadan basit hız/yön/hareket.
   - İlk telefonda sürülebilir placeholder otobüs.

3. **Faz 2.3 — Mobil sürüş HUD input bölgeleri**
   - Sağ/sol el düzeni hazırlığı.
   - Gaz/fren/direksiyon bölgeleri.
   - Safe-area yaklaşımı.

4. **Faz 2.4 — Takip/kabin kamera prototipi**
   - Araca bağlı takip kamera.
   - Sabit debug kamera yerine sürüş kamerası.

5. **Faz 2.5 — Test pisti / yol segmentleri**
   - Düz yol yerine dönüşlü küçük test pisti.
   - Şerit ve zemin referansları.

6. **Faz 2.6 — Fizik backend adapter tasarımı**
   - `IPhysicsWorld`
   - `IVehiclePhysicsBackend`
   - Jolt değerlendirmesi için sınırların hazırlanması.

7. **Faz 2.7 — Raycast teker/süspansiyon prototipi**
   - Teker temas noktaları.
   - Spring/damper.
   - Basit yer tutuş sinyali.

8. **Faz 2.8 — Motor/şanzıman/fren başlangıcı**
   - Tork eğrisi.
   - Otomatik vites state machine.
   - Fren/retarder ayrımı.

9. **Faz 2.9 — Çarpışma/yakıt/hasar ilk sinyalleri**
   - Kaba collision proxy.
   - Yakıt tüketimi sinyali.
   - Hasar event'i temeli.

10. **Faz 2.10 — Sürüş metrikleri ve stabilizasyon**
    - 5 dakikalık sürüş testi.
    - FPS/stabilite gözlemi.
    - Input latency ve kamera hissi düzeltmeleri.

Olası ek adımlar:

- **Faz 2.11 — Physics tuning pass**
- **Faz 2.12 — Sürüş vertical slice kapanış APK'sı**

Kabul kriteri:

- 5 dakikalık sürüş yapılabilir.
- Araç kararlı ve input duyarlıdır.
- Kamera aracı takip eder.
- Basit test pistinde sürüş hissi anlaşılırdır.
- Profil cihazda 60 FPS hedefi korunur.

## Faz 3 — Sefer vertical slice

Tahmini: **8–12 ara adım.**

- Trafik AI
- Yolcu biniş/iniş
- Hava/zaman
- Mola
- GPS
- Terminalden terminale 10–15 dakikalık tam sefer

## Faz 4 — Şirket MVP

Tahmini: **8–10 ara adım.**

- Yerel/sürümlü kayıt
- Filo/otobüs satın alma
- Rota seçimi
- Bilet/ikram fiyatı
- Ledger
- İtibar/talep
- Ana menü/garaj/sefer planlama

## Faz 5 — Üretim kalitesi

Tahmini: **10–15+ ara adım.**

- Dinamik çözünürlük
- Cihaz kalite profilleri
- Asset streaming/pak
- KTX/Basis texture
- Ses/titreşim
- Erişilebilirlik
- TR/EN lokalizasyon
- Onaylı crash telemetry
- QA
