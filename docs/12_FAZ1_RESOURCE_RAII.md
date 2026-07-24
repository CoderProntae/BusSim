# Faz 1.7 — Vulkan Resource RAII Temeli

Bu teslim, renderer içinde büyümeye başlayan Vulkan buffer/image temizleme kodlarını ilk küçük RAII kaynak sınıflarına ayırır.

## Eklenen dosyalar

```text
app/src/main/cpp/include/roadforge/renderer/VulkanResources.hpp
app/src/main/cpp/src/renderer/VulkanResources.cpp
```

## Eklenen sınıflar

### `DeviceBuffer`

Sahip olduğu kaynaklar:

```text
VkDevice
VkBuffer
VkDeviceMemory
VkDeviceSize
```

Görevleri:

- Copy kapalıdır.
- Move desteklenir.
- `destroy()` buffer ve memory'yi güvenli sırayla kapatır.
- Destructor `destroy()` çağırır.
- `valid()` ile handle durumu kontrol edilir.

Şu an debug vertex/index buffer kaynakları bu sınıfa taşındı.

### `DeviceImage`

Sahip olduğu kaynaklar:

```text
VkDevice
VkImage
VkDeviceMemory
VkImageView
VkFormat
width / height
```

Görevleri:

- Copy kapalıdır.
- Move desteklenir.
- `destroy()` image view, image ve memory'yi güvenli sırayla kapatır.
- Destructor `destroy()` çağırır.
- `valid()` / `hasView()` ile handle durumu kontrol edilir.

Şu an depth image kaynağı bu sınıfa taşındı.

## Neden önemli?

Yakında renderer'a şunlar eklenecek:

- Daha fazla mesh buffer'ı
- Texture image'ları
- Sampler'lar
- Descriptor set'ler
- Pipeline varyantları
- Asset streaming kaynakları

Bunları raw handle olarak tek sınıfta tutmak hızla hata üretir. Bu RAII adımı, ileride kaynak sızıntısı ve yanlış destroy sırasını azaltmak için atıldı.

## Telefon kabul testi

Bu adımın görsel farkı yoktur. Beklenen davranış önceki sürümle aynı:

1. APK açılır.
2. Perspektif yol plakası görünür.
3. Diagnostic input renkleri çalışır.
4. Pause/resume ve ekran döndürme/surface recreate çökmez.

## Sonraki teslim

Faz 1.8 hedefi:

1. Entity/world iskeletine giriş.
2. Transform component / mesh component gibi temel bileşenler.
3. Renderer debug mesh çizimini ileride world'den beslemeye hazırlamak.
