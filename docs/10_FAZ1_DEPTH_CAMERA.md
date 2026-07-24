# Faz 1.5 — Depth Buffer ve Perspektif Kamera/MVP Hattı

Bu teslim, 2D/NDC debug mesh çiziminden gerçek 3D render hazırlığına geçiştir.

## Eklenenler

### Math

Yeni dosya:

```text
app/src/main/cpp/include/roadforge/math/Mat4.hpp
```

İlk kapsam:

- Column-major `Mat4`
- `identity`
- `multiply`
- Vulkan uyumlu left-handed perspektif projection
- `lookAtLH`
- `dot`, `cross`, `subtract` yardımcıları

Bu temel daha sonra `Transform`, `Quat`, frustum ve kamera sistemine genişletilecek.

### Shader MVP push constant

Vertex shader artık `uMvp` push constant kullanıyor:

```glsl
layout(push_constant) uniform PushConstants {
    mat4 uMvp;
} pc;
```

Bu, her draw call için model-view-projection matrix göndermenin ilk küçük adımıdır.

### Depth buffer

Renderer'a eklendi:

- Depth format seçimi: `D32`, `D16` fallback
- Depth image
- Depth image memory
- Depth image view
- Render pass depth attachment
- Framebuffer color + depth attachment
- Pipeline depth test/write

### 3D debug road plate

Debug mesh artık 3D koordinatlarda yer alıyor. Kamera şu an sabit:

```text
eye    = (0, 1.65, -4.25)
target = (0, 0.00,  3.40)
```

Yani ekrandaki yol plakası artık perspective/MVP hattından geçerek çiziliyor.

## Telefon kabul testi

1. APK açılır.
2. Lacivert arka plan görünür.
3. Ortada perspektif hissi veren koyu yol plakası görünür.
4. Alt sağ/alt sol diagnostic renkleri hâlâ çalışır.
5. Pause/resume çökmez.

## Sonraki teslim

Faz 1.6 için hedef:

1. Renderer buffer/image resource kodunu sınıflaştırmak.
2. `Transform`, `Quat`, `Frustum` eklemek.
3. Basit kamera controller ve debug overlay hazırlığı.
4. ECS/world iskeletine giriş.
