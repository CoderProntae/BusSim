# Faz 1.8 — Minimal World/ECS İskeleti

Bu teslim, debug render objesini renderer içine gömülü state olmaktan çıkarıp ilk küçük world/entity temsiline taşır.

## Eklenen dosyalar

```text
app/src/main/cpp/include/roadforge/world/World.hpp
app/src/main/cpp/src/world/World.cpp
```

## Eklenen kavramlar

### Entity

İlk entity handle yapısı:

```text
Entity { index, generation }
```

Generation alanı, ileride silinmiş entity handle'larının yanlışlıkla geçerli sanılmasını engellemek için eklendi.

### Components

İlk iki component:

```text
TransformComponent { Transform }
MeshComponent      { MeshKind, boundingRadius, visible }
```

Şimdilik sadece debug yol plakası için kullanılıyor.

### World

İlk world sorumlulukları:

- Entity oluşturma/silme
- Alive kontrolü
- Transform component ekleme/okuma
- Mesh component ekleme/okuma
- Debug road entity oluşturma
- Sabit tick içinde debug road transform güncelleme

Bu aşama tam üretim ECS değildir. Ama renderer'ın sahne objelerini ileride world'den alması için ilk sınırı açar.

## Engine bağlantısı

`Engine` artık surface oluşturulduğunda world'ü resetler ve debug road entity yaratır:

```text
Engine
→ World::createDebugRoadEntity
→ SimulationClock fixed tick
→ World::fixedUpdate
→ Renderer::setDebugRoadTransform
→ Vulkan draw
```

Yani debug yol plakasının transform'u artık renderer içinde üretilmiyor; world tarafından yönetiliyor.

## Telefon kabul testi

Bu adımın büyük görsel farkı yoktur. Beklenenler:

1. APK açılır.
2. Perspektif yol plakası görünür.
3. Diagnostic input renkleri çalışır.
4. Pause/resume çökmez.

## Sonraki teslim

Faz 1.9 hedefi:

1. Kamera controller hazırlığı.
2. Debug overlay için frame stats veri yüzeyi.
3. World içinden render scene/proxy veri aktarma modelinin ilk hali.
