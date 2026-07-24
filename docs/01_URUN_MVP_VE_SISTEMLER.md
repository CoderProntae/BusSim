# RoadForge Bus Sim — Ürün MVP ve Sistemler

## Ürün hedefi

Mobil cihazlarda native çalışan, offline tek oyunculu, şirket yönetimi ve gerçekçi sürüş hissi olan 3D otobüs simülasyonu.

## Araç ve sürüş

- Mobil touch eksenleri: gaz, fren, direksiyon
- Kamera gesture'ları
- Motor tork eğrisi
- Diferansiyel
- Otomatik/manuel şanzıman state machine
- Teker başına raycast süspansiyon + yay/sönüm
- Longitudinal/lateral tire slip
- Yük transferi
- Kabin, takip, dış/sinematik kamera
- Çarpışma enerjisi/aşınma → bakım maliyeti ve görsel durum

## Şirket ve ekonomi

Önerilen veri modelleri:

```text
Company { cash, reputation, ownedBuses[], routeUnlocks[], ledger[] }
Bus     { purchasePrice, fuelType, capacity, wear, serviceCost, upgrades }
Route   { origin, destination, distanceKm, duration, demandCurve, tolls }
Trip    { busId, routeId, ticketPrice, servicePlan, passengers[], state }
Ledger  { timestamp, category, debit, credit, referenceId }
```

Kural: bakiye doğrudan değiştirilmez. Her parasal olay bir `LedgerEntry` üretir. Bu yaklaşım kayıt, hata ayıklama ve ileride olası cloud sync için gereklidir.

## Yolcu sistemi

- Yolcu grupları archetype + seed olarak üretilir.
- Yolcu sadece biniş/iniş çevresinde görünür NPC seviyesine yükseltilir.
- Koltuğa oturunca ekonomik simülasyon nesnesine indirgenir.
- Memnuniyet bileşenleri:
  - Zamanında varış
  - Sürüş konforu
  - Fiyat algısı
  - İkram kalitesi
  - Sıcaklık/iklimlendirme
  - Güvenlik / hasar / çarpışma

## Trafik AI

- Bake edilmiş şerit-segment graph
- Uzak araçlar: `segmentId`, `s`, `speed` ile ucuz simülasyon
- Yakın araçlar: takip modeli + çarpışma proxy
- Tam rigid body yalnızca gerektiğinde

## Sefer, hava ve mola

- Dünya zamanı
- Hava state'i: açık, bulutlu, yağmur, sis
- Hava hem görsel kaliteyi hem tutunmayı etkiler
- Mola: rota üzeri servis düğümü; yakıt/dinlenme/yolcu ihtiyaçları

Sefer state machine:

```text
Planning → Boarding → Departing → EnRoute → RestStop → Arriving → Settlement
```

## HUD / mobil UI

Öncelik sırası:

1. Hayati sürüş: hız, vites, sinyal, yakıt, hasar
2. Bağlamsal eylemler: kapı, mola, geri manevra/kamera
3. Navigasyon: minimap, yön, sonraki dönüş, terminal

Safe area, çentik, farklı aspect ratio ve sağ/sol el yerleşimi desteklenmelidir.
