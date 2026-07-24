# Faz 1.3 — Shader Pipeline ve İlk Vulkan Üçgeni

Bu teslim, clear-screen bootstrap'ten gerçek shader tabanlı GPU çizimine geçiştir.

## Eklenenler

### GLSL shader kaynakları

```text
app/src/main/cpp/shaders/debug_triangle.vert
app/src/main/cpp/shaders/debug_triangle.frag
```

Vertex shader şu an vertex buffer kullanmaz; `gl_VertexIndex` ile üç debug köşesi üretir. Bu, vertex buffer abstraction gelmeden önce pipeline'ın doğru çalıştığını test etmek için bilinçli bir ara adımdır.

### Shader build/embed hattı

```text
app/src/main/cpp/cmake/EmbedShaders.cmake
app/src/main/cpp/CMakeLists.txt
```

CMake artık Android NDK içindeki `glslc` aracını bulur, GLSL shader'ları SPIR-V'e derler ve generated C++ header içine byte array olarak gömer. Böylece runtime asset loader gerekmeksizin native renderer shader modülü oluşturabilir.

Akış:

```text
GLSL source
→ NDK glslc
→ SPIR-V binary
→ generated roadforge/renderer/GeneratedShaders.hpp
→ VkShaderModule
→ VkPipeline
→ vkCmdDraw(3)
```

### Vulkan graphics pipeline

Renderer'a eklenenler:

- Shader module oluşturma
- Pipeline layout
- Graphics pipeline
- Triangle-list input assembly
- Viewport/scissor
- Rasterizer
- Color blend state
- Swapchain recreate sırasında pipeline recreate
- Render pass içinde `vkCmdBindPipeline` + `vkCmdDraw(3)`

## Telefon kabul testi

1. APK açılır.
2. Lacivert arka plan görünür.
3. Ekranın ortasında renkli üçgen görünür.
4. Dokununca turuncu pulse hâlâ çalışır.
5. Alt sağ/alt sol/sağ-sol dokunuşlar arka plan tonunu çok hafif etkiler.
6. Pause/resume çökmez.

## Sonraki teslim

Faz 1.4 hedefi:

1. Vertex/index buffer abstraction.
2. Basit CPU-side mesh tanımı.
3. Quad veya basit yol plakası çizimi.
4. Depth buffer + perspektif kamera hazırlığı.
