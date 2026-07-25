# Faz 1.13 — Renderer Debug Sahnesini Proxy Listesinden Dinamik Üretme

Bu teslim, bir önceki adımda başlayan World → RenderProxy köprüsünü renderer tarafında fiilen kullanır.

## Değişiklik

Önceden debug yol + placeholder otobüs tek seferlik hardcoded vertex/index buffer olarak oluşturuluyordu.

Artık renderer her frame için güvenli şekilde, mevcut frame-in-flight buffer'ına şu akışla veri yazar:

```text
World entities/components
→ World::collectRenderProxies
→ Engine maps World RenderProxy to Renderer DebugRenderProxy
→ VulkanRenderer::setDebugRenderProxies
→ updateDebugSceneBuffers(currentFrame)
→ proxy mesh kind'e göre vertex/index üretimi
→ Vulkan draw
```

## MeshKind ayrımı

Renderer tarafında iki debug mesh kind aktif:

```text
RoadSurface
BusPlaceholder
```

Her proxy kendi `Transform` bilgisiyle mesh üretimine girer. Şu an görünüm önceki sürüme yakın kalır; önemli fark veri kaynağıdır.

## Frame-in-flight güvenliği

Debug scene buffer'ları artık frame başına ayrılır:

```text
debugSceneVertexBuffers_[frame]
debugSceneIndexBuffers_[frame]
debugSceneIndexCounts_[frame]
```

Bu, CPU'nun güncel frame vertex/index verisini yazarken GPU'nun önceki frame verisini okumasıyla çakışmasını önler.

## Kullanıcı testi gerekli mi?

Bu adımda görsel değişiklik beklenmez. APK build'in başarılı olması yeterli kabul edilir. Telefonda test zorunlu değildir.

## Sonraki teslim

Faz 1.14 hedefi:

1. Mesh data tanımlarını renderer kaynak kodundan ayırmak.
2. Basit static mesh asset veri yapısı hazırlamak.
3. Faz 1 kapanış temizliği ve Faz 2 sürüş vertical slice hazırlığı.
