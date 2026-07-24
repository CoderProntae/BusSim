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
- Ortada depth buffer ve perspektif kamera ile çizilen koyu debug yol plakası görünür.
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
    └── 11_FAZ1_TRANSFORM_FRUSTUM.md
```

## Bilgisayarsız APK test akışı

1. Bu depoyu GitHub'a yükle veya Arena branch'ini aç.
2. Eğer repoda `.github/workflows/android-debug.yml` yoksa `docs/03_GITHUB_ACTIONS_TELEFON_APK.md` dosyasındaki telefon adımlarını izleyerek workflow'u GitHub web arayüzünden oluştur.
3. GitHub'da **Actions → Android Debug APK** workflow'unu aç.
4. Workflow başarılı olursa artifact olarak `roadforge-bussim-debug-apk` indir.
5. ZIP içinden `app-debug.apk` dosyasını telefona çıkar.
6. Android telefonda “bilinmeyen uygulama yükleme” izni verip APK'yı kur.
7. Beklenen sonuç: lacivert Vulkan arka planı, ortada debug yol plakası, dokununca turuncu pulse; alt sağ parlak yeşil, alt sol parlak kırmızı input diagnostic rengi.

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

Faz 0 gerçek cihazda onaylandı. Faz 1 için eklenen parçalar: `SimulationClock`, temel `Vec` math yardımcıları, native input action/axis sınırı, frame stats altyapısı, renderer debug timing/input bağlantısı, shader tabanlı Vulkan çizim, ilk vertex/index buffer debug mesh, görünür input debug geri bildirimi, depth buffer, perspektif kamera/MVP hattı, Transform/Quat ve frustum culling math temeli. Sıradaki parçalar:

1. Renderer buffer/resource kodunu temiz abstraction katmanına ayırmak.
2. ECS/world iskeleti.
3. Kamera kontrolü ve debug overlay.
4. Basit 3D otobüs/yol debug sahnesi.
5. Asset/mesh veri formatı hazırlığı.
