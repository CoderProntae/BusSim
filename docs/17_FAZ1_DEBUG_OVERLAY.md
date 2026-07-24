# Faz 1.10 — Görsel Debug Overlay / Frame Stats Yüzeyi

Bu teslim, logcat dışında ekranda görünen ilk native Vulkan debug overlay yüzeyini ekler.

## Eklenen davranış

Ekranın sol üstünde küçük bir overlay paneli çizilir. Bu panel shader/mesh pipeline üzerinden çizilen basit 2D NDC quad'lardan oluşur.

Panel satırları:

| Etiket | Anlam |
|---|---|
| `FPS` | Tahmini FPS oranı, 60 FPS'e göre doluluk |
| `MS` | Ortalama frame süresi sağlığı |
| `SIM` | Ortalama fixed simulation step sayısı |
| `DRP` | Dropped accumulated simulation time uyarısı |

Şimdilik sayısal font yok; değerler bar doluluklarıyla temsil ediliyor. Bu bilinçli: önce Vulkan overlay hattını doğruluyoruz, sonra daha okunabilir font/texture atlas ekleyeceğiz.

## Teknik akış

```text
FrameStatsSnapshot
→ Engine::renderer.setFrameStats
→ VulkanRenderer::updateDebugOverlayBuffers
→ per-frame host-visible overlay vertex/index buffer
→ identity MVP ile NDC overlay draw
```

Overlay buffer'ları frame-in-flight başına ayrılır. Böylece CPU'nun güncel frame overlay verisini yazması, önceki frame'in GPU okumasıyla çakışmaz.

## Telefon kabul testi

1. APK açılır.
2. `SOL / SAG / UST / ALT` etiketli kare görünür.
3. Sol üstte küçük debug overlay paneli görünür.
4. Panelde `FPS`, `MS`, `SIM`, `DRP` etiketleri ve renkli barlar görünür.
5. Diagnostic input renkleri çalışır.
6. Pause/resume çökmez.

## Sonraki teslim

Faz 1.11 hedefi:

1. Placeholder otobüs mesh'i.
2. Basit yol/debug scene kompozisyonu.
3. MeshKind çeşitlendirme ve World → Render proxy akışının ilk hali.
