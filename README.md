# RoadForge Bus Sim

RoadForge Bus Sim, Android mobil cihazlar için C++20 + Vulkan tabanlı özel oyun motoru üzerine geliştirilen 3D otobüs simülasyonu projesidir.

Bu depo şu an **Faz 1** durumundadır. Faz 0 Android/Vulkan hattı gerçek cihazda doğrulandı; şimdi motor çekirdeği ve ilk gerçek GPU çizim hattı küçük, test edilebilir parçalarla ekleniyor.

## Faz 0'da çalışan zincir

```text
Android SurfaceView + touch
→ Java MainActivity
→ JNI bridge
→ C++ Engine
→ Vulkan instance / Android surface / GPU / logical device / swapchain
→ command buffer + render pass
→ Android ekrana GPU present
```

Beklenen davranış:

- Uygulama landscape tam ekran açılır.
- Vulkan 1.1 destekli cihazda koyu lacivert arka plan görünür.
- Ortada depth buffer ve perspektif kamera ile çizilen yön etiketli kare diagnostic mesh görünür: SOL, SAG, UST, ALT.
- Ekrana dokununca kısa turuncu pulse görünür.
- Alt sağ dokunma parlak yeşil, alt sol dokunma parlak kırmızı, sol/sağ dokunma parlak mavi/sarı-turuncu diagnostic geri bildirimi verir ve yaklaşık 1.25 saniye ekranda kalır.
- Pause/resume ve Surface yeniden oluşturma durumlarında Vulkan kaynakları güvenli kapatılıp yeniden kurulur.

> Not: Bu sürümde shader pipeline ve debug mesh vardır; otobüs modeli, fizik ve oyun UI henüz yoktur. Bunlar Faz 1/Faz 2 içinde küçük doğrulanabilir adımlarla eklenecektir.

## Teknik hedefler

- Android API 29+ / Android 10+
- ABI: `arm64-v8a`
- Vulkan 1.1 zorunlu
- C++20
- Offline tek oyunculu yerel kayıt mimarisi
- Üst segment Android cihazlarda 60 FPS hedefi

## Depo yapısı

```text
.
├── README.md
├── settings.gradle.kts
├── build.gradle.kts
├── gradle.properties
├── app/
│   ├── build.gradle.kts
│   └── src/main/
│       ├── AndroidManifest.xml
│       ├── java/com/roadforge/bussim/MainActivity.java
│       ├── res/values/styles.xml
│       └── cpp/
│           ├── CMakeLists.txt
│           ├── include/roadforge/
│           │   ├── core/Log.hpp
│           │   ├── engine/Engine.hpp
│           │   └── renderer/VulkanRenderer.hpp
│           └── src/
│               ├── core/Log.cpp
│               ├── engine/Engine.cpp
│               ├── platform/android/JniBridge.cpp
│               └── renderer/VulkanRenderer.cpp
└── docs/
    ├── 00_TEKNIK_ANAYASA.md
    ├── 01_URUN_MVP_VE_SISTEMLER.md
    ├── 02_ROADMAP.md
    ├── 03_GITHUB_ACTIONS_TELEFON_APK.md
    ├── 04_FAZ1_CORE_CLOCK_MATH.md
    ├── 05_FAZ1_INPUT_FRAME_STATS.md
    ├── 06_FAZ1_SHADER_TRIANGLE.md
    ├── 07_FAZ1_VERTEX_INDEX_MESH.md
    ├── 08_FAZ1_VISIBLE_INPUT_FEEDBACK.md
    ├── 09_FAZ1_INPUT_DIAGNOSTIC_COLORS.md
    ├── 10_FAZ1_DEPTH_CAMERA.md
    ├── 11_FAZ1_TRANSFORM_FRUSTUM.md
    ├── 12_FAZ1_RESOURCE_RAII.md
    ├── 13_FAZ1_WORLD_ECS.md
    ├── 14_FAZ1_CAMERA_CONTROLLER.md
    ├── 15_FAZ1_ORIENTATION_SQUARE.md
    ├── 16_FAZ1_SEPARATED_CAMERA_INPUT.md
    ├── 17_FAZ1_DEBUG_OVERLAY.md
    ├── 18_FAZ1_OVERLAY_ORIENTATION_FIX.md
    ├── 19_FAZ1_OVERLAY_AXIS_FIX.md
    ├── 20_FAZ1_OVERLAY_TOP_LEFT_FIX.md
    ├── 21_FAZ1_TRACK_CAMERA.md
    ├── 22_FAZ1_FIXED_DEBUG_CAMERA.md
    ├── 23_FAZ1_BUS_ROAD_SCENE.md
    ├── 24_FAZ1_RENDER_PROXIES.md
    ├── 25_FAZ1_DYNAMIC_PROXY_SCENE.md
    ├── 26_FAZ1_DEBUG_MESH_ASSETS.md
    ├── 27_FAZ1_CLOSEOUT_FAZ2_PREP.md
    ├── 28_FAZ2_VEHICLE_STATE_MODEL.md
    ├── 29_FAZ2_KINEMATIC_BUS.md
    └── 30_FAZ2_MULTITOUCH_DRIVING_INPUT.md
```

