# RoadForge Bus Sim — Yol Haritası

## Faz 0 — Native Android/Vulkan bootstrap

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

1. Core: assert, error/result, dosya sistemi, job sistemi, profiler, zamanlayıcı.
2. Input action/axis sistemi ve input recording.
3. Math: vector/matrix/quaternion/frustum.
4. Entity/component/sparse set world ve system scheduler.
5. Renderer: shader asset pipeline, mesh/texture, depth, PBR, culling/LOD.
6. Debug overlay: FPS, CPU/GPU frame time, draw call, bellek.
7. glTF test otobüsü + test yol parçası + serbest kamera.

Kabul kriteri:

- Ekranda 3D test otobüsü/yol görünür.
- Mobilde 60 FPS metrik overlay'i çalışır.

## Faz 2 — Sürüş vertical slice

1. Sabit 60 Hz `SimulationClock`; render interpolation.
2. Jolt teknik değerlendirmesi, `IPhysicsWorld` adapterı.
3. Otobüs gövdesi, 4x2/6x2 teker raycast süspansiyonu.
4. Motor, şanzıman, retarder, fren.
5. Kabin/takip kamera + HUD.
6. Test pisti, çarpışma, yakıt.

Kabul kriteri:

- 5 dakikalık sürüş yapılabilir.
- Araç kararlı ve input duyarlıdır.
- Profil cihazda 60 FPS korunur.

## Faz 3 — Sefer vertical slice

- Trafik AI
- Yolcu biniş/iniş
- Hava/zaman
- Mola
- GPS
- Terminalden terminale 10–15 dakikalık tam sefer

## Faz 4 — Şirket MVP

- Yerel/sürümlü kayıt
- Filo/otobüs satın alma
- Rota seçimi
- Bilet/ikram fiyatı
- Ledger
- İtibar/talep
- Ana menü/garaj/sefer planlama

## Faz 5 — Üretim kalitesi

- Dinamik çözünürlük
- Cihaz kalite profilleri
- Asset streaming/pak
- KTX/Basis texture
- Ses/titreşim
- Erişilebilirlik
- TR/EN lokalizasyon
- Onaylı crash telemetry
- QA
