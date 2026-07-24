# Faz 1.4 — Vertex/Index Buffer ve İlk Debug Mesh

Bu teslim, `gl_VertexIndex` ile çizilen geçici üçgenden gerçek vertex/index buffer kullanan ilk debug mesh'e geçiştir.

## Eklenen/Değişenler

### Shader vertex input

`debug_triangle.vert` artık vertex buffer'dan veri okur:

```glsl
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
```

### Renderer mesh kaynakları

Renderer içine ilk küçük mesh resource hattı eklendi:

- Host-visible vertex buffer
- Host-visible index buffer
- Vulkan memory type seçimi
- `vkCreateBuffer`
- `vkAllocateMemory`
- `vkBindBufferMemory`
- `vkMapMemory` / `vkUnmapMemory`
- `vkCmdBindVertexBuffers`
- `vkCmdBindIndexBuffer`
- `vkCmdDrawIndexed`

Şimdilik staging buffer yok; debug mesh çok küçük olduğu için doğrudan host-visible memory kullanılıyor. Üretim mesh/asset pipeline'a geçerken device-local buffer + staging upload yolu eklenecek.

### İlk debug mesh

Ekranda perspektif hissi veren koyu renkli bir quad/yol plakası çizilir. Bu henüz 3D kamera değildir; NDC koordinatlarıyla çizilen debug geometri katmanıdır. Ama artık renderer gerçek vertex/index data ile draw call atmaktadır.

## Telefon kabul testi

1. APK açılır.
2. Lacivert arka plan görünür.
3. Ekranın ortasında koyu renkli yol plakası/quad görünür.
4. Dokununca turuncu pulse hâlâ çalışır.
5. Alt sağ/alt sol/sağ-sol dokunuşlar arka plan tonunu çok hafif etkiler.
6. Pause/resume çökmez.

## Sonraki teslim

Faz 1.5 hedefi:

1. Buffer/resource kodunu renderer içinde daha temiz abstraction'a ayırmak.
2. Depth buffer eklemek.
3. `Mat4`, `Quat`, `Transform` math tiplerini eklemek.
4. Perspektif kamera ve ilk 3D debug scene hazırlığı.