## Bilgisayarsız APK test akışı

1. Bu depoyu GitHub'a yükle veya Arena branch'ini aç.
2. Eğer repoda `.github/workflows/android-debug.yml` yoksa `docs/03_GITHUB_ACTIONS_TELEFON_APK.md` dosyasındaki telefon adımlarını izleyerek workflow'u GitHub web arayüzünden oluştur.
3. GitHub'da **Actions → Android Debug APK** workflow'unu aç.
4. Workflow başarılı olursa artifact olarak `roadforge-bussim-debug-apk` indir.
5. ZIP içinden `app-debug.apk` dosyasını telefona çıkar.
6. Android telefonda “bilinmeyen uygulama yükleme” izni verip APK'yı kur.
7. Beklenen sonuç: lacivert Vulkan arka planı, ortada SOL/SAG/UST/ALT etiketli kare, dokununca turuncu pulse; alt sağ parlak yeşil, alt sol parlak kırmızı input diagnostic rengi.

> Not: Arena'nın GitHub bağlantısında `workflows` yetkisi olmadığı için workflow dosyasını otomatik push edemedim. Kod branch'e push edildi; workflow içeriği `docs/03_GITHUB_ACTIONS_TELEFON_APK.md` içinde hazırdır.

## Yerel derleme

Android Studio veya yüklü Android SDK/NDK olan bir bilgisayarda:

```bash
gradle :app:assembleDebug
```

Gerekli paketler:

- JDK 17
- Android platform 35
- Build Tools 35.0.0
- CMake 3.22.1
- NDK 27.2.12479018
- Gradle 8.9+

## Sonraki adım

Faz 0 gerçek cihazda onaylandı. Faz 1 kapanışa alındı. Faz 2 başladı. Kinematik placeholder otobüs hareketi ve çoklu dokunmalı sürüş input bölgeleri eklendi. Faz 1 için eklenen parçalar: `SimulationClock`, temel `Vec` math yardımcıları, native input action/axis sınırı, frame stats altyapısı, renderer debug timing/input bağlantısı, shader tabanlı Vulkan çizim, ilk vertex/index buffer debug mesh, görünür input debug geri bildirimi, depth buffer, perspektif kamera/MVP hattı, Transform/Quat, frustum culling math temeli, ilk Vulkan resource RAII katmanı ve minimal World/ECS iskeleti, ilk debug kamera controller bağlantısı, yön etiketli orientation square diagnostic mesh ve ayrıştırılmış kamera inputu ve ilk görsel debug overlay/frame stats yüzeyi ve overlay geçici olarak kapalı, debug kamera sabit, placeholder otobüs ve basit 3D yol sahnesi eklendi; World → Render proxy köprüsü başladı, renderer debug sahnesi proxy listesinden dinamik üretiliyor, debug mesh asset verisi renderer ana dosyasından ayrıldı ve Faz 1 kapanış temizliği yapıldı. Sıradaki parçalar:

1. Faz 2.4 Takip/kabin kamera prototipi.
2. Faz 2.5 Test pisti / yol segmentleri.
3. Faz 2.6 Fizik backend adapter tasarımı.
3. Asset/mesh veri formatı hazırlığı.
4. Renderer resource abstraction katmanını pipeline/descriptor tarafına genişletmek.
5. Input action mapping'i gerçek sürüş HUD'una hazırlamak.
