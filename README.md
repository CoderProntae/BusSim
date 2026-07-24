# RoadForge Bus Sim

RoadForge Bus Sim, Android mobil cihazlar için C++20 + Vulkan tabanlı özel oyun motoru üzerine geliştirilen 3D otobüs simülasyonu projesidir.

Bu depo şu an **Faz 0 / native Vulkan bootstrap** durumundadır. Amaç henüz otobüs fiziği veya 3D dünya üretmek değil; Android telefon üzerinde gerçek native render yolunu doğrulamaktır.

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
- Vulkan 1.1 destekli cihazda koyu lacivert ekran görünür.
- Ekrana dokununca kısa süre turuncu tonlu clear color görünür.
- Pause/resume ve Surface yeniden oluşturma durumlarında Vulkan kaynakları güvenli kapatılıp yeniden kurulur.

> Not: Bu temel sürümde shader, mesh, otobüs modeli, fizik veya UI yoktur. Bunlar Faz 1 ve Faz 2'de küçük doğrulanabilir adımlarla eklenecektir.

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
├── .github/workflows/android-debug.yml
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
    └── 02_ROADMAP.md
```

## Bilgisayarsız APK test akışı

1. Bu depoyu GitHub'a yükle veya Arena branch'inden GitHub Actions çalıştır.
2. GitHub'da **Actions → Android Debug APK** workflow'unu aç.
3. Workflow başarılı olursa artifact olarak `roadforge-bussim-debug-apk` indir.
4. ZIP içinden `app-debug.apk` dosyasını telefona çıkar.
5. Android telefonda “bilinmeyen uygulama yükleme” izni verip APK'yı kur.
6. Beklenen sonuç: koyu lacivert Vulkan ekranı; dokununca kısa turuncu tepki.

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

Faz 0 gerçek cihazda onaylandıktan sonra Faz 1'e geçilecek:

1. Core altyapı: assert, result/error, zamanlayıcı, profiler iskeleti.
2. Math: vector/matrix/quaternion.
3. Sabit 60 Hz simulation clock + render interpolation hazırlığı.
4. ECS/world iskeleti.
5. Shader/mesh pipeline ve 3D debug otobüs/yol sahnesi.
