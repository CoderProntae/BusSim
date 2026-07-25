# Faz 1.11 — Placeholder Otobüs + Basit 3D Yol Sahnesi

Bu teslim, yön etiketli diagnostic kareden daha oyun benzeri ilk 3D debug sahneye geçiştir.

## Eklenen görsel içerik

Renderer debug mesh artık tek başına kare değil; basit bir sahne içerir:

- Uzun asfalt yol düzlemi
- Yol kenar çizgileri
- Kesik orta şerit çizgileri
- `BUS` yer etiketi
- Turuncu placeholder otobüs gövdesi
- Basit kabin/roof parçası
- Koyu mavi cam paneller
- Siyah tekerlek blokları
- Basit far blokları

Bu hâlâ programmer art/debug mesh'tir. Ama artık renderer gerçek bir yol + otobüs ilişkisini gösterir.

## Teknik durum

Bu sahne hâlâ tek debug vertex/index buffer içine yazılıyor. Bir sonraki adımda mesh ayrımı ve World → Render proxy akışı netleştirilecek.

Kullanılan mevcut altyapılar:

```text
Vulkan pipeline
Vertex/index buffer
Depth buffer
MVP camera path
World transform
RAII DeviceBuffer / DeviceImage
```

## Kamera/input kararı

Kamera sabittir. Sol/sağ/alt inputlar yalnızca diagnostic arka plan rengi verir. Problemli geçici kamera hareketleri kapalıdır.

## Telefon kabul testi

1. APK açılır.
2. Basit 3D yol görünür.
3. Turuncu placeholder otobüs görünür.
4. Şerit çizgileri ve cam/teker/far blokları seçilebilir.
5. Sol/sağ/alt inputlarda arka plan diagnostic renkleri çalışır.
6. Kamera hareket etmez.
7. Overlay panel görünmez.
8. Pause/resume çökmez.

## Sonraki teslim

Faz 1.12 hedefi:

1. MeshKind ayrımı: Road, BusPlaceholder, DebugMarker.
2. World'deki MeshComponent listesinden renderer'a basit render proxy üretimi.
3. Debug scene objelerini tek buffer içinde hardcoded olmak yerine world verisinden beslemeye hazırlamak.
