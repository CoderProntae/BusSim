# Faz 1.12 — World → Render Proxy Köprüsü

Bu teslim, debug sahnenin ileride renderer'a doğrudan hardcoded veri olarak değil, World/ECS tarafındaki render proxy listesiyle aktarılması için ilk köprüyü ekler.

## Eklenenler

### World tarafı

`MeshKind` genişletildi:

```text
DebugRoadSurface
BusPlaceholder
```

Yeni `RenderProxy` tipi eklendi:

```text
RenderProxy { transform, meshKind, boundingRadius, visible }
```

World artık iki debug entity yaratır:

- Road surface entity
- Bus placeholder entity

ve `collectRenderProxies(...)` ile görünür mesh component'lerini render proxy listesine toplar.

### Renderer tarafı

Yeni dosya:

```text
app/src/main/cpp/include/roadforge/renderer/RenderProxy.hpp
```

Renderer için debug proxy tipi eklendi:

```text
DebugRenderProxy { transform, meshKind, boundingRadius, visible }
```

Engine artık World proxy'lerini Renderer proxy'lerine map ederek renderer'a gönderir:

```text
World::collectRenderProxies
→ Engine mapping
→ VulkanRenderer::setDebugRenderProxies
```

## Not

Bu adımda görsel değişiklik beklenmez. Sahne hâlâ önceki placeholder otobüs + yol görünümünde kalır. Ama veri sahipliği daha doğru yöne taşındı: sahne objeleri World tarafında temsil edilmeye başladı.

Bir sonraki adımda renderer debug mesh buffer'ı bu proxy listesinden dinamik üretilecek. Böylece Road ve Bus ayrı mesh kind olarak çizilebilir hale gelecek.

## Telefon kabul testi

1. APK açılır.
2. Basit 3D yol görünür.
3. Placeholder otobüs görünür.
4. Diagnostic input renkleri çalışır.
5. Kamera sabit kalır.
6. Overlay panel görünmez.
7. Pause/resume çökmez.
