# Faz 1.14 — Debug Mesh Asset Verisini Renderer Ana Dosyasından Ayırma

Bu teslim, debug sahne mesh üretimini `VulkanRenderer.cpp` içinden çıkarıp ayrı bir mesh asset/build modülüne taşır.

## Eklenen dosyalar

```text
app/src/main/cpp/include/roadforge/renderer/DebugMeshAssets.hpp
app/src/main/cpp/src/renderer/DebugMeshAssets.cpp
```

## Neden yapıldı?

`VulkanRenderer.cpp` içinde şu sorumluluklar birikmişti:

- Vulkan lifecycle
- Swapchain
- Render pass
- Pipeline
- Buffer/image kaynakları
- Debug sahne mesh verisi
- Placeholder otobüs/yol geometri üretimi

Mesh verisini ayrı dosyaya almak, Faz 2 öncesinde renderer ana dosyasını temiz tutar ve ileride gerçek asset loader'a geçişi kolaylaştırır.

## Yeni yapı

`DebugMeshAssets.hpp`:

```text
DebugVertex
buildDebugSceneFromProxies(...)
```

`DebugMeshAssets.cpp`:

- Road surface mesh üretimi
- Bus placeholder mesh üretimi
- Basit blok/quad helper'ları
- Proxy transform uygulama

Renderer artık sadece şunu çağırır:

```cpp
buildDebugSceneFromProxies(debugRenderProxies_, vertices, indices);
```

## Görsel değişiklik

Beklenen görsel değişiklik yoktur. Bu adım mimari temizliktir.

## Kullanıcı testi gerekli mi?

Hayır. APK build'in başarılı olması yeterli kabul edilir.

## Sonraki teslim

Faz 1.15 hedefi:

1. Faz 1 kapanış cleanup.
2. Roadmap güncellemesi.
3. Faz 2 için araç/sürüş sistemi başlangıç planını kodda ve dokümanda hazırlamak.
